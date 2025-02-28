#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "utils.h"
#include <time.h>
#include "list.h"
#include <ctype.h>


/******************************************************************************
 * Pour chaque position de la clé donne les caractère qui produisent un caractère 
 * lisible une fois le xor effectué avec le fichier crypté.
 *****************************************************************************/
list_t** crack_C1(const char* crypted_mess_filename, int keysize){
    FILE *crypted_mess_file = fopen(crypted_mess_filename,"rb");
    if (crypted_mess_file == NULL){
        perror("Erreur à l'ouverture du fichier");
        exit(EXIT_FAILURE);
    }
    unsigned char test_car;
    unsigned char reader;    
    unsigned char* alphanum=get_alpha_car();
    bool valid=true;
    int position;
    // Obtenir la taille totale du fichier
    fseek(crypted_mess_file, 0, SEEK_END);
    long taille_fichier = ftell(crypted_mess_file);
    rewind(crypted_mess_file); // Retour au début
    list_t** key=malloc(sizeof(list_t*) * keysize);
    if(key == NULL){
        perror("Erreur à l'allocation de la mémoire");
        exit(EXIT_FAILURE);
    }
    for (int k = 0; k < keysize; k++) {
        key[k] = list_new();
        if (key[k] == NULL) {
            // Clean up previously allocated lists
            for (int j = 0; j < k; j++) {
                list_destroy(key[j]);
            }
            free(key);
            fclose(crypted_mess_file);
            return NULL;
        }
    }
    for (int i = 0; i<63; i++){
        fflush(stdout);
        test_car = alphanum[i];
        for (int j = 0; j<keysize; j++){
            valid=true;
            fseek(crypted_mess_file,j,SEEK_SET);
            while ((fread(&reader,sizeof(unsigned char),1,crypted_mess_file))!=0) {
                if (!isalnum(test_car^reader) && !ispunct(test_car^reader) && !isspace(test_car^reader)){
                    valid = false;
                    break;
                }
                fseek(crypted_mess_file,keysize-1,SEEK_CUR);
                // Vérifier la position
                position = ftell(crypted_mess_file);
                if (position >= taille_fichier) {
                    break;
                }
            }
            if (valid){
                unsigned char* car = malloc(sizeof(unsigned char));
                if (car==NULL){
                    perror("Erreur à l'allocation de la mémoire");
                    exit(EXIT_FAILURE);
                }
                *car=test_car;
                list_rpush(key[j],list_node_new((void*)car));
            }
        }
    }
    free(alphanum);
    fclose(crypted_mess_file);
    return key;
}

/******************************************************************************
* Génère le produit cartésiens de toute les caractère obtenu par la fonction crack_C1
 *****************************************************************************/
unsigned char** generate_combinations(list_t** key, int keysize, int* result_size) {
    int nbCombinations = key[0]->len;
    for (int i = 1; i < keysize; i++) {
        nbCombinations = nbCombinations * key[i]->len;
    }
    *result_size = nbCombinations;

    unsigned char** Combinations = malloc(sizeof(unsigned char*) * nbCombinations);
    unsigned char* currentCombination = malloc(keysize + 1);
    int* indices = calloc(keysize, sizeof(int));

    for (int i = 0; i < nbCombinations; i++) {
        // Construire la combinaison courante
        for (int j = 0; j < keysize; j++) {
            currentCombination[j] = *(unsigned char*)list_at(key[j], indices[j])->val;
        }
        currentCombination[keysize] = '\0'; // Terminer la chaîne

        // Allouer et copier manuellement pour éviter le problème de signedness
        Combinations[i] = malloc(keysize + 1);
        memcpy(Combinations[i], currentCombination, keysize + 1);

        // Mettre à jour les indices pour passer à la combinaison suivante
        for (int j = keysize - 1; j >= 0; j--) {
            indices[j]++;
            if (indices[j] < (int)key[j]->len) {
                break; // On reste dans les limites, pas besoin d'incrémenter les tableaux précédents
            }
            indices[j] = 0; // Réinitialiser cet indice et passer au tableau précédent
        }
    }

    free(currentCombination);
    free(indices);
    return Combinations;
}

// int main(void){
//     list_t** key =crack_C1("crypt",8);
//     int size;
//     unsigned char** combinations = generate_combinations(key,8, &size);
//     for (int i = 0; i < size; i++) {
//         printf("%s\n", combinations[i]);
//     }
//     return 0;
    
// }