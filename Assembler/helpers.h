#ifndef DEFS_H
#define DEFS_H

#include <stdint.h>

#define VALID_TOKENS 33 // .ORIG and .END are not counted

// ENUMS
enum OPCODES{ // 15 OPCODES in LC 3 and One reserved code
    OP_BR = 0,
    OP_ADD,
    OP_LD,
    OP_ST,
    OP_JSR,
    OP_AND,
    OP_LDR,
    OP_STR,
    OP_RTI,
    OP_NOT,
    OP_LDI,
    OP_STI,
    OP_RET,
    OP_RESERVED,
    OP_LEA,
    OP_TRAP
};
enum TRAPS{
    TRAP_GETC = 0x20,
    TRAP_OUT,
    TRAP_PUTS,
    TRAP_IN,
    TRAP_PUTSP,
    TRAP_HALT
};
enum assembler_directive{
    BLKW = 1000,
    FILL,
    STRINGZ
};

// Dyanmic Lists
typedef struct list{
    char ** elements;
    unsigned int size;
}list;
typedef struct short_int_list{
    uint16_t * elements;
    unsigned int size;
}short_int_list;

// Helper Function Prototypes 
int add_list_element(char * element, list * input_list);
void clear_list(list * input_list);

int add_short_int_list(uint16_t num,short_int_list * list_addr);
void clear_short_int_list(short_int_list * list_addr);

int valid_token(char * token);
int is_directive(char * token, char * param);
void trim_spaces(char *string);
int decode_string(char * string, uint16_t *out);

#endif