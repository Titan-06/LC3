#include "helpers.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// All the Opcodes/instructions decalration
char * valid_tokens[VALID_TOKENS] = {"ADD","AND","JMP","JSR","JSRR","LD"
                                    ,"LDI","LDR","LEA","NOT","RET","RTI","ST"
                                    ,"STI","STR","TRAP","GETC","OUT","PUTS","IN","PUTSP","HALT"
                                    ,".BLKW",".FILL",".STRINGZ"
                                    ,"BR","BRn","BRp","BRz","BRzp","BRnz","BRnp","BRnzp"};

uint16_t token_code[VALID_TOKENS] = {OP_ADD,OP_AND,OP_RET,OP_JSR,OP_JSR,OP_LD
                                    ,OP_LDI,OP_LDR,OP_LEA,OP_NOT,OP_RET,OP_RTI,OP_ST
                                    ,OP_STI,OP_STR,OP_TRAP,TRAP_GETC,TRAP_OUT,TRAP_PUTS,TRAP_IN,TRAP_PUTSP,TRAP_HALT
                                    ,BLKW,FILL,STRINGZ
                                    ,OP_BR,OP_BR,OP_BR,OP_BR,OP_BR,OP_BR,OP_BR,OP_BR};

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
    for(int i = 0; i < input_list->size; i++){
        free(input_list->elements[i]);
    }
    free(input_list->elements);
    input_list->size = 0;
    input_list->elements = NULL;
}

// Handle the buffer

int add_short_int_list(uint16_t num,short_int_list * list_addr)
{
    uint16_t * temp = realloc(list_addr->elements,(list_addr->size + 1) * sizeof(__uint16_t));
    if(temp == NULL) return 0;

    list_addr->elements = temp;
    list_addr->elements[list_addr->size] = num;
    list_addr->size += 1;
    return 1;

}
void clear_short_int_list(short_int_list * list_addr)
{
    free(list_addr->elements);
}

/* Helper Functions */

// To check for Valid Tokens, this returns -1 when not found 
int valid_token(char * token){ 
    for(int i = 0; i < VALID_TOKENS; i++)
    {
        if(strcmp(token,valid_tokens[i]) == 0){
            return token_code[i];
        } 
    }
    return -1;
}

// To check if its a assembler directive and return How much PC should be incremeneted
int is_directive(char * token, char * param)
{
    if(strcmp(token,".FILL")==0){
        return 1;
    }else if(strcmp(token,".BLKW")==0){
        int allot = atoi(param);
        return allot;
    }else if(strcmp(token,".STRINGZ")==0){
        return decode_string(param,NULL) - 2 + 1; // -3 to remove the "", +1 for null terminator
    }
    return 0;
}

void trim_spaces(char *string)
{
    if(string == NULL || *string == '\0') return;

    int start = 0;
    int end = 0;

    // Go over until you encounter a character
    for(start = 0; string[start] == ' ' ; start++);
    if(string[start] == '\0') return;

    for(int i = 0; string[i] != '\0'; i++)
    {
        if(string[i] != ' ') end = i;
    }
    string[end+1] = '\0';
    memmove(string,&string[start],end - start + 2);
}

int decode_string(char * string, uint16_t *out)
{
    int count =  0;
    for(int i = 0 ; string[i] != '\0'; i++)
    {
        uint16_t c = string[i];
        if(c == '\\')
        {
            i++;
            switch(string[i])
            {
                case 'n':  c = 0x0A; break;
                case 't':  c = 0x09; break;
                case 'r':  c = 0x0D; break;
                case 'e':  c = 0x1B; break;
                case '0':  c = 0x00; break;
                case '\\': c = '\\'; break;
                case '"':  c = '"';  break;
                default: i--; break;
            }
        }
        if(out) out[count] = c;
        count++;
    }
    return count;
}
