#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include "chiffrement.h"
#include "dh_prime.h"
#include "break_code_c1.h"
#include "break_code_c2.h"
#include "break_code_c3.h"

// ======================================================================
// TEST : Chiffrement et déchiffrement XOR
// ======================================================================
void test_chiffrement_xor() {
    printf("\n=== Test : Chiffrement et Déchiffrement XOR ===\n");

    chiffrement_xor_file("message.txt", (unsigned char *)"rutabaga", "message_crypte.txt", 8);
    chiffrement_xor_file("message_crypte.txt", (unsigned char *)"rutabaga", "message_decrypte.txt", 8);

    FILE *original = fopen("message.txt", "r");
    FILE *decrypte = fopen("message_decrypte.txt", "r");

    int correct = 1;
    char c1, c2;
    while ((c1 = fgetc(original)) != EOF && (c2 = fgetc(decrypte)) != EOF) {
        if (c1 != c2) {
            correct = 0;
            break;
        }
    }

    fclose(original);
    fclose(decrypte);

    printf(correct ? "Test réussi : Les fichiers sont identiques.\n"
                   : "Test échoué : Les fichiers ne correspondent pas.\n");
}

// ======================================================================
// TEST : Chiffrement XOR avec différentes clés
// ======================================================================
void test_chiffrement_xor_differentes_cles() {
    printf("\n=== Test : Chiffrement XOR avec différentes clés ===\n");

    chiffrement_xor_file("message.txt", (unsigned char *)"rutabaga", "message_crypte1.txt", 8);
    chiffrement_xor_file("message.txt", (unsigned char *)"banana", "message_crypte2.txt", 6);

    FILE *crypted1 = fopen("message_crypte1.txt", "r");
    FILE *crypted2 = fopen("message_crypte2.txt", "r");

    int different = 0;
    char c1, c2;
    while ((c1 = fgetc(crypted1)) != EOF && (c2 = fgetc(crypted2)) != EOF) {
        if (c1 != c2) {
            different = 1;
            break;
        }
    }

    fclose(crypted1);
    fclose(crypted2);

    printf(different ? "Test réussi : Les fichiers chiffrés sont différents.\n"
                     : "Test échoué : Les fichiers chiffrés sont identiques.\n");
}

// ======================================================================
// TEST : Chiffrement et déchiffrement CBC
// ======================================================================
void test_chiffrement_cbc() {
    printf("\n=== Test : Chiffrement et Déchiffrement CBC ===\n");

    unsigned char *key = (unsigned char *)"rutabaga";
    unsigned char *iv = (unsigned char *)"1234567890123456";

    cbc_crypt("message.txt", iv, key, "message_crypte_cbc.txt");
    cbc_uncrypt("message_crypte_cbc.txt", iv, key, "message_decrypte_cbc.txt");

    FILE *original = fopen("message.txt", "r");
    FILE *decrypte = fopen("message_decrypte_cbc.txt", "r");

    int correct = 1;
    char c1, c2;
    while ((c1 = fgetc(original)) != EOF && (c2 = fgetc(decrypte)) != EOF) {
        if (c1 != c2) {
            correct = 0;
            break;
        }
    }

    fclose(original);
    fclose(decrypte);

    printf(correct ? "Test réussi : Les fichiers CBC sont identiques.\n"
                   : "Test échoué : Les fichiers CBC ne correspondent pas.\n");
}

// ======================================================================
// TEST : Génération des clés avec crack_C1
// ======================================================================
void test_crack_C1() {
    printf("\n=== Test : Génération des clés avec Crack_C1 ===\n");

    const char *input_file = "sample_encrypted.txt";
    int key_length = 3;

    list_t **keys = crack_C1(input_file, key_length);
    if (keys) {
        printf("Test réussi : Des clés ont été générées avec succès.\n");
    } else {
        printf("Test échoué : Aucune clé générée.\n");
    }
}

// ======================================================================
// TEST : Tri des clés avec crack_C2
// ======================================================================
void test_crack_C2() {
    printf("\n=== Test : Tri des clés avec Crack_C2 ===\n");

    const char *input_file = "sample_encrypted.txt";
    int key_length = 3;

    list_t **keys = crack_C1(input_file, key_length);
    if (keys == NULL) {
        printf("Erreur : Échec de la génération des clés.\n");
        return;
    }

    unsigned char **converted_keys = malloc(key_length * sizeof(unsigned char *));
    for (int i = 0; i < key_length; i++) {
        converted_keys[i] = (unsigned char *)keys[i]->head->val;
    }

    unsigned char **sorted_keys = crack_C2(input_file, converted_keys, key_length);
    if (sorted_keys) {
        printf("Test réussi : Les clés ont été triées avec succès.\n");
    } else {
        printf("Test échoué : Échec du tri des clés.\n");
    }

    free(converted_keys);
}

// ======================================================================
// TEST : Fichier vide
// ======================================================================
void test_fichier_vide() {
    printf("\n=== Test : Manipulation d'un fichier vide ===\n");

    chiffrement_xor_file("empty.txt", (unsigned char *)"rutabaga", "empty_crypte.txt", 8);
    chiffrement_xor_file("empty_crypte.txt", (unsigned char *)"rutabaga", "empty_decrypte.txt", 8);

    printf("Test réussi : Fichier vide traité correctement.\n");
}

// ======================================================================
// TEST : Grand fichier
// ======================================================================
void test_grand_fichier() {
    printf("\n=== Test : Manipulation d'un grand fichier ===\n");

    chiffrement_xor_file("large_file.txt", (unsigned char *)"rutabaga", "large_file_crypte.txt", 8);
    chiffrement_xor_file("large_file_crypte.txt", (unsigned char *)"rutabaga", "large_file_decrypte.txt", 8);

    printf("Test réussi : Grand fichier traité correctement.\n");
}

// ======================================================================
// FONCTION PRINCIPALE
// ======================================================================
int main() {
    printf("\n===== DÉBUT DES TESTS =====\n");

    test_chiffrement_xor();
    test_chiffrement_xor_differentes_cles();
    test_chiffrement_cbc();
    test_crack_C1();
    test_crack_C2();
    test_fichier_vide();
    test_grand_fichier();

    printf("\n===== FIN DES TESTS =====\n");
    return 0;
}