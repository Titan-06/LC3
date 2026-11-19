#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VALID_TOKENS 31 // .ORIG and .END are not counted

// A Dyanmic List
typedef struct list{
    char ** elements;
    unsigned int size;
}list;

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

// PC counter for the entire file
unsigned int PC;

// All the Opcodes/instructions decalrations
char * valid_tokens[VALID_TOKENS] = {"ADD","AND","JMP","JSR","JSRR","LD"
                                    ,"LDI","LDI","LDR","LEA","NOT","RET","ST"
                                    ,"STI","STR","GETC","OUT","PUTS","IN","PUTSP","HALT"
                                    ,".BLKW",".FILL",".STRINGZ"
                                    ,"BR","BRn","BRp","BRz","BRzp","BRnz","BRnzp"};


/* Functions to handle the Dyanmic List */
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

// To check for Valid Tokens 
int valid_token(char * token){ 
    for(int i = 0; i < VALID_TOKENS; i++)
    {
        if(strcmp(token,valid_tokens[i]) == 0){
            if(token[0] == '.') return 2;
            return 1;
        } 
    }
    return 0;
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
                if(token_no){
                    for(int j = 0; j < label_list.size; j++){
                        add_symbol_entry(label_list.elements[j],PC);
                    }
                    clear_list(&label_list);
                    if(token_no == 2)
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

    if(!first_pass(argv[1])){ // Load the Symbol Table
        printf("First pass didnt work\n");
        return 1;
    }
    // To check
    for(int i = 0; i < SYMBOL_TABLE.size; i++){
        printf("%s: %#x\n", SYMBOL_TABLE.entries[i].symbol,SYMBOL_TABLE.entries[i].PC_LOC);
    }
}