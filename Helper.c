struct opcodeTable OPTAB[]= {{"ADD", "18"}, {"AND", "40"}, {"COMP", "28"}, {"DIV", "24"},
        {"J", "3C"}, {"JEQ", "30"}, {"JGT", "34"}, {"JLT", "38"},
        {"JSUB", "48"}, {"LDA", "00"}, {"LDCH", "50"}, {"LDL", "08"},
        {"LDX", "04"}, {"MUL", "20"}, {"OR", "44"}, {"RD", "D8"},
        {"RSUB", "4C"}, {"STA", "0C"}, {"STCH", "54"}, {"STL", "14"},
        {"STX", "10"}, {"SUB", "1C"}, {"TD", "E0"}, {"TIX", "2C"},
        {"WD", "DC"}
};

int OptabLength= sizeof(OPTAB)/sizeof(OPTAB[0]);
char *getOpcodeValue(char *opcode) {

        for (int i = 0; i < OptabLength; i++) {
                if (strcmp(opcode, OPTAB[i].mnemonic) == 0) {
                        return OPTAB[i].code;
                }
        }
        return NULL; // not found

}

int isDirective(const char *opcode){
const char *directive[]={"START", "BYTE", "RESB", "END", "RESW", "WORD"};
int length= sizeof(directive)/sizeof(directive[0]);
for (int i=0; i<length; i++){
 if (strcmp(directive[i], opcode)==0) return 1;
}
return 0;
}


int checkForFormat(char *opcode) {


        for (int j=0; j<DirectiveLength; j++) {
                if (strcmp(opcode, directive[j].mnemonic) ==0) {
                        return directive[j].Format;
                }
        }
        return -1;
}

void convertToUpper(char *word) {
        for (int i=0; word[i] != '\0'; i++) {
                word[i]= toupper((unsigned char)word[i]);
        }
}

long Convert(const char *operand,int base, int lineNumber, const char*opcode) {
        char *ptr;
        long convert_base=strtol(operand,&ptr,base);
        if(*ptr != '\0') {
                printf("ASSEMBLY ERROR: Line %d: Invalid  constant '%s'\n", lineNumber, operand);
                exit(-1);
        }
        return convert_base;
}
int checkForOperand(SYMTAB MysymbolTable, char *operand) {

        SYMBOL *current= MysymbolTable;

        while (current !=NULL) {
                if (strcmp (current->name, operand) ==0) {
                        return current->address;
                }
                current = current->next;

        }
        return -1;
}

