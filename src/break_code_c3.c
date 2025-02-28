#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include <ctype.h>
#include <stdbool.h>
#include "list.h"
#include "break_code_c1.h"

#define MAX_WORD_LENGTH 100  // Longueur maximale d'un mot dans le dictionnaire

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
    unsigned char word[46]={0}; //le plus long mot de la langue anglaise a part des nom de molécule chimique
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
        word[index_letter] = '\0';
        if(binary_search_in_file(dico_filename,(const char*)word)) nb_correct_words++;
        else nb_incorrect_words++;
    }
    fclose(file);
    return nb_correct_words/(nb_correct_words+nb_incorrect_words);
}



/******************************************************************************
* Trie les clés en fonction de la proportion de mot correcte une fois
le texte décodé.
 *****************************************************************************/
unsigned char** crack_c3(const char* filename, unsigned char** keys, int NBkeys,const char* dico_filename){
    struct freq_index frequency[NBkeys];
    unsigned char** sorted_keys = malloc(NBkeys * sizeof(unsigned char*));
    int keysize = strlen((char*)keys[0]);
    float freq = 0;
    for(int i=0; i<NBkeys; i++){
        freq=get_correct_words(keys[i],filename,keysize,dico_filename);
        frequency[i].frequency = freq;
        frequency[i].i = i;

        // Allocate memory for each key copy
        sorted_keys[i] = malloc((keysize + 1) * sizeof(unsigned char));
        if (sorted_keys[i] == NULL) {
            perror("Memory allocation failed");
            exit(EXIT_FAILURE);
        }
    }

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
//         printf("%s\n", combinations[i]);
//     }
//     unsigned char** sorted_keys = crack_c3("1234_msg2.txt", combinations, size,"../dictionary.txt");
//     for (int i = 0; i < size; i++) {
//         printf("%s\n", sorted_keys[i]);
//         free(sorted_keys[i]);  // Free each key
//         free(combinations[i]); // Free each combination
//     }
//     free(combinations);  // Free the array of pointers
//     free(sorted_keys);  // Free the array of pointers
// }

//compilation : gcc -Wall -Wextra -o crack3 break_code_c3.c list.c utils.c dh_prime.c break_code_c1.c