#include "helpers.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

/* Functions to Handle the Symbol Table */

int add_symbol_entry(char * symbol, uint16_t loc){ // 0 if it fails
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
    
    short_int_list buffer;
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