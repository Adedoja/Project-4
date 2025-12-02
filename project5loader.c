#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*
    This function takes a string of hex characters like "001000"
    and turns it into an unsigned int (0x001000).

    I use this whenever I read an address or length from the
    object file (H, T, M, E records).
*/
static unsigned int parse_hex(const char *s) {
    unsigned int value = 0;

    while (*s) {
        char c = *s++;

        /* If I hit whitespace, I'm done with this hex field. */
        if (isspace((unsigned char)c)) {
            break;
        }

        /* Shift the current value 4 bits left to make room for the new hex dig>
        value <<= 4;

        /* Now I convert the character into its numeric hex value. */
        if (c >= '0' && c <= '9') {
            value |= (unsigned int)(c - '0');
        } else if (c >= 'A' && c <= 'F') {
            value |= (unsigned int)(c - 'A' + 10);
        } else if (c >= 'a' && c <= 'f') {
            value |= (unsigned int)(c - 'a' + 10);
        } else {
            /* If I get here, it's not a valid hex character. */
            printf("error: invalid hex digit '%c'\n", c);
            exit(1);
        }
    }

    return value;
}

/*
    Here I parse an H record in fixed format.

    Example H line (no spaces):
        HCOPY  00100000107A

    Layout:
        [0]     = 'H'
        [1-6]   = program name (6 chars)
        [7-12]  = start address (6 hex)
        [13-18] = program length (6 hex)

    I grab:
        progname   = name of the program
        start_str  = start address as string
        len_str    = program length as string
*/
static void parse_H_record(char *line,
                           char progname[7],
                           char start_str[7],
                           char len_str[7]) {

    /* Grab the program name (6 characters starting at index 1). */
    strncpy(progname, line + 1, 6);
    progname[6] = '\0';

    /* Grab the starting address (6 characters starting at index 7). */
    strncpy(start_str, line + 7, 6);
    start_str[6] = '\0';

    /* Grab the program length (6 characters starting at index 13). */
    strncpy(len_str, line + 13, 6);
    len_str[6] = '\0';
}
/*
    Here I parse a T record in fixed format.

    Example T line (no spaces):
        T0010301EF800141033281030...

    Layout:
        [0]    = 'T'
        [1-6]  = start address (6 hex)
        [7-8]  = length in bytes (2 hex)
        [9-..] = object code as hex characters (2 hex chars per byte)

    I split this into:
        addr_str = starting address as a string
        len_str  = length as a string
        obj_str  = entire object hex string
*/
static void parse_T_record(char *line,
                           char addr_str[7],
                           char len_str[3],
                           char obj_str[256]) {

    /* Copy the 6-character start address from positions 1's6. */
    strncpy(addr_str, line + 1, 6);
    addr_str[6] = '\0';

    /* Copy the 2-character length from positions 7's8. */
    strncpy(len_str, line + 7, 2);
    len_str[2] = '\0';

    /* Everything from index 9 onward is the object code (as hex). */
    strcpy(obj_str, line + 9);

    /* If there's a newline or carriage return at the end, remove it. */
    size_t n = strlen(obj_str);
    if (n > 0 && (obj_str[n - 1] == '\n' || obj_str[n - 1] == '\r')) {
        obj_str[n - 1] = '\0';
    }
}

/*
    Here I parse an E record.

    Example:
        "E001000" or just "E"

    Layout:
        [0]    = 'E'
        [1-6]  = optional execution address (6 hex chars)

    If there's no address, I set exec_str to an empty string.
*/
static void parse_E_record(char *line, char exec_str[7]) {
    size_t len = strlen(line);

    /* If the last char is a newline or carriage return, trim it off. */
    if (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
        line[len - 1] = '\0';
        len--;
    }

    /* If the line is basically just 'E', then there's no address. */
    if (len <= 1) {
        exec_str[0] = '\0';
        return;
    }

    /* Otherwise, copy the 6-character execution address starting at index 1. */
    strncpy(exec_str, line + 1, 6);
    exec_str[6] = '\0';
}

