#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include "break_code_c1_multi_threaded.h"
#include "break_code_c2_multi_threaded.h"
#include "break_code_c3_multi_threaded.h"


int main(int argc, char **argv){
    if (argc<5){
        printf("Not enough arguments\n");
        printf("Usage: ./break_code -i input_filename -m methode (c1,all) -k keysize -d dictionary_filename -l log_filename -t nbthreads\n");
        return 0;
    }

    char *input_filename = NULL;
    char *dico_filename = NULL;
    int keysize = 0;
    char *method = NULL;
    int threads;

    // Lecture des options de la ligne de commande
    int opt;
    while ((opt = getopt(argc, argv, "i:k:d:m:l:t:")) != -1) {
        switch (opt) {
            case 'h':
                printf("Usage: ./break_code -i input_filename -m methode (c1,all) -k keysize -d dictionary_filename -l log_filename -t nbthreads\n");
                return 0;
            case 'i':
                input_filename = optarg;
                break;
            case 'd':
                dico_filename = optarg;
                break;
            case 'k':
                keysize = atoi(optarg);
                break;
            case 'm':
                method = optarg;
                break;
            case 'l':
                break;
            case 't':
                threads=atoi(optarg);
                break;
            default:
                fprintf(stderr, "Usage: %s -i input_file -o output_file -k key|-f key_file -m method -v vector_file\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (strcmp(method, "c1")==0){
        printf("Using crack C1\n");
        list_t** key =crack_C1(input_filename,keysize);
        int size;
        unsigned char** combinations = generate_combinations(key,keysize, &size);
        for (int i = 0; i < size; i++) {
            printf("%s\n", combinations[i]);
            free(combinations[i]);
        }
        for (int i=0; i<keysize; i++) {
            for(int j = 0; j < key[i]->len; j++) {
                free(list_at(key[i],j)->val);
            }
            list_destroy(key[i]);
        }
        free(key);
        free(combinations);
        printf("Nombre de clef générés : %d\n", size);
        return 0;
    }else if(strcmp(method, "all")==0){
        if (dico_filename==NULL){
            printf("Not enough arguments\n");
            printf("Usage: ./break_code -i input_filename -m methode (c1,all) -k keysize -d dictionary_filename -l log_filename\n");
            return 0;
        }
        printf("Using crack C1 C2 and C3\n");
        list_t** key =crack_C1(input_filename,keysize);
        int size;
        unsigned char** combinations = generate_combinations(key,keysize, &size);
        printf("Trie selon la fréquence des caractères : \n");
        unsigned char** new_keys = crack_C2(input_filename, combinations, size,threads);
        for (int i = size-1; i >=0; i--) {
            printf("%s\n", new_keys[i]);
        }
        unsigned char** sorted_keys = crack_c3(input_filename, new_keys, size,dico_filename,threads);
        printf("Trie selon la proportion de mots corrects : \n");
        for (int i = size-1; i >=0; i--) {
            printf("%s\n", sorted_keys[i]);
            free(sorted_keys[i]);
            free(combinations[i]);
            free(new_keys[i]);
        }
        for (int i=0; i<keysize; i++) {
            for(int j = 0; j < key[i]->len; j++) {
                free(list_at(key[i],j)->val);
            }
            list_destroy(key[i]);
        }
        free(key);
        free(new_keys);
        free(combinations);
        free(sorted_keys);
    }else{
        printf("You did not choose a valid option");
        return 0;
    }

    return 0;

}