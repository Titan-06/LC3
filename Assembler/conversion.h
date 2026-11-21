#ifndef CONVERSION_H
#define CONVERSION_H
#include "helpers.h"
#include <stdint.h>

int is_num(char *num);

int handle_conversion(int token_code,list tokens,uint16_t * return_conversion);
int handle_directive(int token_code,list tokens,short_int_list * to_fill);

#endif