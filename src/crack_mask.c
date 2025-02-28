#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "chiffrement.h"
#include "break_code_c1.h"
#include "break_code_c2.h"
#include "break_code_c3.h"



/******************************************************************************
* Extrait tout le contenu d'un fichier
 *****************************************************************************/
unsigned char* get_key(char * filename, int* size){
    FILE *key_file = fopen(filename, "r");
    if (key_file == NULL) {
        perror("Erreur à l'ouverture du fichier");
        exit(EXIT_FAILURE);
    }
    fseek(key_file, 0, SEEK_END);
    int taille_fichier = ftell(key_file); // Récupérer la taille en octets
    *size=taille_fichier;
    rewind(key_file); // Revenir au début du fichier

    // Allouer de la mémoire pour stocker la clé (ajouter 1 pour le caractère nul)
    unsigned char* key = malloc((taille_fichier + 1) * sizeof(char));
    if (key == NULL) {
        perror("Erreur d'allocation mémoire");
        exit(EXIT_FAILURE);
    }

    fread(key, sizeof(char), taille_fichier, key_file);
    key[taille_fichier] = '\0';
    fclose(key_file);
    return key;
}


int main(int argc, char **argv){
    if (argc<5){
        printf("Not enough arguments\n");
        printf("Usage: ./crack_mask chiffré1 chiffré2 clair1 clairRésultat\n");
        return 0;
    }

    char *crypted1 = argv[1];
    char *crypted2 = argv[2];
    char *plain1 = argv[3];
    char *plainResult = argv[4];
    unsigned char *key1;
    unsigned char *key2;
    int size;
    key1=get_key(crypted1,&size);
    chiffrement_xor_file(crypted2,key1,"temporary",size);
    free(key1);
    key2=get_key(plain1,&size);
    chiffrement_xor_file("temporary",key2,plainResult,size);
    free(key2);

}