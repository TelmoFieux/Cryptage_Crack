#ifndef BREAK_CODE_C1_H   // Protection contre l'inclusion multiple
#define BREAK_CODE_C1_H
#include <stdlib.h>
#include <stdbool.h>
#include "list.h"

list_t** crack_C1(const char* crypted_mess_filename, int keysize);
unsigned char** generate_combinations(list_t** key, int keysize, int* result_size);


#endif