/*
    Here I parse an M record for SIC/XE.

    Example XE M record (no spaces):
        M00100705+

    Layout:
        [0]    = 'M'
        [1-6]  = starting address of the field
        [7-8]  = length in half-bytes (nibbles), as hex
        [9]    = sign ('+' or '-')

    I pull out:
        addr_str  = where in memory to start modifying
        len_str   = how many half-bytes are in the field
        sign_char = whether to add or subtract reloc
*/

static void parse_M_record(char *line,
                           char addr_str[7],
                           char len_str[3],
                           char *sign_char) {

    /* Address of field (6 chars starting at index 1). */
    strncpy(addr_str, line + 1, 6);
    addr_str[6] = '\0';

    /* Length of the field in half-bytes (2 chars starting at index 7). */
    strncpy(len_str, line + 7, 2);
    len_str[2] = '\0';

    /* Sign is a single character at index 9. */
    *sign_char = line[9];
}

/*
    This function handles relocation for plain SIC (machine_type == 0).

    The idea here:
      - I read the H record to find the original start address.
      - I compute reloc = new_start - orig_start.
      - For every T record, I bump its starting address by reloc.
      - For the E record, if it has an address, I bump that by reloc too.

    In this version I am NOT touching individual words inside the T
    record's object bytes, because without extra info (like bitmasks)
    I don't know which bytes are relocatable and which are just data.

    The assignment says the output should just be the T and E records
    with relocation done, so at least I relocate their addresses.
*/

static void relocate_sic(FILE *object, unsigned int new_start) {
    char line[256];
    unsigned int orig_start = 0;
    unsigned int reloc = 0;
    int header_found = 0;

    /* First, I scan the file to find the H record and compute reloc. */
    while (fgets(line, sizeof(line), object) != NULL) {
        if (line[0] == 'H') {
            char progname[7];
            char start_str[7];
            char len_str[7];

            /* Break H into pieces: name, start address, length. */
            parse_H_record(line, progname, start_str, len_str);

            /* Convert the starting address string to an int. */
            orig_start = parse_hex(start_str);

            /* My relocation constant is new_start - original_start. */
            reloc = new_start - orig_start;
            header_found = 1;
            break;
        }
    }

    if (!header_found) {
        printf("error: no H record found in object file\n");
        return;
    }

    /* Now I go back to the beginning to process ALL the records. */
    rewind(object);

    /* Second pass: actually relocate T and E records. */
    while (fgets(line, sizeof(line), object) != NULL) {

        if (line[0] == 'H') {
            /* I skip H because the assignment only wants T and E output. */
            continue;
        } else if (line[0] == 'T') {
            char addr_str[7];
            char len_str[3];
            char obj_str[256];

            /* Break the T line into address, length, and object bytes. */
            parse_T_record(line, addr_str, len_str, obj_str);

            /* Convert address and length from hex strings to numbers. */
            unsigned int start_addr = parse_hex(addr_str);
            unsigned int length_bytes = parse_hex(len_str);

            /* Add reloc to the starting address of this T record. */
            unsigned int relocated_start = start_addr + reloc;

            /*
                Now I reprint the T record using the relocated starting
                address, the same length, and the same object bytes.

                Format:
                    T<6-hex-new-start><2-hex-length><object-bytes-as-hex>
            */
            printf("T%06X%02X%s\n", relocated_start, length_bytes, obj_str);

        } else if (line[0] == 'E') {
            char exec_str[7];
            unsigned int exec;

            /* Pull out any execution address from the E record. */
            parse_E_record(line, exec_str);

            /* If there's no address, I just print "E". */
            if (exec_str[0] == '\0') {
                printf("E\n");
            } else {
                /* Otherwise, relocate that execution address too. */
                exec = parse_hex(exec_str);
                exec += reloc;
                printf("E%06X\n", exec);
            }
        } else {
            /* Any other record type (if present), I simply ignore. */
            continue;
        }
    }
}

