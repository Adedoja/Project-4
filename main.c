#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*
    We will call project5loader() from project5loader.c.
    This function will actually perform the relocation and print
    the relocated T and E records.
*/

float project5loader(FILE *object, unsigned int new_start, int machine_type);

/*
    Helper function:
    Converts a string containing HEX digits into an unsigned int.
    Example: "4000" -> 0x4000
    We need this because the relocation address is given in HEX.
*/

static unsigned int parse_hex(const char *s) {
    unsigned int value = 0;

    while (*s) {
        char c = *s++;

       /* stop if whitespace at end */
        if (isspace((unsigned)c)) {
            break;
        }

        /* shift existing value by one hex digit (4 bits) */
        value <<= 4;

        /* convert hex char to number */
       if (c >= '0' && c <= '9') {
            value |= (unsigned int)(c - '0');
        } else if (c >= 'A' && c <= 'F') {
            value |= (unsigned int)(c - 'A' + 10);
        } else if (c >= 'a' && c <= 'f') {
            value |= (unsigned int)(c - 'a' + 10);
        } else {
            /* if we get here, the digit wasn't valid HEX */
            printf("error: invalid hex digit '%c'\n", c);
            exit(1);
       }
    }

    return value;
}

int main(int argc, char *argv[]) {

    FILE *object = NULL;          /* file pointer for opening the object file */
    unsigned int new_start = 0;   /* relocation starting address (hex -> int) */
    int machine_type = -1;        /* 0 = SIC, 1 = SICXE */
    int returnvalue = 0;          /* what we return from project5loader() */

/*
        We expect EXACTLY three arguments after the program name:

            argv[1] = object file name
            argv[2] = relocation address (hex)
            argv[3] = machine type (SIC or SICXE)

        argc must therefore be 4 (program name + 3 arguments).
    */

if (argc != 4) {
        printf("error: there should be three arguments\n");
        printf("usage: %s <objectfile> <new_start_hex> <SIC|SICXE>\n", argv[0]);
        return 1;
    }

 /*
        Save the object file name from the command line.
        We do NOT modify it or add extensions. The user gives the exact name.
    */

    const char *ObjectFileName = argv[1];

/* 
    convert the hex string (argv[2]) into a interger.
    Example input: "4000" -> 0x4000
    */
    new_start = parse_hex(argv[2]);

/*
        Determine which machine type: SIC or SICXE.
        We simply compare the string.
    */
    if (strcmp(argv[3], "SIC") == 0) {
        machine_type = 0;
    } else if (strcmp(argv[3], "SICXE") == 0) {
        machine_type = 1;
    } else {
        printf("error: machine type must be SIC or SICXE\n");
        return 1;
    }

   /*
        Try to open the object file that the user passed in.
        We open in text mode because SIC object files are text.
    */
    object = fopen(ObjectFileName, "r");
    if (!object) {
        printf("error: cannot open object file %s\n", ObjectFileName);
        return 1;
    }

/*
        Call the function that does all the work.
        project5loader.c must provide this function.

        It should:
            - read the object file
            - apply relocation according to machine_type
            - print the relocated T and E records to stdout
    */

returnvalue = project5loader(object, new_start, machine_type);

    /*
        Close the file because we are done with it.
    */
    fclose(object);

    /*
        Return whatever project5loader() returned (usually 0 if successful).
    */
    return returnvalue;
}
