#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "utils.h"
#include <time.h>
#include "list.h"
#include "break_code_c1.h"
#include <ctype.h>

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
    int car;
    while ((car = fgetc(file))!=EOF) {
        car = car^key[index];
        if (isalpha(car)){
            car = tolower(car);
            car_frequency[car - 'a']++;
        }
        index=(index + 1)%keysize;
    }
    fclose(file);
    return car_frequency;
}



/******************************************************************************
* Applique la fonction get_car_frequency pour toute les clés
 *****************************************************************************/
int** get_all_frequency(const char* filename, unsigned char** keys, int NBkeys){
    int** car_frequency = malloc(NBkeys * sizeof(int*));
    int keysize = strlen((char*)keys[0]);
    for(int i=0; i<NBkeys; i++){
        car_frequency[i] = get_car_frequency(keys[i], filename, keysize);
    }
    return car_frequency;
}


/******************************************************************************
* A partir des fréquence de chaque lettre, pour chaque clé, on va calculer
la somme des écarts à la fréquence moyenne. On utilisera ensuite cette valeur
pour effectuer un tri afin de ranger les clé dans l'ordre des décroissant. 
Des plus proches de la fréquence observé aux plus éloignés.
 *****************************************************************************/
unsigned char** crack_C2(const char* filename, unsigned char** keys, int NBkeys){
    int** car_frequency = get_all_frequency(filename, keys, NBkeys);
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

    quicksort_iterative(frequency, 0, NBkeys - 1);

    for (int i=0; i<NBkeys; i++){
        strcpy((char*)best_keys[i], (char*)keys[frequency[i].i]);
    }

    return best_keys;
}

// int main(void){
//     list_t** key = crack_C1("crypt", 8);
//     int size;
//     unsigned char** combinations = generate_combinations(key, 8, &size);
//     for (int i = 0; i < size; i++) {
//         printf("%s\n", combinations[i]);
//     }
//     unsigned char** sorted_keys = crack_C2("crypt", combinations, size);
//     for (int i = 0; i < size; i++) {
//         printf("%s\n", sorted_keys[i]);
//         free(sorted_keys[i]);  // Free each key
//     }
//     free(sorted_keys);  // Free the array of pointers
// }