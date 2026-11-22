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
    int converted;
    if(num[0] == 'X' || num[0] == 'x')
    {
        converted = strtol(&num[1],&endptr,16);
    }else if(num[0] == 'b' || num[0] == 'B')
    {
        converted = strtol(&num[1],&endptr,2);
    }else if(num[0] == '#'){
        converted = strtol(&num[1],&endptr,10);
    }else{
        converted = strtol(&num[0],&endptr,10); // By Default 
    }
    if(*endptr != '\0') return -1;
    return (uint16_t)converted;
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
        if(r3 == -1 || !is_in_range(r3,5)) return 0;
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
    if(pcoffset == -1 || !is_in_range(pcoffset,9)) return 0;
    pcoffset = trim_bits(pcoffset,9);

    uint16_t instr = (condition_flag << 9) | pcoffset;
    return instr;
}
uint16_t indirect_traps(int token_code, list tokens)
{
    if(tokens.size != 2) return 0;
    int vector = is_num(tokens.elements[1]);
    if(vector == -1 || !is_in_range(vector,9)) return 0;

    return (0xF << 12) | trim_bits(vector,8);
}
uint16_t direct_traps(int token_code, list tokens)
{
    if(tokens.size != 1) return 0;

    uint16_t vector = trim_bits(token_code,8);
    return (0xF << 12) | vector;
}
uint16_t jmp_jsrr(int token_code, list tokens)
{
    if(tokens.size != 2) return 0;

    int base = is_regsiter(tokens.elements[1]);
    if(base == -1) return 0;

    return (token_code << 12) | (base << 6);
}
uint16_t jsr_ret(int token_code, list tokens)
{
    if(token_code == OP_RET) // RET is JMP R7
    {
        if(tokens.size != 1) return 0;
        return (token_code << 12) | (7 << 6);
    }

    // JSR
    if(tokens.size != 2) return 0;
    int pcoffset = is_num(tokens.elements[1]);
    if(pcoffset == -1 || !is_in_range(pcoffset,11)) return 0;

    return (token_code << 12) | (1 << 11) | trim_bits(pcoffset,11);
}
uint16_t pc_offset9(int token_code, list tokens) // LD, LDI, LEA, ST, STI
{
    if(tokens.size != 3) return 0;

    int r1 = is_regsiter(tokens.elements[1]);
    if(r1 == -1) return 0;

    int pcoffset = is_num(tokens.elements[2]);
    if(pcoffset == -1 || !is_in_range(pcoffset,9)) return 0;

    return (token_code << 12) | (r1 << 9) | trim_bits(pcoffset,9);
}
uint16_t ldr_str_not(int token_code, list tokens)
{
    if(token_code == OP_NOT)
    {
        if(tokens.size != 3) return 0;

        int dr = is_regsiter(tokens.elements[1]);
        int sr = is_regsiter(tokens.elements[2]);
        if(dr == -1 || sr == -1) return 0;

        return (token_code << 12) | (dr << 9) | (sr << 6) | 0x3F;
    }

    // LDR, STR
    if(tokens.size != 4) return 0;

    int r1 = is_regsiter(tokens.elements[1]);
    int base = is_regsiter(tokens.elements[2]);
    if(r1 == -1 || base == -1) return 0;

    int offset = is_num(tokens.elements[3]);
    if(offset == -1) return 0;

    return (token_code << 12) | (r1 << 9) | (base << 6) | trim_bits(offset,6);
}
uint16_t rti(int token_code, list tokens)
{
    if(tokens.size != 1) return 0;
    return token_code << 12;
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
        case OP_TRAP:
            instr = indirect_traps(token_code,tokens);
            break;
        case TRAP_GETC:
        case TRAP_HALT:
        case TRAP_IN:
        case TRAP_OUT:
        case TRAP_PUTS:
        case TRAP_PUTSP:
            instr = direct_traps(token_code,tokens);
            break;
                case OP_RET:
        case OP_JSR:
            if(strcmp(tokens.elements[0],"JMP") == 0 || strcmp(tokens.elements[0],"JSRR") == 0)
                instr = jmp_jsrr(token_code,tokens);
            else
                instr = jsr_ret(token_code,tokens);
            break;
        case OP_LD:
        case OP_LDI:
        case OP_LEA:
        case OP_ST:
        case OP_STI:
            instr = pc_offset9(token_code,tokens);
            break;
        case OP_LDR:
        case OP_STR:
        case OP_NOT:
            instr = ldr_str_not(token_code,tokens);
            break;
        case OP_RTI:
            instr = rti(token_code,tokens);
            break;
    }
    if(!instr) return 0;
    *return_conversion = instr;
    return 1;
}

int handle_directive(int token_code,list tokens,short_int_list * to_fill){
    if(tokens.size != 2) return 0;
    switch (token_code)
    {
    case STRINGZ:
        trim_spaces(tokens.elements[1]);
        int i = decode_string(tokens.elements[1],NULL); // Doesnt count null Terminator
        i++; // Account for null terminator;

        uint16_t * decoded_string = malloc(i * sizeof(uint16_t));
        decode_string(tokens.elements[1],decoded_string);
        decoded_string[i-1] = '\0';

        if(i < 3){
            free(decoded_string);
            return 0;
        }
        if(decoded_string[0] == '"' && decoded_string[i-2] == '"')
        {
            for(int j = 1;j < (i-2); j++)
            {
                add_short_int_list(decoded_string[j],to_fill);
            }
            add_short_int_list(0,to_fill);
            free(decoded_string);
            return i - 2; // Also include the null term so +1
        }else{
            free(decoded_string);
            return 0;
        }
    case BLKW:
        int empty_addr = is_num(tokens.elements[1]);
        if(empty_addr == -1) return 0;

        for(int j = 0 ; j < empty_addr; j++) add_short_int_list(0,to_fill);

        return empty_addr;
    case FILL:
        int num = is_num(tokens.elements[1]);
        if(num == -1) return 0;
        add_short_int_list(num,to_fill);
        return 1;
    default:
        return 0;
    }
}