/*
    This function handles relocation for SIC/XE (machine_type == 1).

    For XE, I can't just bump T and E addresses, because there are
    M records that say exactly which fields inside the object bytes
    are relocatable.

    So I do this in multiple passes:

      1. Read all lines and find the H record. From H I get:
            - orig_start   (starting address)
            - program_length
         and then I can compute:
            - reloc = new_start - orig_start

      2. Allocate a "memory image" big enough for the program length.

      3. Go through all T records and load their bytes into the memory
         buffer at the correct offsets.

      4. Go through all M records and, for each one:
            - figure out which bytes the field covers
            - read that field from memory
            - add (or subtract) the reloc value
            - write it back into memory

      5. Finally, I print new T and E records:
            - T records are rebuilt from memory, but start address is
              relocated.
            - E record's address is also relocated if it has one.
*/
static void relocate_sicxe(FILE *object, unsigned int new_start) {
    char line[256];

    /* I keep all the lines from the object file in this array. */
    char *lines[1024];
    int line_count = 0;

    unsigned int orig_start = 0;
    unsigned int program_length = 0;
    unsigned int reloc = 0;
    int header_found = 0;

    /* First, I read every line of the object file into memory. */
    while (fgets(line, sizeof(line), object) != NULL) {
        lines[line_count] = (char *)malloc(strlen(line) + 1);
        if (lines[line_count] == NULL) {
            printf("error: out of memory\n");
            exit(1);
        }
        strcpy(lines[line_count], line);
        /* While I'm reading, I look for the H record once. */
        if (lines[line_count][0] == 'H' && !header_found) {
            char progname[7];
            char start_str[7];
            char len_str[7];

            parse_H_record(lines[line_count], progname, start_str, len_str);

            /* Get original start address and program length. */
            orig_start = parse_hex(start_str);
            program_length = parse_hex(len_str);

            /* Calculate relocation constant for XE. */
            reloc = new_start - orig_start;
            header_found = 1;
        }

        line_count++;
    }

    if (!header_found) {
        printf("error: no H record found in XE object file\n");
        for (int i = 0; i < line_count; i++) {
            free(lines[i]);
        }
        return;
    }

    /* Now I allocate a memory buffer that represents the whole program. */
    unsigned char *memory = (unsigned char *)calloc(program_length, 1);
    if (memory == NULL) {
        printf("error: cannot allocate memory image\n");
        for (int i = 0; i < line_count; i++) {
            free(lines[i]);
        }
        exit(1);
   }

    /*
        PASS 1 (XE):
        Here I load all the T records into the memory buffer.

        For each T line:
            - I parse the start address and length
            - I convert the hex object string to real bytes
            - I place those bytes into the correct offset in memory
    */
    for (int i = 0; i < line_count; i++) {
        if (lines[i][0] == 'T') {
            char addr_str[7];
            char len_str[3];
            char obj_str[256];

            parse_T_record(lines[i], addr_str, len_str, obj_str);

            unsigned int start_addr = parse_hex(addr_str);
            unsigned int length_bytes = parse_hex(len_str);

            /* Find where this T block goes in my memory buffer. */
            unsigned int offset = start_addr - orig_start;
            if (offset + length_bytes > program_length) {
                printf("error: T record exceeds program length\n");
                free(memory);
                for (int j = 0; j < line_count; j++) {
                    free(lines[j]);
                }
                exit(1);
            }

            /*
                Now I walk through obj_str two characters at a time,
                turn each pair (like "F8") into a byte, and store it in memory.
            */
            unsigned int mem_index = offset;
            unsigned int obj_len = (unsigned int)strlen(obj_str);
            unsigned int k = 0;

            while (k + 1 < obj_len && mem_index < offset + length_bytes) {
                char byte_hex[3];
                byte_hex[0] = obj_str[k];
                byte_hex[1] = obj_str[k + 1];
                byte_hex[2] = '\0';

                unsigned int byte_val = parse_hex(byte_hex);
                if (byte_val > 0xFF) {
                    printf("error: byte value > 0xFF in T record\n");
                    free(memory);
                    for (int j = 0; j < line_count; j++) {
                        free(lines[j]);
                    }
                    exit(1);
                }

                memory[mem_index++] = (unsigned char)byte_val;
                k += 2;
            }
        }
    }

    /*
        PASS 2 (XE):
        Now I go through all M records and apply relocation to the
        specific fields they point to.
    */
    for (int i = 0; i < line_count; i++) {
        if (lines[i][0] == 'M') {
            char addr_str[7];
            char len_str[3];
            char sign_char;

            parse_M_record(lines[i], addr_str, len_str, &sign_char);

            /* Address in the program where this field starts. */
            unsigned int field_addr = parse_hex(addr_str);

            /* Length of the field in half-bytes. */
            unsigned int half_bytes = parse_hex(len_str);

            /* Number of full bytes in the field. */
            unsigned int num_bytes = (half_bytes + 1) / 2;

            /* Calculate offset into memory buffer. */
            unsigned int offset = field_addr - orig_start;
            if (offset + num_bytes > program_length) {
                printf("error: M record exceeds program length\n");
                free(memory);
                for (int j = 0; j < line_count; j++) {
                    free(lines[j]);
                }
                exit(1);
            }
            /*
                I read the field as a big-endian unsigned int.

                Example: if num_bytes = 3 and memory[offset..offset+2]
                is 00 10 00, I build value = 0x001000.
            */
            unsigned int value = 0;
            for (unsigned int b = 0; b < num_bytes; b++) {
                value = (value << 8) | memory[offset + b];
            }

            /* Apply relocation (+ or - reloc). If sign is weird, I just add. */
            if (sign_char == '+') {
                value += reloc;
            } else if (sign_char == '-') {
                value -= reloc;
            } else {
                value += reloc;
            }
            /*
                I create a mask to make sure I only keep the right number
                of bytes in the value after relocation.
            */
            unsigned int mask = 0xFFFFFFFF;
            if (num_bytes == 3) mask = 0x00FFFFFF;
            else if (num_bytes == 2) mask = 0x0000FFFF;
            else if (num_bytes == 1) mask = 0x000000FF;

            value &= mask;

            /*
                Now I write the adjusted value back into memory in
                big-endian form (most significant byte first).
            */
            for (int b = (int)num_bytes - 1; b >= 0; b--) {
                memory[offset + b] = (unsigned char)(value & 0xFF);
                value >>= 8;
            }
        }

  }

    /*
        PASS 3 (XE):
        At this point, the memory buffer contains the fully relocated
        program. Now I rebuild and print out the relocated T and E records.

        For T:
            - I relocate the start address.
            - I grab the correct bytes from memory and print them as hex.

        For E:
            - I relocate the execution address if there is one.
    */
    for (int i = 0; i < line_count; i++) {
        if (lines[i][0] == 'T') {
            char addr_str[7];
            char len_str[3];
            char obj_str[256]; /* not used for printing; we rebuild from memory */
            parse_T_record(lines[i], addr_str, len_str, obj_str);

            unsigned int start_addr = parse_hex(addr_str);
            unsigned int length_bytes = parse_hex(len_str);

            unsigned int offset = start_addr - orig_start;

            /* Compute relocated start address for this T record. */
            unsigned int relocated_start = start_addr + reloc;

            /* First print the T, relocated address, and length. */
            printf("T%06X%02X", relocated_start, length_bytes);

            /*
                Now I walk through the memory buffer and print each byte
                as a two-digit hex value, same count as length_bytes.
            */
            for (unsigned int b = 0; b < length_bytes; b++) {
                printf("%02X", memory[offset + b]);
            }

            printf("\n");
        } else if (lines[i][0] == 'E') {
            char exec_str[7];
            parse_E_record(lines[i], exec_str);

            /* If E has no address, I just print "E". */
            if (exec_str[0] == '\0') {
                printf("E\n");
            } else {
                /* Otherwise, relocate the execution address and print it. */
                unsigned int exec = parse_hex(exec_str);
                exec += reloc;
                printf("E%06X\n", exec);
            }
        }
        /* I skip H and M here because the output should only be T and E. */
    }

    /* Finally, I free the memory buffer and all the stored lines. */
    free(memory);
    for (int i = 0; i < line_count; i++) {
        free(lines[i]);
    }
}

/*
    This is the function that main.c calls.

    - object: already opened FILE* for the object file
    - new_start: relocation start address from the command line
    - machine_type: 0 for SIC, 1 for SICXE

    I just pick which relocation path to follow based on machine_type.
*/
int project5loader(FILE *object, unsigned int new_start, int machine_type) {

    if (machine_type == 0) {
        /* Do plain SIC relocation. */
        relocate_sic(object, new_start);
    } else {
        /* Do SIC/XE relocation using M records. */
        relocate_sicxe(object, new_start);
    }

    return 0;
}

