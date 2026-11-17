#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct symbol_entry {
    char *symbol;
    unsigned short PC_LOC;
} symbol_entry;

typedef struct symbol_table {
    symbol_entry *entries;
    int size;
} symbol_table;

symbol_table SYMBOL_TABLE;

// INIT function for the symbol table
void init_table()
{
    SYMBOL_TABLE.size = 0;
    SYMBOL_TABLE.entries = NULL;
}
// Helper Functions for the table
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
int main()
{
    add_symbol_entry("Hello",24);
    printf("Size Now: %d\n",SYMBOL_TABLE.size);
    add_symbol_entry("Bye",25);
    printf("Size Now: %d\n",SYMBOL_TABLE.size);

    for(int i = 0; i < SYMBOL_TABLE.size; i++)
    {
        printf("%s: %d\n",SYMBOL_TABLE.entries[i].symbol,SYMBOL_TABLE.entries[i].PC_LOC);
    }
    free_symbol_table();
}