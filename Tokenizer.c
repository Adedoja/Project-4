
#include "headers.h"
#include "opcode.h"

extern struct opcode directive[];
extern int DirectiveLength;

int isOpcode(char *word) {
    if (!word) return 0;

    for (int i = 0; i < DirectiveLength; i++) {
        if (strcasecmp(word, directive[i].mnemonic) == 0) {  // case-insensitive
            return 1;
        }
    }
    return 0;
}

void trimTrailing(char *str) {
    int len = strlen(str);
    while (len > 0 && isspace((unsigned char)str[len - 1])) str[--len] = '\0';
}


int Tokenizer(char *line, char *label, char *opcode, char *operand) {
    if (!line || !label || !opcode || !operand) 
        return 0;

    label[0] = opcode[0] = operand[0] = '\0';
    while (*line == ' ' || *line == '\t') line++;
    if (*line == '\0' || *line == '.' || *line == '#')
        return 0;
   
     char buff[1024]; 
    strncpy(buff, line, sizeof(buff) - 1);
    buff[sizeof(buff) - 1] = '\0';

    
    char *cmt = strchr(buff + 1, '.');
    if (cmt) *cmt = '\0';
    cmt = strchr(buff + 1, '#');
    if (cmt) *cmt = '\0';
    char *ptr = buff;
    char *t1 = ptr;
    while (*ptr && !isspace((unsigned char)*ptr)) ptr++;
    if (*ptr) { *ptr = '\0'; ptr++; }  
    if (!t1 || *t1 == '\0') return 0;
    if (isOpcode(t1)) {
        strcpy(opcode, t1);
    } else {
        strcpy(label, t1);
        while (*ptr && isspace((unsigned char)*ptr)) ptr++;
        if (*ptr) {
            char *t2 = ptr;
            while (*ptr && !isspace((unsigned char)*ptr)) ptr++;
            if (*ptr) { *ptr = '\0'; ptr++; }
            strcpy(opcode, t2);
        }
    }

    while (*ptr && isspace((unsigned char)*ptr)) ptr++;
    if (*ptr) {
        strcpy(operand, ptr);
        trimTrailing(operand); 
    }

    convertToUpper(label);
    convertToUpper(opcode);
    convertToUpper(operand);

    return 1;
}
int stringwithacomma(char *operand){
    int length= strlen(operand);
    if (length<2) return 0;
    if(operand[length-2]==',' && operand[length-1]=='X') return 1;
    return 0;
}
