#ifndef BREAK_CODE_C3_H   // Protection contre l'inclusion multiple
#define BREAK_CODE_C3_H
#include <stdlib.h>
#include <stdbool.h>

unsigned char** crack_c3(const char* filename, unsigned char** keys, int NBkeys,const char* dico_filename);


#endif