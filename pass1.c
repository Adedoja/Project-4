#include "headers.h"
int pass1(FILE *fp, FILE *file,SYMTAB *MySymbolTable ) {

        int lineNumber =0, Location_Counter = 0, address=0;
        char line[1024], label[20], opcode[20], operand[120];
        long convert=0;
        int i=0,counter=0,token=0;
        int errorcheck=0;

        while(fgets(line, sizeof(line), fp) != NULL) {
                lineNumber++;
                if(line[0] =='.') {
                        fprintf(file, "%s", line);
                        continue;
                }

                token = Tokenizer(line, label, opcode, operand);
                if (token==-1) return -1;
                if (token ==0) continue;
                convertToUpper(opcode);

               if (label[0] != '\0' && isDirective(label) == 1) {
               printf("ASSEMBLY ERROR:\n%s\nLine %d: Invalid label. This is a directive name '%s'\n",line, lineNumber, label);
               return -1;
               }



                        if (strcmp(opcode, "START") == 0) {

                        if (operand[0] == '\0') {
                                printf("ASSEMBLY ERROR:\n%s\nLine %d: START missing address\n", line, lineNumber);
                                errorcheck = 1;
                break;

                        }
                        convert= Convert(operand, 16, lineNumber, "START");
                        if(convert >= 32768 ) {
                                printf("ASSEMBLY ERROR:\n%s\nLine %d:No room left in SIC memory. '%s'\n", line, lineNumber, label);
                        errorcheck = 1;
                break;
                        }
                        Location_Counter = convert;
                        address = convert;
                        fprintf(file, "%04X\t%s\t%s\t%s\n", Location_Counter, label, opcode, operand);
                        continue;
                }

                if (label[0] != '\0' && isalpha(label[0])) {
                        int j;
                        if(strlen(label)>6) {
                                printf("ASSEMBLY ERROR:\n%s\nLine %d:Invalid stringn length '%s'\n", line, lineNumber, label);
                                errorcheck = 1;
                break;
                        }
                        for(j=0; label[j]; j++) {
                                if(!isalnum(label[j])) {
                                        printf("ASSEMBLY ERROR:\n%s\nLine %d:Invalid string '%s'\n", line, lineNumber, label);
                                        errorcheck = 1;
                    break;
                                }

                        }


                        if (SymbolExists(*MySymbolTable, label)) {
                                printf("ASSEMBLY ERROR:\n%s\nLine %d: Duplicate symbol '%s'\n", line, lineNumber, label);
                                errorcheck = 1;
                break;
                        }
                        *MySymbolTable = InsertSymbol(*MySymbolTable, label, Location_Counter, lineNumber);


                }
                fprintf(file, "%04X\t%s\t%s\t%s\n", Location_Counter, label, opcode, operand);

                if (strcmp(opcode, "WORD") == 0) {
                       trimTrailing(operand);
                        long opcode_convert= Convert(operand, 10, lineNumber, "WORD");
                        if(opcode_convert < -8388608 || opcode_convert > 16777215 ) {
                                printf("ASSEMBLY ERROR:\n%s\nLine %d: WORD Constant Exceeds 24 bit limitation '%s'\n", line, lineNumber, label);
                                errorcheck = 1;
                break;
                        }

                        Location_Counter += 3;
                }
                else if (strcmp(opcode, "RESW") == 0) {

                        long value= Convert(operand, 10, lineNumber, "RESW");
                        Location_Counter += 3 * value;
                }
                else if (strcmp(opcode, "RESB") == 0) {
                        long value= Convert(operand, 10, lineNumber, "RESB");
                        Location_Counter += value;
                }


                else if (strcmp(opcode, "BYTE") == 0) {
                        if (operand[0] != 'C' && operand[0] != 'X') {
                                printf("ASSEMBLY ERROR:\n%s\nLine %d: Invalid BYTE constant\n", line, lineNumber);
                                errorcheck = 1;
                break;
                        }


                        if (operand[1] != '\'') {
                                printf("ASSEMBLY ERROR:\n%s\nLine %d: Invalid BYTE constant missing quotes\n", line, lineNumber);
                                errorcheck = 1;
                break;
                        }

                         i = 2;
                         counter = 0;
                        if (operand[0]=='C') {
                                while (operand[i] != '\'' && operand[i] != '\0') {
                                        i++;
                                        counter++;
                                }
                        }
                        else {
                                while (operand[i] != '\'' && operand[i] != '\0') {

                                        if ((operand[i] >= '0' && operand[i] <= '9') ||
                                                (operand[i] >= 'a' && operand[i] <= 'f') ||
                                                (operand[i] >= 'A' && operand[i] <= 'F')) {
                                                i++;
                                                counter++;
                                        } else {
                                                printf("ASSEMBLY ERROR:\n%s\nLine %d: Invalid hex digit found\n", line, lineNumber);
                                                errorcheck = 1;
                        break;
                                        }
                                }



                                if ((counter % 2) != 0 && operand[0] == 'X') {
                                        printf("ASSEMBLY ERROR:\n%s\nLine %d: Uneven number of hex digits in BYTE constant\n", line, lineNumber);
                                        errorcheck = 1;
                    break;
                                }
                        }

                        int total_bytes = counter / 2;

                        if (operand[0] == 'C') {
                                Location_Counter += counter;
                        } else if (operand[0] == 'X') {
                                Location_Counter += total_bytes;
                        } else {
                                printf("ASSEMBLY ERROR:\n%s\nLine %d: Unrecognized BYTE format\n", line, lineNumber);
                                errorcheck = 1;
                break;
                        }
                }

                else if (strcmp(opcode, "END") == 0) {
                     
                        break;
                }
                else {
                        int format = checkForFormat(opcode);
                        if (format == -1) {
                                printf("ASSEMBLY ERROR:\n%s\nLine %d: Invalid opcode '%s'\n", line, lineNumber, opcode);
                                errorcheck = 1;
                break;
                        }
                        Location_Counter += format;
                }
                
        }

        int Length = Location_Counter - address;
        if(errorcheck) return -1;
        return Length;
}

