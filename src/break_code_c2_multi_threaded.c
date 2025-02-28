#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "utils.h"
#include <time.h>
#include "list.h"
#include "break_code_c1_multi_threaded.h"
#include <ctype.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

float letter_frequencies[26] = {
    8.167, // A
    1.492, // B
    2.782, // C
    4.253, // D
    12.702, // E
    2.228, // F
    2.015, // G
    6.094, // H
    6.966, // I
    0.153, // J
    0.772, // K
    4.025, // L
    2.406, // M
    6.749, // N
    7.507, // O
    1.929, // P
    0.095, // Q
    5.987, // R
    6.327, // S
    9.056, // T
    2.758, // U
    0.978, // V
    2.360, // W
    0.150, // X
    1.974, // Y
    0.074  // Z
};

struct param{
    unsigned char** k;
    const char* f;
    int ks;
    int** res;
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
* Pour une clé et un fichier crypté, fourni la proportion de chaque lettre dans
le texte décodé
 *****************************************************************************/
int* get_car_frequency(unsigned char* key, const char* filename, int keysize){
    FILE *file = fopen(filename,"r");
    if (file == NULL){
        perror("Erreur à l'ouverture du fichier");
        exit(EXIT_FAILURE);
    }
    int* car_frequency = calloc(26, sizeof(int));
    if (car_frequency == NULL) {
        perror("Erreur d'allocation de mémoire");
        fclose(file);
        exit(EXIT_FAILURE);
    }
    int index=0;
    unsigned char reader[512];
    int elem_red;
    int car;

    while ((elem_red=fread(&reader,sizeof(unsigned char),sizeof(reader),file))!=0) {
        int indice=0;
        while (indice <elem_red){
            car = reader[indice]^key[index];
            if (isalpha(car)){
                car = tolower(car);
                car_frequency[car - 'a']++;
            }
            index=(index + 1)%keysize;
            indice++;
        }
    }
    fclose(file);
    return car_frequency;
}



/******************************************************************************
* Applique la fonction get_car_frequency pour toute les clés
 *****************************************************************************/
void get_all_frequency(const char* filename, unsigned char** keys, int NBkeys, int start, int end, int** car_frequency){
    int keysize = strlen((char*)keys[0]);
    int* val;
    for(int i=start; i<end; i++){
        val = get_car_frequency(keys[i], filename, keysize);
        pthread_mutex_lock(mutex_car);
        car_frequency[i] = val;
        pthread_mutex_unlock(mutex_car);
    }
}

static void* fils(void* arg){
    struct param* p = arg;
    get_all_frequency(p->f,p->k,p->ks,p->start,p->end,p->res);
    regroupement();
    pthread_exit(NULL);
}


/******************************************************************************
* A partir des fréquence de chaque lettre, pour chaque clé, on va calculer
la somme des écarts à la fréquence moyenne. On utilisera ensuite cette valeur
pour effectuer un tri afin de ranger les clé dans l'ordre des décroissant. 
Des plus proches de la fréquence observé aux plus éloignés.
 *****************************************************************************/
unsigned char** crack_C2(const char* filename, unsigned char** keys, int NBkeys, int NBThreads){

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

    int** car_frequency = malloc(NBkeys * sizeof(int*));
    if (car_frequency==NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }


    struct param** p= malloc(sizeof(struct param*)*NBThreads);
    pthread_t pid[NBthreads];

    int base = NBkeys / NBThreads;
    int extra = NBkeys % NBThreads;
    int current_start = 0;          // Point de départ pour chaque thread
    int err;
    for (int i = 0; i < NBthreads; i++) {
        p[i] = malloc(sizeof(struct param));
        if (!p[i]) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        p[i]->f = filename;
        p[i]->k = keys;
        p[i]->ks = NBkeys;
        p[i]->res = car_frequency;

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

    struct freq_index frequency[NBkeys];
    unsigned char** best_keys = malloc(NBkeys * sizeof(unsigned char*));
    int mess_size = get_file_size(filename);
    int keysize = strlen((char*)keys[0]);
    for(int i=0; i<NBkeys; i++){
        float freq = 0;
        for(int j=0; j<26; j++){
            double observed_freq = (double)car_frequency[i][j] / mess_size;  // Ensure floating-point division
            double diff = letter_frequencies[j] - observed_freq;
            freq += diff * diff;
        }
        frequency[i].frequency = freq;
        frequency[i].i = i;

        // Allocate memory for each key copy
        best_keys[i] = malloc((keysize + 1) * sizeof(unsigned char));
        if (best_keys[i] == NULL) {
            perror("Memory allocation failed");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < NBkeys; i++) {
        free(car_frequency[i]);
    }
    free(car_frequency);
    for (int i = 0; i <NBThreads; i++){
        free(p[i]);
    }
    free(p);

    quicksort_iterative(frequency, 0, NBkeys - 1);

    for (int i=0; i<NBkeys; i++){
        strcpy((char*)best_keys[i], (char*)keys[frequency[i].i]);
    }

    return best_keys;
}

// int main(void){
//     list_t** key = crack_C1("1234_msg2.txt", 4);
//     int size;
//     unsigned char** combinations = generate_combinations(key, 4, &size);
//     for (int i = 0; i < size; i++) {
//         // printf("%s\n", combinations[i]);
//     }
//     unsigned char** sorted_keys = crack_C2("1234_msg2.txt", combinations, size, 1);
//     for (int i = size-1; i>=0; i--) {
//         // printf("%s\n", sorted_keys[i]);
//         free(sorted_keys[i]);  // Free each key
//     }
//     free(sorted_keys);  // Free the array of pointers
//     return 0;
// }