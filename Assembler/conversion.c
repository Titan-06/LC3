#include "conversion.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Helpers
int is_regsiter(char *regis) // -1 if not, othwerise return register(0->7)
{
    if(regis[0] != 'R' || strlen(regis) != 2) return -1;
    if(regis[1] >= '0' && regis[1] <= '7'){
        return regis[1] - '0';
    }
    return -1;
}
int is_num(char *num) // Binary, Dec and Hex allowed, returned as unsigned 16 bit number
{
    char *endptr;
    uint16_t converted;
    if(num[0] == 'X' || num[0] == 'x')
    {
        converted = strtol(&num[1],&endptr,16);
    }else if(num[0] == 'b' || num[0] == 'B')
    {
        converted = strtol(&num[1],&endptr,2);
    }else if(num[0] == '#'){
        converted = strtol(&num[1],&endptr,10);
    }else{
        return -1;
    }
    if(*endptr != '\0') return -1;
    return converted;
}
int trim_bits(uint16_t num, uint16_t bit) // Returns 0 if the number is not under bit limit
{
    return num & ~(0xFFFF << bit);
}

// Some Instruction(OPCODE) follow the same underlying syntax
// So We will check all of those together with one function nd return that instruction
// if there is some error return 0 , since no instruction can be zero

uint16_t add_and(int token_code, list tokens)
{
    if(tokens.size != 4) return 0;

    int r1 = is_regsiter(tokens.elements[1]);
    int r2 = is_regsiter(tokens.elements[2]);
    if( r1 == -1 || r2 == -1) return 0;
    int r3 = is_regsiter(tokens.elements[3]);
    if(r3 == -1){
        r3 = is_num(tokens.elements[3]);
        if(r3 == -1) return 0;
        r3 = trim_bits(r3,5) | (1 << 5);
    }
    uint16_t instr = (token_code << 12) | (r1 << 9) | (r2 << 6) | r3;
    return instr;
}
uint16_t br(int token_code, list tokens)
{
    if(tokens.size != 2) return 0;

    int br_length = strlen(tokens.elements[0]);
    char *br_token = tokens.elements[0];
    uint8_t condition_flag = 0;
    for(int i = 2; i < br_length; i++)
    {
        switch(br_token[i])
        {
            case 'n':
                condition_flag |= 4;
                break;
            case 'z':
                condition_flag |= 2;
                break;
            case 'p':
                condition_flag |= 1;
                break;
        }
    }

    int pcoffset = is_num(tokens.elements[1]);
    if(pcoffset == -1) return 0;
    pcoffset = trim_bits(pcoffset,9);

    uint16_t instr = (condition_flag << 9) | pcoffset;
    return instr;
}

// Main Handle Conversion Function
int handle_conversion(int token_code,list tokens,uint16_t * return_conversion)
{
    uint16_t instr = 0;
    switch(token_code)
    {
        case OP_ADD:
        case OP_AND:
            instr = add_and(token_code,tokens);
            break;
        case OP_BR:
            instr = br(token_code,tokens);
            break;
    }
    if(!instr) return 0;
    *return_conversion = instr;
    return 1;
}