#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int main(int argc, char *argv[]){
    FILE *intermediate=NULL;
    FILE *fp=NULL;
    FILE *object=NULL;
    int returnvalue=0;

    if (argc <3){
        printf("error, there should be three arguments %s <sourcefile>\n", argv[0]);
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
