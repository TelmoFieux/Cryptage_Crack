#ifndef UTILS_H   // Protection contre l'inclusion multiple
#define UTILS_H
#include <stdlib.h>
#include <stdbool.h>

struct freq_index {
    int i;
    float frequency;
};

unsigned char* padding(unsigned char *data,size_t newlen);
unsigned char* gen_key(long len);
unsigned char* get_alpha_car();
bool is_alpha(int car);
void quicksort_iterative(struct freq_index arr[], int low, int high);
void find_invalid_chars(const char* filename);
long get_file_size(const char* filename);

#endif