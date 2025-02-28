#ifndef BREAK_CODE_C2_MULTI_THREADED_H   // Protection contre l'inclusion multiple
#define BREAK_CODE_C2_MULTI_THREADED_H
#include <stdlib.h>
#include <stdbool.h>

unsigned char** crack_C2(const char* filename, unsigned char** keys, int NBkeys, int NBThreads);


#endif