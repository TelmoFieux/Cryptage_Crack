#include "utils.h"
#include <stdlib.h>
#include <string.h>
#include "dh_prime.h"
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

/******************************************************************************
 * Génère un nouvelle clé de taille newlen à partir de l'ancienne clé qui sera
 * cyclé autant que nécessaire
 * Exemple : padding("rutabaga", 16) = "rutabagarutabaga"
 *****************************************************************************/
unsigned char* padding(unsigned char *data,size_t newlen){
    unsigned char* newdata = malloc(newlen);
    size_t len_data=strlen((const char*) data);
    for(int i =0; i<(int)newlen; i++){
        newdata[i]=data[(i%len_data)];
    }
    return newdata;
}


/******************************************************************************
 * Génère un clé de longueur len composé de caractère alphanumérique aléatoires
 *****************************************************************************/
unsigned char* gen_key(long len){
    srand(time(NULL));
    unsigned char* newdata = malloc(sizeof(unsigned char)*len);
    for(int i =0; i<len; i++){
        newdata[i]=random_long(0,127);
    }
    return newdata;
}


unsigned char* get_alpha_car(){
    unsigned char* alphanum = malloc(sizeof(unsigned char)*63);  // 26 lettres majuscules + 26 lettres minuscules + 10 chiffres = 62
    int index = 0;

    // Ajouter les lettres majuscules
    for (char c = 'A'; c <= 'Z'; c++) {
        alphanum[index++] = c;
    }

    // Ajouter les lettres minuscules
    for (char c = 'a'; c <= 'z'; c++) {
        alphanum[index++] = c;
    }

    // Ajouter les chiffres
    for (char c = '0'; c <= '9'; c++) {
        alphanum[index++] = c;
    }

    // Terminer le tableau avec un caractère nul
    alphanum[index] = '\0';
    return alphanum;
}

bool is_alpha(int car) {
    return ((car >= '0' && car <= '9') ||  // Digits
            (car >= 'A' && car <= 'Z') ||  // Uppercase
            (car >= 'a' && car <= 'z') ||  // Lowercase
            (car == '.' || car == ',' || car == '?' || 
             car == ' ' || car == '!' || car == ';' || car == '\'' ||
             car == 10)); // Punctuation
}

// Function to swap two elements
void swap(struct freq_index* a, struct freq_index* b) {
    struct freq_index temp = *a;
    *a = *b;
    *b = temp;
}

// Function to partition the array and return the pivot index
int partition(struct freq_index arr[], int low, int high) {
    float pivot = arr[low].frequency;
    int i = low;
    int j = high;

    while (i < j) {
        // Find the first element greater than the pivot from the left
        while (arr[i].frequency <= pivot && i <= high) {
            i++;
        }

        // Find the first element smaller than the pivot from the right
        while (arr[j].frequency > pivot && j >= low) {
            j--;
        }

        // Swap the elements at indices i and j
        if (i < j) {
            swap(&arr[i], &arr[j]);
        }
    }

    // Swap the pivot element with the element at index j
    swap(&arr[low], &arr[j]);

    return j;
}

void quicksort_iterative(struct freq_index arr[], int low, int high) {
    int stack[high - low + 1];
    int top = -1;

    stack[++top] = low;
    stack[++top] = high;

    while (top >= 0) {
        high = stack[top--];
        low = stack[top--];

        int pivot = partition(arr, low, high);

        if (pivot - 1 > low) {
            stack[++top] = low;
            stack[++top] = pivot - 1;
        }

        if (pivot + 1 < high) {
            stack[++top] = pivot + 1;
            stack[++top] = high;
        }
    }
}


void find_invalid_chars(const char* filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Erreur à l'ouverture du fichier");
        exit(EXIT_FAILURE);
    }

    int car;
    printf("Characters not covered by is_alpha:\n");
    while ((car = fgetc(file)) != EOF) {
        if (!is_alpha(car)) {
            printf("Character: %c (ASCII: %d)\n", car, car);
        }
    }

    fclose(file);
}


long get_file_size(const char* filename) {
    FILE* file = fopen(filename, "rb"); // Open in binary mode
    if (file == NULL) {
        perror("Error opening file");
        return -1;
    }

    fseek(file, 0, SEEK_END); // Move to the end of the file
    long size = ftell(file); // Get the current file pointer position
    fclose(file);

    return size;
}

