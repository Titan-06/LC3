#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VALID_TOKENS 30 // .ORIG and .END are not counted

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

struct buffer{
    __uint16_t * elements;
    unsigned int size;
};

// Struct for Handling Symbol Table 
typedef struct symbol_entry {
    char *symbol;
    unsigned short PC_LOC;
} symbol_entry;

typedef struct symbol_table {
    symbol_entry *entries;
    unsigned int size;
} symbol_table;

symbol_table SYMBOL_TABLE;

// All the Opcodes/instructions decalrations
char * valid_tokens[VALID_TOKENS] = {"ADD","AND","JMP","JSR","JSRR","LD"
                                    ,"LDI","LDR","LEA","NOT","RET","ST"
                                    ,"STI","STR","GETC","OUT","PUTS","IN","PUTSP","HALT"
                                    ,".BLKW",".FILL",".STRINGZ"
                                    ,"BR","BRn","BRp","BRz","BRzp","BRnz","BRnzp"};

__uint16_t token_code[VALID_TOKENS] = {OP_ADD,OP_AND,OP_RET,OP_JSR,OP_JSR,OP_LD
                                    ,OP_LDI,OP_LDR,OP_LEA,OP_NOT,OP_RET,OP_ST
                                    ,OP_STI,OP_STR,TRAP_GETC,TRAP_OUT,TRAP_PUTS,TRAP_IN,TRAP_PUTSP,TRAP_HALT
                                    ,BLKW,FILL,STRINGZ
                                    ,OP_BR,OP_BR,OP_BR,OP_BR,OP_BR,OP_BR,OP_BR};
// Some Tokens have the same code

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

int add_buffr_instr(__uint16_t instr, struct buffer * buffr_addr)
{
    __uint16_t * temp = realloc(buffr_addr->elements,(buffr_addr->size + 1) * sizeof(__uint16_t));
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

/* Functions to Handle the Symbol Table */

int add_symbol_entry(char * symbol, __uint16_t loc){ // 0 if it fails
    // Alllocate space for a new entry
    symbol_entry * temp = realloc(SYMBOL_TABLE.entries, (SYMBOL_TABLE.size + 1) * sizeof(symbol_entry));
    if(!temp) return 0;
    SYMBOL_TABLE.entries = temp;
    int index = SYMBOL_TABLE.size++;

    //Aloocate space for the string
    char * copy_symbol = malloc((strlen(symbol)+1)* sizeof(char));
    if(!copy_symbol) return 0;

    
    SYMBOL_TABLE.entries[index].symbol = copy_symbol;
    strcpy(SYMBOL_TABLE.entries[index].symbol,symbol);
    SYMBOL_TABLE.entries[index].PC_LOC = loc;
    return 1;
}

void free_symbol_table()
{
    for(int i = 0 ; i < SYMBOL_TABLE.size; i++)
    {
        free(SYMBOL_TABLE.entries[i].symbol);
    }
    free(SYMBOL_TABLE.entries);
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

/* Functions related to the First/Second Pass */

// For First Pass, All The labels are loaded in The Symbol Table with their associated address
// Any error in Syntax is not considered
int first_pass(char * filename)
{
    FILE *fptr = fopen(filename,"r");
    if(fptr == NULL)
    {
        return 0;
    }

    char line[64];
    list label_list = {0}; // Keeps storing all labels until a valid token appears
    unsigned int PC;

    while(fgets(line,64,fptr) != NULL){
        // For an empty line
        if(strcmp(line,"\n") == 0)
        {
            continue;
        }
        //Sanitising comments
        char *cmmnt_ptr = strchr(line,';');
        if(cmmnt_ptr) *cmmnt_ptr = '\0';

        // Go Over Every Token 
        char *token = strtok(line," ");

        // This loops run until a valid token appears, then it stores all the labels that it encountered with the PC (Already incremented)
        // Increments the PC according to the type of token (Directive or instruiction)
        while(token != NULL){
            // When ORIG is encountered set the PC
            if(strcmp(token,".ORIG") == 0)
            {
                char * PC_start = strtok(NULL," "); // Get the next Token
                PC = hexToDec(PC_start);
                break;
            }else {
                int token_no = valid_token(token);
                if(token_no != -1){
                    for(int j = 0; j < label_list.size; j++){
                        add_symbol_entry(label_list.elements[j],PC);
                    }
                    clear_list(&label_list);
                    if(token_no > 999)
                    {
                        char * param = strtok(NULL,"");
                        int increm = is_directive(token,param); // It returns how much to incremenet the PC based on the directive and parameter
                        if(increm == 0) return 0;
                        PC += increm;
                    }else{
                        PC += 1;
                    }
                    break;
                }else{
                    add_list_element(token,&label_list);
                }
            }

            token = strtok(NULL," ");
        }
    }
    fclose(fptr);
    return 1;
}

int second_pass(char * filename)
{
    FILE * file_to_read = fopen(filename,"r");
    
    struct buffer buff;
    char line[64];
    int orig_set = 0;
    unsigned int PC;
    int line_no = 0;

    while(fgets(line,64,file_to_read) != NULL){

        line_no++; // Every Line of the file is counted for the error message(if any)

        // For an empty line
        if(strcmp(line,"\n") == 0)
        {
            continue;
        }
        //Sanitising comments
        char *cmmnt_ptr = strchr(line,';');
        if(cmmnt_ptr) *cmmnt_ptr = '\0';

        // Go Over Every Token 
        char *token = strtok(line," ");
        list instr_tokens;

        // This loops run until a valid token appears, then it stores all the labels that it encountered with the PC (Already incremented)
        // Increments the PC according to the type of token (Directive or instruiction)
        while(token != NULL){
            // When ORIG is encountered set the PC
            if(strcmp(token,".ORIG") == 0)
            {
                if(orig_set){
                    printf("ERROR: Line %d, .ORIG repeated twice\n",line_no);
                    return 0;
                }
                char * PC_start = strtok(NULL," "); // Get the next Token
                PC = hexToDec(PC_start);
                orig_set = 1;
                break;
            }else {
                if(!orig_set)
                {
                    printf("ERROR: Instruction or Label before .ORIG\n");
                    return 0;
                }
                
                int token_no = valid_token(token);
                if(token_no == -1){ // Skip all the Labels
                    token = strtok(NULL," ");
                    continue;
                }
                
                // If a valid token is encountered, take all tokens including and after it
                do{
                    add_list_element(token,&instr_tokens);
                    token = strtok(NULL," ,"); // make sure to get the comma ones
                }while(token != NULL);
                break; // Break out since line is over
            }
        }
    }
    fclose(file_to_read);
    return 1;
}

/* INIT functions */
void init_table() // For the Symbol Table
{
    SYMBOL_TABLE.size = 0;
    SYMBOL_TABLE.entries = NULL;
}

// THE MAIN FUNCTION
int main(int argc, char * argv[])
{
    init_table();
    if(argc != 2){
        printf("Usage: ./main {PROGRAM NAME}\n");
        return 1;
    }

    /*
    if(!first_pass(argv[1])){ // Load the Symbol Table
        printf("First pass didnt work\n");
        return 1;
    }
    */
    second_pass(argv[1]);
}