#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*
    Forward declaration of the loader function that lives in project5loader.c

    - object:      already opened FILE* for the object file
    - new_start:   relocation start address (from command line, already parsed)
    - machine_type: 0 = SIC, 1 = SICXE

    The function will do all the relocation work and print T and E records.
*/
int project5loader(FILE *object, unsigned int new_start, int machine_type);

/*
    This helper function takes a string that contains hex digits,
    like "003000", and converts it into an unsigned int (0x003000).

    I use this to convert the relocation address from argv[2].
*/
static unsigned int parse_hex(const char *s) {
    unsigned int value = 0;

    while (*s) {
        char c = *s++;

        /* If I hit whitespace, I consider the hex field done. */
        if (isspace((unsigned char)c)) {
            break;
        }

        /* Make room for one hex digit (4 bits) by shifting left. */
        value <<= 4;

       if (c >= '0' && c <= '9') {
            value |= (unsigned int)(c - '0');
        } else if (c >= 'A' && c <= 'F') {
            value |= (unsigned int)(c - 'A' + 10);
        } else if (c >= 'a' && c <= 'f') {
            value |= (unsigned int)(c - 'a' + 10);
        } else {
            /* If it's not a 0?^`^s9, A?^`^sF, or a?^`^sf, it's not valid hex. >
            printf("error: invalid hex digit '%c' in relocation address\n", c);
            exit(1);
        }
    }
    return value;
}

int main(int argc, char *argv[]) {

    FILE *object = NULL;          /* will point to the object file */
    unsigned int new_start = 0;   /* relocation start address (parsed from hex)>
    int machine_type = -1;        /* 0 = SIC, 1 = SICXE */
    int returnvalue = 0;          /* value returned by project5loader() */

    /*
        I expect the user to run the program like this:

            ./project5loader <objectfile> <new_start_hex> <SIC|SICXE>

        So there should be exactly 3 arguments after the program name.
        That means argc must be 4 total.
    */
    if (argc != 4) {
        printf("error: there should be three arguments\n");
        printf("usage: %s <objectfile> <new_start_hex> <SIC|SICXE>\n", argv[0]);
        return 1;
    }

    /*
        argv[1] is the name of the object file produced by the assembler.
        I just store it in a const char* and use it for fopen.
    */
    const char *ObjectFileName = argv[1];

    /*
argv[2] is the relocation start address in hex (string).
        I convert it to an unsigned int using my helper.
        Example: "003000" becomes 0x003000.
    */
    new_start = parse_hex(argv[2]);

    /*
        argv[3] tells me which machine we are loading for: SIC or SICXE.
        I store this as a simple integer flag.
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
        Now I try to open the object file the user gave me.
        The loader expects the file to contain H, T, (M for XE), and E records.
    */
    object = fopen(ObjectFileName, "r");
    if (!object) {
        printf("error: cannot open object file %s\n", ObjectFileName);
        return 1;
    }

    /*
        Here I hand off control to the actual loader function defined in
        project5loader.c. It will read the object file, relocate it, and
        print the relocated T and E records to stdout.
    */
    returnvalue = project5loader(object, new_start, machine_type);

    /*
        Done with the file, so I close it.
    */
    fclose(object);

    /*
        Return whatever the loader returned (0 for success, non-zero for error).
    */
    return returnvalue;
}

