

#include "headers.h"
#include "opcode.h"
int main(int argc, char *argv[]){
    FILE *intermediate=NULL;
    FILE *fp=NULL;
    FILE *object=NULL;
    int returnvalue=0;
    int returnpass2 = 0;
    SYMTAB mysymboltable= NULL;

    if (argc <2){
        printf("error, there should be two arguments %s <sourcefile>\n", argv[0]);
        return 1;
    }
    const char *Source= argv[1];
    char ObjectFileName[256];
    char intermediate_file[256];

    sprintf(intermediate_file,  "%.*s.int", (int)(strlen(Source) -4), Source);
    sprintf(ObjectFileName, "%s.obj", Source );


    fp= fopen(Source, "r" );
    if (!fp){
       printf("error: cannot open input file %s\n", argv[1]);
       return 1;
    }
    intermediate= fopen(intermediate_file, "w+" );
    if (!intermediate){
        printf("error: cannot open intermediate file %s\n");
        fclose(fp);
        return 1;
        }


    printf("Running Pass1 \n");
    returnvalue = pass1(fp, intermediate, &mysymboltable);
    fclose(fp);

    if (returnvalue == -1){
        printf("error: pass1 failed \n");
        return 1;
    }
   printf("pass1 successfull program Length: %06X\n", returnvalue);



   rewind(intermediate);
   object = fopen(ObjectFileName, "w+");
   if (!object){
        printf("error creating file %s\n");
        return 1;
        }



    printf("Running pass2 \n");

    returnpass2=pass2(object,intermediate,mysymboltable, returnvalue);
    if(returnpass2==-1){
         printf("error: pass2 failed \n");
    }
    else{
    printf("Pass 2 successful. Object code saved to %s\n", ObjectFileName);
    }
    fclose(object);
    fclose(intermediate);
    return returnpass2;
}
