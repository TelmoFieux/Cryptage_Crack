#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include <ctype.h>
#include <stdbool.h>
#include "list.h"
#include "break_code_c1_multi_threaded.h"
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define MAX_WORD_LENGTH 100  // Longueur maximale d'un mot dans le dictionnaire


struct param{
    unsigned char** k;
    const char* f;
    int ks;
    const char* df;
    struct freq_index* res;
    int start;
    int end;
};

static pthread_mutex_t* mutex;
static pthread_mutex_t* mutex_car;
static sem_t* rdv;
static int Thread_bloquee=0;
static int NBthreads;

static void regroupement(){
    pthread_mutex_lock(mutex);
    Thread_bloquee++;
    if(Thread_bloquee==NBthreads){
        pthread_mutex_unlock(mutex);
        sem_post(rdv);
        printf("Je termine : %ld\n", pthread_self());

    }else{
        pthread_mutex_unlock(mutex);
        printf("J'attends les autres : %ld\n", pthread_self());
        sem_wait(rdv);
        printf("Je termine : %ld\n", pthread_self());
        pthread_mutex_lock(mutex);
        if(Thread_bloquee!=0) sem_post(rdv);
        Thread_bloquee--;
        pthread_mutex_unlock(mutex);
    }
}




/******************************************************************************
* Recherche dichotomique sur un fichier
 *****************************************************************************/
bool binary_search_in_file(const char *filename, const char *target) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Erreur d'ouverture du fichier");
        return false;
    }

    long left = 0;
    fseek(file, 0, SEEK_END);
    long right = ftell(file);  // Taille du fichier en octets
    char buffer[MAX_WORD_LENGTH] = {0};  // Initialisation de `buffer`

    while (left <= right) {
        long mid = (left + right) / 2;

        // Se positionner au milieu
        fseek(file, mid, SEEK_SET);

        // Avancer jusqu'à la fin de la ligne en cours (si non au début du fichier)
        if (mid != 0) {
            if (!fgets(buffer, MAX_WORD_LENGTH, file)) {
                break;  // Gestion de l'échec de lecture
            }
        }

        // Lire la ligne suivante
        if (!fgets(buffer, MAX_WORD_LENGTH, file)) {
            break;  // Fin du fichier atteinte
        }

        // Supprimer le caractère de fin de ligne
        buffer[strcspn(buffer, "\n")] = '\0';

        // Comparer avec la cible
        int cmp = strcmp(target, buffer);
        if (cmp == 0) {
            fclose(file);
            return true;  // Mot trouvé
        } else if (cmp < 0) {
            right = mid - 1;  // Aller à la moitié gauche
        } else {
            left = mid + 1;   // Aller à la moitié droite
        }
    }

    fclose(file);
    return false;  // Mot non trouvé
}


/******************************************************************************
* Renvoi la proportion de mot contenu dans le message décodé qui existent dans le 
dictionnaire.
 *****************************************************************************/
float get_correct_words(unsigned char* key, const char* filename, int keysize, const char* dico_filename){
    FILE *file = fopen(filename,"r");
    if (file == NULL){
        perror("Erreur à l'ouverture du fichier");
        exit(EXIT_FAILURE);
    }
    unsigned char word[45]; //le plus long mot de la langue anglaise a part des nom de molécule chimique
    unsigned char reader[1];
    float nb_incorrect_words = 0;
    float nb_correct_words = 0;
    int index_letter=0;
    int index_key=0;
    int car;
    while ((fread(&reader,sizeof(unsigned char),1,file))!=0) {
        car = reader[0]^key[index_key];
        if (isalpha(car)){
            if (index_letter < 45){
                word[index_letter] = toupper(car);
                index_letter++;
            } else {
                nb_incorrect_words++;
                index_letter=0;
            }
        } else {
            if (index_letter > 0){
                word[index_letter] = '\0';
                if(binary_search_in_file(dico_filename,(const char*)word)){
                    nb_correct_words++;
                } else {
                    nb_incorrect_words++;
                }
                index_letter=0;
            } else {
                nb_incorrect_words++;//On compte un enchainement de caratcère autre que des lettre comme une erreur
            }
        }
        index_key=(index_key + 1)%keysize; 
        
    }
    if (index_letter>0){
        if(binary_search_in_file(dico_filename,(const char*)word)) nb_correct_words++;
        else nb_incorrect_words++;
    }
    fclose(file);
    return nb_correct_words/(nb_correct_words+nb_incorrect_words);
}


static void* fils (void* arg){
    struct param* p = arg;
    float val;
    for(int i=p->start; i<p->end; i++){
        val=get_correct_words(p->k[i],p->f,p->ks,p->df);
        pthread_mutex_lock(mutex_car);
        p->res[i].frequency=val;
        p->res[i].i=i;
        pthread_mutex_unlock(mutex_car);
    }
    regroupement();
    pthread_exit(NULL);
}

