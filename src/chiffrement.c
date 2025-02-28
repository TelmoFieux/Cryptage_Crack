#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "utils.h"
#include <time.h>

void chiffrement_xor(char* mess, unsigned char* key, char* crypted_mess, size_t message_len){
    unsigned char* pad_key=padding(key,message_len);
    for(int i=0;i<message_len;i++){
        crypted_mess[i]=mess[i]^pad_key[i];
    }
    free(pad_key);
}

/******************************************************************************
 * Génère un message crypté ou décrypté écris dans le fichier crypted_mess_filename a partir
 * d'une clé et d'un message se trouvant dans mess_filename avec la méthode xor
 *****************************************************************************/
void chiffrement_xor_file(char* mess_filename, unsigned char* key, char* crypted_mess_filename, int keysize){
    FILE *mess_file = fopen(mess_filename,"r");
    FILE *crypted_mess_file = fopen(crypted_mess_filename,"w+");
    if (mess_file == NULL || crypted_mess_file == NULL){
        perror("Erreur à l'ouverture du fichier");
        exit(EXIT_FAILURE);
    }
    unsigned char reader[keysize];
    size_t elem_red=0;
    unsigned char encoded_char[keysize+1];
    printf("sizeof key = %d\n", keysize);
    while ((elem_red=fread(reader,sizeof(char),keysize,mess_file))!=0) {
        printf("%d elem red\n",elem_red);
        for (int k = 0; k<elem_red;k++){
            encoded_char[k]=reader[k]^key[k];
        }
        encoded_char[elem_red]='\0';
        fwrite(encoded_char, sizeof(char), elem_red, crypted_mess_file);
    }
    fclose(mess_file);
    fclose(crypted_mess_file);
}

void chiffrement_xor_mask(char* mess, char* crypted_mess, size_t message_len){
    unsigned char* key= gen_key(message_len);
    for(int i=0;i<message_len;i++){
        crypted_mess[i]=mess[i]^key[i];
    }
    free(key);
}


/******************************************************************************
 * Génère un message crypté écris dans le fichier crypted_mess_filename a partir
 * d'une clé généré aléatoirement et d'un message se trouvant dans mess_filename
 * avec la méthode xor avec mask aléatoire jetable.
 * la clé généré aléatoirement est stocké dans le fichier keyfile.txt
 * Pour décrypté le message généré, utiliser chiffrement_xor_file avec comme clé
 * la clé qui se trouve dans le fichier keyfile.txt
 *****************************************************************************/
void chiffrement_xor_mask_file(char* mess_filename,char* crypted_mess_filename){
    FILE *mess_file = fopen(mess_filename,"r");
    FILE *crypted_mess_file = fopen(crypted_mess_filename,"w+");
    if (mess_file == NULL || crypted_mess_file == NULL){
        perror("Erreur à l'ouverture du fichier");
        exit(EXIT_FAILURE);
    }
    fseek(mess_file, 0, SEEK_END);
    long taille_fichier = ftell(mess_file); 
    rewind(mess_file);
    unsigned char* key=gen_key(taille_fichier);
    unsigned char reader[20];
    unsigned char encoded_char[21];
    size_t elem_red=0;
    //printf("taille fichier : %d\n", taille_fichier);
    int index_key=0;
    while ((elem_red=fread(reader,sizeof(char),20,mess_file))!=0) {
        //printf("%d elem red\n",elem_red);
        fflush(stdout);
        for (int k = 0; k<elem_red;k++){
            encoded_char[k]=reader[k]^key[index_key];
            index_key++;
        }
        encoded_char[elem_red]='\0';
        fwrite(encoded_char, sizeof(char), elem_red, crypted_mess_file);
    }
    FILE *key_file = fopen("keyfile.txt","w+");
    fwrite(key,sizeof(char),taille_fichier,key_file);
    fclose(key_file);
    fclose(mess_file);
    fclose(crypted_mess_file);
    free(key);
}

void chiffrement_CBC(char* mess, char* crypted_mess, size_t message_len, size_t bloc_len, char* vecteur, unsigned char* key){
    unsigned char bloc[bloc_len];
    for (int i = 0; i < message_len; i += bloc_len){
        if (i + bloc_len < message_len) {
            for (int k = 0; k<bloc_len;k++){
                bloc[k]=mess[i+k]^vecteur[k]^key[k];
                crypted_mess[i+k]=bloc[k];

            }
        } else {
            for (int k = 0; k<message_len-i;k++){
                bloc[k]=mess[i+k]^vecteur[k]^key[k];
                crypted_mess[i+k]=bloc[k];
            }
            for (int k = message_len-i; k<bloc_len;k++){
                bloc[k]=' '^vecteur[k]^key[k];
                crypted_mess[i+k]=bloc[k];
            }
        }
        vecteur=bloc;
    }
}


/******************************************************************************
 * Génère un message crypté écris dans le fichier crypted_mess_filename a partir
 * d'une clé, d'un vecteur et d'un message se trouvant dans mess_filename
 * avec la méthode CBC
 *****************************************************************************/
