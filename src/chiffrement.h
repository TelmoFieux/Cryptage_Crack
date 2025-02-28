#ifndef CHIFFREMENT_H   // Protection contre l'inclusion multiple
#define CHIFFREMENT_H
#include <stdlib.h>
#include <stdio.h>


void chiffrement_xor(unsigned char* mess, unsigned char* key, unsigned char* crypted_mess, size_t message_len);
void chiffrement_xor_mask(char* mess, char* crypted_mess, size_t message_len);
void cbc_crypt(char* mess_file_name,unsigned char* vecteur, unsigned char* key, char* crypted_mess_file_name);
void cbc_uncrypt(char* mess_file_name,unsigned char* vecteur, unsigned char* key, char* uncrypted_mess_file_name);
void chiffrement_xor_file(char* mess_filename, unsigned char* key, char* crypted_mess_filename, int keysize);
void chiffrement_xor_mask_file(char* mess_filename, char* crypted_mess_filename);

#endif