/******************************************************************************
* Trie les clés en fonction de la proportion de mot correcte une fois
le texte décodé.
 *****************************************************************************/
unsigned char** crack_c3(const char* filename, unsigned char** keys, int NBkeys,const char* dico_filename, int NBThreads){

    NBthreads = NBThreads;
    mutex = malloc(sizeof(pthread_mutex_t));
    mutex_car = malloc(sizeof(pthread_mutex_t));
    rdv = malloc(sizeof(sem_t));

    if (!mutex || !rdv || !mutex_car) {
        perror("malloc");
        exit(1);
    }

    if (pthread_mutex_init(mutex, NULL) != 0) {
        perror("pthread_mutex_init");
        exit(1);
    }

    if (sem_init(rdv, 0, 0) != 0 ) {
        perror("sem_init");
        exit(1);
    }

    if (pthread_mutex_init(mutex_car, NULL) != 0) {
        perror("pthread_mutex_init");
        exit(1);
    }

    struct freq_index frequency[NBkeys];

    struct param** p= malloc(sizeof(struct param*)*NBThreads);
    pthread_t pid[NBthreads];

    int base = NBkeys / NBThreads;
    int extra = NBkeys % NBThreads;
    int current_start = 0;          // Point de départ pour chaque thread
    int err;
    int keysize = strlen((char*)keys[0]);
    for (int i = 0; i < NBthreads; i++) {
        p[i] = malloc(sizeof(struct param));
        if (!p[i]) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        p[i]->f = filename;
        p[i]->k = keys;
        p[i]->ks = keysize;
        p[i]->res = frequency;
        p[i]->df= dico_filename;

        int thread_load = base + (i < extra ? 1 : 0); // Ajoutez une clé supplémentaire pour les premiers `extra` threads
        p[i]->start = current_start;
        p[i]->end = current_start + thread_load;     // L'intervalle se termine ici (exclusif)

        current_start = p[i]->end;                   // Mettez à jour le début pour le prochain thread


        // Assurez-vous que chaque thread a au moins un intervalle valide
        if (p[i]->start >= p[i]->end) {
            fprintf(stderr, "Thread %d has an invalid range: start=%d, end=%d\n", i, p[i]->start, p[i]->end);
            exit(EXIT_FAILURE);
        }

        printf("Thread %d commence avec start=%d, end=%d\n", i, p[i]->start, p[i]->end);

        if ((err = pthread_create(&pid[i], NULL, fils, (void*)p[i])) != 0) {
            printf("Failed to create");
            perror("pthread_create");
            exit(1);
        }
    }

    printf("Processus parent %d attend la fin de son fils\n", getpid());
    for(int i = 0; i< NBthreads; i++){
        if((err=pthread_join(pid[i], NULL))!=0){
            perror("pthread_join");
            exit(1);
        }
    }
    pthread_mutex_destroy(mutex_car);
    pthread_mutex_destroy(mutex);
    sem_destroy(rdv);
    free(mutex);
    free(rdv);
    free(mutex_car);

    printf("Processus parent effectue le trie\n");

    
    unsigned char** sorted_keys = malloc(NBkeys * sizeof(unsigned char*));
    for(int i=0; i<NBkeys; i++){
        // Allocate memory for each key copy
        sorted_keys[i] = malloc((keysize + 1) * sizeof(unsigned char));
        if (sorted_keys[i] == NULL) {
            perror("Memory allocation failed");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i <NBThreads; i++){
        free(p[i]);
    }
    free(p);

    quicksort_iterative(frequency, 0, NBkeys - 1);

    for (int i=0; i<NBkeys; i++){
        strcpy((char*)sorted_keys[NBkeys-i-1], (char*)keys[frequency[i].i]);
    }

    return sorted_keys;

}

// int main(void){
//     list_t** key = crack_C1("1234_msg2.txt", 4);
//     int size;
//     unsigned char** combinations = generate_combinations(key, 4, &size);
//     for (int i = 0; i < size; i++) {
//         //printf("%s\n", combinations[i]);
//     }
//     unsigned char** sorted_keys = crack_c3("1234_msg2.txt", combinations, size,"../dictionary.txt",12);
//     for (int i = size-1; i >=0; i--) {
//         printf("%s\n", sorted_keys[i]);
//         free(sorted_keys[i]);  // Free each key
//     }
//     free(sorted_keys);  // Free the array of pointers
//     return 0;
// }

//compilation : gcc -Wall -Wextra -o crack3 break_code_c3.c list.c utils.c dh_prime.c break_code_c1.c

// real    0m12,479s
// user    0m29,177s
// sys     1m52,315s