void cbc_crypt(char* mess_file_name, unsigned char* vecteur, unsigned char* key, char* crypted_mess_file_name){
    FILE *mess_file = fopen(mess_file_name,"r");
    FILE *crypted_mess_file = fopen(crypted_mess_file_name,"w+");
    if (mess_file == NULL || crypted_mess_file == NULL){
        perror("Erreur à l'ouverture du fichier");
        exit(EXIT_FAILURE);
    }
    unsigned char* padded_vecteur = padding(vecteur, 16);
    unsigned char* padded_key = padding(key, 16);
    unsigned char bloc[16];
    unsigned char reader[16];
    size_t elem_red=0;
    while ((elem_red=fread(reader,sizeof(char),16,mess_file))!=0) {
        reader[16]='\0';
        for (int k = 0; k<16;k++){
            bloc[k]=reader[k]^padded_vecteur[k]^padded_key[k];
        }
        fwrite(bloc, sizeof(char), elem_red, crypted_mess_file);
        memcpy(padded_vecteur, bloc, 16);
    }
    free(padded_vecteur);
    fclose(mess_file);
    fclose(crypted_mess_file);
    free(padded_key);
}


/******************************************************************************
 * Génère un message décrypté écris dans le fichier uncrypted_mess_filename a partir
 * d'une clé identique a celle utilisé pour le cryptage,
 * d'un vecteur identique aussi a celui utilisé pour le cryptage 
 * et d'un message se trouvant dans mess_filename
 * avec la méthode CBC
 *****************************************************************************/
void cbc_uncrypt(char* mess_file_name, unsigned char* vecteur, unsigned char* key, char* uncrypted_mess_file_name){
    FILE *mess_file = fopen(mess_file_name,"r");
    FILE *uncrypted_mess_file = fopen(uncrypted_mess_file_name,"w");
    if (mess_file == NULL || uncrypted_mess_file == NULL){
        perror("Erreur à l'ouverture du fichier");
        exit(EXIT_FAILURE);
    }
    vecteur=padding(vecteur,16);
    key=padding(key,16);
    unsigned char bloc[16];
    unsigned char reader[16];
    size_t elem_red=0;
    while ((elem_red=fread(reader,sizeof(char),16,mess_file))!=0) {
        reader[16]='\0';
        for (int k = 0; k<16;k++){
            bloc[k]=reader[k]^key[k]^vecteur[k];
            vecteur[k]=reader[k];
        }
        fwrite(bloc, sizeof(char), elem_red, uncrypted_mess_file);
    }
    fclose(mess_file);
    fclose(uncrypted_mess_file);
    free(vecteur);
    free(key);
}

void dechiffrement_CBC(char* mess, char* crypted_mess, size_t message_len, size_t bloc_len, char* vecteur, unsigned char* key){
    unsigned char bloc[bloc_len];
    for (int i = 0; i < message_len; i += bloc_len){
        if (i + bloc_len < message_len) {
            for (int k = 0; k<bloc_len;k++){
                crypted_mess[i+k]=mess[i+k]^key[k]^vecteur[k];
                bloc[k]=mess[i+k];
            }
        } else {
            for (int k = 0; k<message_len-i;k++){
                crypted_mess[i+k]=mess[i+k]^key[k]^vecteur[k];
                bloc[k]=mess[i+k];
            }
            for (int k = message_len-i; k<bloc_len;k++){
                bloc[k]=' '^key[k]^vecteur[k];
                bloc[k]=mess[i+k];
            }
        }
        vecteur=bloc;
    }
}


/*
int main(int argc, char **argv){
    srand(time(NULL));
    int bloc_size = 16; 
    unsigned char mess[]= "Les carottes sont cuites, et les patates sont molles ! I'm blue dabedidabedaedabedidabeda";
    size_t message_len = strlen((char *)mess); 
    unsigned char* key= gen_key(bloc_size);
    unsigned char* vecteur= gen_key(bloc_size);
    int crypted_mess_len =message_len+(bloc_size-(message_len%bloc_size));
    unsigned char crypted_mess[crypted_mess_len];
    printf("%d",crypted_mess_len);
    printf("clé (en hexadécimal) :\n");
    for (size_t i = 0; i < strlen(key); i++) {
        printf("%02x ", key[i]);  // Affichage en hexadécimal
    }
    printf("\n");
    
    //chiffrement_CBC(mess,crypted_mess,message_len,bloc_size, vecteur, key);
    //chiffrement_xor(mess,key,crypted_mess,message_len);
    //cbc_crypt("message.txt",vecteur, key, "crypted_mess.txt");
    printf("Message chiffré (en hexadécimal) :\n");
    for (size_t i = 0; i < message_len; i++) {
        printf("%02x ", crypted_mess[i]);  // Affichage en hexadécimal
    }
    printf("\n");

    unsigned char uncrypted_mess[message_len];
    //dechiffrement_CBC(crypted_mess,uncrypted_mess,message_len,bloc_size, vecteur, key);
    //chiffrement_xor(crypted_mess,key,uncrypted_mess,message_len);
    //cbc_uncrypt("crypted_mess.txt",vecteur, key, "decrypted_mess.txt");
    chiffrement_xor_mask_file("message.txt","crypted_mess.txt");
    FILE *key_file = fopen("keyfile.txt", "r");
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
    chiffrement_xor_file("crypted_mess.txt",key,"decrypted_mess.txt");

    printf("Message déchiffré : ");
    for (size_t i = 0; i < message_len; i++) {
        printf("%c", uncrypted_mess[i]);  // Affichage caractère par caractère
    }
    printf("\n");
    return 0;

}*/