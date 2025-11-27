#include "headers.h"



void Destroy( SYMTAB table ){
         SYMTAB temp;

       while( table != NULL){
        temp = table;
       table = temp->next;
        free(temp);
        }
       printf("The Symbol table destroyed successfully. \n");
}

SYMTAB InsertSymbol(SYMTAB table, char name[7], int addr, int srcline) {
    SYMBOL *new = malloc(sizeof(SYMBOL));
    if (new == NULL) {
        printf("Memory allocation failed for new symbol.\n");
        exit(1);
    }
    memset(new, 0, sizeof(SYMBOL));
    strcpy(new->name, name);
    new->address = addr;
    new->sourceline = srcline;
    new->next = table;
    table = new;
    return table;
}

int SymbolExists(SYMTAB table, char *name) {
    SYMTAB temp = table;
    while (temp != NULL) {
        if (strcmp(temp->name, name) == 0) {
            return 1;
        }
        temp = temp->next;
    }
    return 0;
}

