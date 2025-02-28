#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "chiffrement.h"
#include <getopt.h>
#include <unistd.h>


int main(int argc, char **argv){
    if (argc<5){
        printf("Not enough arguments\n");
        printf("Usage: ./sym_crypt -i input_filename -o output_filename -m methode (cbc-crypt,xor,mask,cbc-uncrypt) -k key -f key_filename -v vecteur_filename\n");
        return 0;
    }

    char *input_filename = NULL;
    char *output_filename = NULL;
    unsigned char *key = NULL;
    char *key_filename = NULL;
    char *method = NULL;
    unsigned char *vecteur = NULL;
    char *vecteur_filename = NULL;

    // Lecture des options de la ligne de commande
    int opt;
    while ((opt = getopt(argc, argv, "i:o:k:f:m:v:")) != -1) {
        switch (opt) {
            case 'h':
                printf("Usage: ./sym_crypt -i input_filename -o output_filename -m methode (cbc-crypt,xor,mask,cbc-uncrypt) -k key -f key_filename -v vecteur_filename");
                return 0;
            case 'i':
                input_filename = optarg;
                break;
            case 'o':
                output_filename = optarg;
                break;
            case 'k':
                key = (unsigned char*)optarg;  // La clé est passée directement
                break;
            case 'f':
                key_filename = optarg;  // La clé est dans un fichier
                FILE *key_file = fopen(key_filename, "r");
                if (key_file == NULL) {
                    perror("Erreur à l'ouverture du fichier");
                    exit(EXIT_FAILURE);
                }
                fseek(key_file, 0, SEEK_END);
                int taille_fichier = ftell(key_file); // Récupérer la taille en octets
                rewind(key_file); // Revenir au début du fichier

                // Allouer de la mémoire pour stocker la clé (ajouter 1 pour le caractère nul)
                key = malloc((taille_fichier + 1) * sizeof(char));
                if (key == NULL) {
                    perror("Erreur d'allocation mémoire");
                    exit(EXIT_FAILURE);
                }

                fread(key, sizeof(char), taille_fichier, key_file);
                key[taille_fichier] = '\0';
                fclose(key_file);
                break;
            case 'm':
                method = optarg;  // Méthode de chiffrement/déchiffrement
                break;
            case 'v':
                vecteur_filename = optarg;  // Fichier contenant le vecteur d'initialisation
                FILE *vecteur_file = fopen(vecteur_filename, "r");
                if (vecteur_file == NULL) {
                    perror("Erreur à l'ouverture du fichier");
                    exit(EXIT_FAILURE);
                }
                fseek(vecteur_file, 0, SEEK_END);
                taille_fichier = ftell(vecteur_file); // Récupérer la taille en octets
                rewind(vecteur_file); // Revenir au début du fichier

                // Allouer de la mémoire pour stocker la clé (ajouter 1 pour le caractère nul)
                vecteur = malloc((taille_fichier + 1) * sizeof(char));
                if (vecteur == NULL) {
                    perror("Erreur d'allocation mémoire");
                    exit(EXIT_FAILURE);
                }

                fread(vecteur, sizeof(char), taille_fichier, vecteur_file);
                vecteur[taille_fichier] = '\0';
                fclose(vecteur_file);
                break;
            default:
                fprintf(stderr, "Usage: %s -i input_file -o output_file -k key|-f key_file -m method -v vector_file\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (strcmp(method, "xor")==0){
        printf("Using methode xor\n");
        chiffrement_xor_file(input_filename,key, output_filename, (int)strlen(key));
    }else if(strcmp(method, "mask")==0){
        printf("Using methode mask\n");
        chiffrement_xor_mask_file(input_filename,output_filename);
    }else if(strcmp(method, "cbc-crypt")==0){
        printf("Using methode cbc-crypt\n");
        cbc_crypt(input_filename, vecteur, key, output_filename);
    }else if(strcmp(method,"cbc-uncrypt")==0){
        printf("Using methode cbc-uncrypt\n");
        cbc_uncrypt(input_filename, vecteur, key, output_filename);
    }else{
        printf("You did not choose a valid option");
        return 0;
    }

    return 0;

}