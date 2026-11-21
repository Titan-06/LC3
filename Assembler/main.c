#include "helpers.h"
#include "conversion.h"

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
int is_symbol(char * symbol) // Returns the index for the Symbol table
{
    for(int i = 0 ; i < SYMBOL_TABLE.size; i++)
    {
        if(strcmp(SYMBOL_TABLE.entries[i].symbol,symbol)==0)
        {
            return i;
        }
    }
    return -1;
}
void free_symbol_table()
{
    for(int i = 0 ; i < SYMBOL_TABLE.size; i++)
    {
        free(SYMBOL_TABLE.entries[i].symbol);
    }
    free(SYMBOL_TABLE.entries);
}

// Writing the binary to the file 
void add_instr_buffer(uint16_t instr, short_int_list * buffer)
{
    instr = (instr << 8) | (instr >> 8);
    add_short_int_list(instr,buffer);
}
void write_binary(char *filename,short_int_list buffer)
{
    char *name = strtok(filename,".");
    if(strlen(name) > 46){
        printf("File Name too big. (Make sure its Under 46 chars)\n");
        return;
    }
    char objfile[50];
    snprintf(objfile,sizeof(objfile),"%s%s",name,".obj");
    FILE * objptr = fopen(objfile,"wb");
    if(!objptr){
        printf("Something went wrong. Couldn't open file.\n");
        return;
    }
    fwrite(buffer.elements,sizeof(uint16_t),buffer.size,objptr);
    printf("Obj File Created: %s\n",objfile);
    fclose(objptr);

}

// Stripping comments
void strip_comment(char *line)
{
    int quote_flag = -1; // -1 is off and 1 is on 
    while(*line != '\0')
    {
        if(*line == ';' && quote_flag == -1){ // If not inside quotes and ; is encountered
            *line = '\0';
            return;
        }
        
        if(*line == '"' && *(line-1) != '\\') quote_flag *= -1;

        line++;
    }
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

    char line[256];
    list label_list = {0}; // Keeps storing all labels until a valid token appears
    unsigned int PC;

    while(fgets(line,256,fptr) != NULL){
        // For an empty line
        if(strcmp(line,"\n") == 0)
        {
            continue;
        }
        //Sanitising comments
        strip_comment(line);

        // Go Over Every Token 
        char *token = strtok(line," \n");

        // This loops run until a valid token appears, then it stores all the labels that it encountered with the PC (Already incremented)
        // Increments the PC according to the type of token (Directive or instruiction)
        while(token != NULL){
            // When ORIG is encountered set the PC
            if(strcmp(token,".ORIG") == 0)
            {
                char * PC_start = strtok(NULL," \n"); // Get the next Token
                PC = is_num(PC_start);
                if(PC == -1) return 0;
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
                        char * param = strtok(NULL,"\n");
                        trim_spaces(param);
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
            token = strtok(NULL," \n");
        }
    }
    fclose(fptr);
    return 1;
}

int second_pass(char * filename)
{
    FILE * file_to_read = fopen(filename,"r");
    
    short_int_list buffer = {0};
    list instr_tokens = {0};
    char line[256];
    int orig_set = 0; // For .ORIG
    int ended = 0; // For .END
    unsigned int PC;
    int line_no = 0;

    while(fgets(line,256,file_to_read) != NULL){
        if(ended) break; // If .END is read
        line_no++; // Every Line of the file is counted for the error message(if any)

        // For an empty line
        if(strcmp(line,"\n") == 0)
        {
            continue;
        }
        //Sanitising comments
        strip_comment(line);
        
        // Go Over Every Token 
        char *token = strtok(line," \n");
        
        
        // This loops run until a valid token appears, then it stores all the labels that it encountered with the PC (Already incremented)
        // Increments the PC according to the type of token (Directive or instruiction)
        while(token != NULL){
            if(strcmp(token,".END") == 0){
                ended = 1;
                break;
            }
            // When ORIG is encountered set the PC
            if(strcmp(token,".ORIG") == 0)
            {
                if(orig_set){
                    printf("ERROR: Line %d, .ORIG repeated twice\n",line_no);
                    return 0;
                }
                char * PC_start = strtok(NULL," \n"); // Get the next Token
                PC = is_num(PC_start);
                if(PC == -1) return 0;
                add_instr_buffer(PC,&buffer);
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
                    token = strtok(NULL," \n");
                    continue;
                }

                // Pc incremented accordingly
                if(token_no < 999)
                {
                    PC += 1;
                }

                uint16_t instr;
                // If a valid token is encountered, take all tokens including and after it
                do{
                    
                    int symbol = is_symbol(token);
                    
                    if(symbol != -1)
                    {
                        char pc_loc[10]; // Since max value for a 16 bit is 65,535
                        int relative_value = SYMBOL_TABLE.entries[symbol].PC_LOC - PC;
                        if(token_no > 999) relative_value += PC ;
                        snprintf(pc_loc,sizeof(pc_loc),"#%d",relative_value); // Label is ccounted realtive to incremneted PC
                        add_list_element(pc_loc,&instr_tokens);
                    }else{
                        add_list_element(token,&instr_tokens);
                    }

                    if(token_no == 1002){
                        token = strtok(NULL,"\n"); // Just get the one after the directive
                    }else{
                        token = strtok(NULL," ,\n"); // make sure to get the comma ones
                    }
                    
                }while(token != NULL);
                // Handling the final conversion
                if(token_no > 999)
                {
                    short_int_list to_fill ={0};
                    int increment = handle_directive(token_no,instr_tokens,&to_fill);
                    if(!increment){
                        clear_list(&instr_tokens);
                        printf("ERROR: Directive is not correct at line %d\n",line_no);
                        return 0;
                    }
                    PC += increment;
                    for(int j = 0; j < to_fill.size; j++)
                    {
                        add_instr_buffer(to_fill.elements[j],&buffer);
                    }
                }else{
                    if(!handle_conversion(token_no,instr_tokens,&instr))
                    {
                        clear_list(&instr_tokens);
                        printf("ERROR: Invalid Syntax at line %d\n",line_no);
                        return 0;
                    }
                    add_instr_buffer(instr,&buffer);
                }
                
                clear_list(&instr_tokens);
                break; // Break out since line is over
            }
        }
    }

    // This is if .END is not written at the end
    if(!ended){
        printf("ERROR: NO .END ENCOUNTERED.\n");
        clear_short_int_list(&buffer);
        fclose(file_to_read);
        return 0;
    }else{
        write_binary(filename,buffer);
        clear_short_int_list(&buffer);
        fclose(file_to_read);
        return 1;
    }
    
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
    
    second_pass(argv[1]);
}