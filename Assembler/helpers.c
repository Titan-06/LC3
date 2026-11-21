#include "helpers.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// All the Opcodes/instructions decalration
char * valid_tokens[VALID_TOKENS] = {"ADD","AND","JMP","JSR","JSRR","LD"
                                    ,"LDI","LDR","LEA","NOT","RET","ST"
                                    ,"STI","STR","GETC","OUT","PUTS","IN","PUTSP","HALT"
                                    ,".BLKW",".FILL",".STRINGZ"
                                    ,"BR","BRn","BRp","BRz","BRzp","BRnz","BRnzp"};

uint16_t token_code[VALID_TOKENS] = {OP_ADD,OP_AND,OP_RET,OP_JSR,OP_JSR,OP_LD
                                    ,OP_LDI,OP_LDR,OP_LEA,OP_NOT,OP_RET,OP_ST
                                    ,OP_STI,OP_STR,TRAP_GETC,TRAP_OUT,TRAP_PUTS,TRAP_IN,TRAP_PUTSP,TRAP_HALT
                                    ,BLKW,FILL,STRINGZ
                                    ,OP_BR,OP_BR,OP_BR,OP_BR,OP_BR,OP_BR,OP_BR};

/* Functions to handle the Dyanmic List */

// To handle the list

int add_list_element(char * element, list * input_list){
    char ** temp = realloc(input_list->elements,(input_list->size + 1) * sizeof(char *));
    if(!temp) return 0;
    input_list->elements = temp;

    char * temp_str = malloc(strlen(element) + 1);
    if(!temp_str) return 0;

    strcpy(temp_str, element);
    input_list->elements[input_list->size] = temp_str;
    input_list->size += 1;

    return 1;
}
void clear_list(list * input_list)
{
    input_list->size = 0;
    for(int i = 0; i < input_list->size; i++){
        free(input_list->elements[i]);
    }
    free(input_list->elements);
    input_list->elements = NULL;
}

// Handle the buffer

int add_buffr_instr(uint16_t instr, struct buffer * buffr_addr)
{
    uint16_t * temp = realloc(buffr_addr->elements,(buffr_addr->size + 1) * sizeof(__uint16_t));
    if(temp == NULL) return 0;

    buffr_addr->elements = temp;
    buffr_addr->elements[buffr_addr->size] = instr;
    buffr_addr->size += 1;
    return 1;

}
void clear_buffer(struct buffer * buffr_addr)
{
    free(buffr_addr->elements);
}

/* Helper Functions */

// To check for Valid Tokens, this returns -1 when not found 
int valid_token(char * token){ 
    for(int i = 0; i < VALID_TOKENS; i++)
    {
        if(strcmp(token,valid_tokens[i]) == 0){
            if(token[0] == '.') return 2;
            return token_code[i];
        } 
    }
    return -1;
}

// Hex to Decimal Helper function (ONLY FOR 4 DIGIT HEX with leading char x)
int hexToDec(char * hex)
{
    // Prepare a hex string
    char hexstr[7];
    hexstr[0] = '0';
    strcpy(&hexstr[1],hex);

    long dec = strtol(hexstr,NULL,16);
    
    return (int)dec;
}

// To check if its a assembler directive and return How much PC should be incremeneted
int is_directive(char * token, char * param)
{
    if(strcmp(token,".FILL")==0){
        return 1;
    }else if(strcmp(token,".BLKW")==0){
        int allot = param[0] - '0';
        return allot;
    }else if(strcmp(token,".STRINGZ")==0){
        return strlen(param) - 3 + 1; // -3 to remove the "" and \n, +1 for null terminator
    }
    return 0;
}
