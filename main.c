#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>
#include <stddef.h>
#include <string.h> 

#include "hexDumper.h"
#include "scanner.h"
#include "checksum.h"
#include "file_handler.h"
#include "sigdb.h"
#include "window.h"

int main(int argc, char **argv) {

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--gui") == 0) {
            for (int j = i; j < argc - 1; j++)
                argv[j] = argv[j + 1];
            argc--;
            gui_launch(argc, argv);
            return 0;
        }
    }

    int   flag_hexDump = 0;
    int   flag_identify = 0;
    int   flag_output = 0;

    int   flag_directoryScan = 0;

    int   flag_hexByteOutput = 0;
    int   flag_reverseHex = 0;
    int   flag_Compact = 0;
    int   flag_LowerCase = 0;
    int   flag_stringExtract = 0;

    int   checksum_flag_value = 0;

    // m | checksum_flag_value = 1
    int   checksum_MD5 = 0;
    // 1 | checksum_flag_value = 2
    int   checksum_SHA1 = 0;
    // 2 | checksum_flag_value = 3
    int   checksum_SHA256 = 0;
    // 3 | checksum_flag_value = 4
    int   checksum_SHA512 = 0;
    // 4 | checksum_flag_value = 5
    int   checksum_SHA224 = 0;
    // 5 | checksum_flag_value = 6
    int   checksum_SHA384 = 0;

    const char *dir_name;


    char *outpath = NULL;
    int   opt;

    sigdb_load("signature.txt");
    
    while ((opt = getopt(argc, argv, "hio:M12345d:BrCLS")) != -1) {
        switch (opt) {
            case 'h':
                flag_hexDump = 1;
                break;
            case 'i':
                flag_identify = 1;
                break;
            case 'o':
                flag_output  = 1;
                outpath = optarg;
                break;
            case 'B':
                flag_hexByteOutput = 1;
                break;
            case 'r':
                flag_reverseHex = 1;
                break;
            case 'C':
                flag_Compact = 1;
                break;
            case 'L':
                flag_LowerCase = 1;
                break;
            case 'd':
                flag_directoryScan = 1;
                dir_name = optarg;
                break;
            case 'S':
                flag_stringExtract = 1;
                break;
            case 'M':
                checksum_MD5 = 1;
                break;
            case '1':
                checksum_SHA1 = 1;
                break;
            case '2':
                checksum_SHA256 = 1;
                break;
            case '3':
                checksum_SHA512 = 1;
                break;
            case '4':
                checksum_SHA224 = 1;
                break;
            case '5':
                checksum_SHA384 = 1;
                break;
            default:
                fprintf(stderr, "usage: hexDumper [-h] [-o outfile] <file>\n");
                return EXIT_FAILURE;
        }
    }

    BinaryFileHandle *b_file = NULL;
    FILE *input;

    if (optind < argc) {
        b_file = bfile_open(argv[optind]);
        if (!b_file) { 
            perror("bfile_open"); 
            return EXIT_FAILURE; 
        }
        input = bfile_raw(b_file);
    } else {
        input = stdin;
    }

    FILE *output;

    if (flag_output && outpath) {
        output = fopen(outpath, "w");
        if (!output) { 
            perror("fopen output"); 
            return EXIT_FAILURE; 
        }
    } else {
        output = stdout;
    }

    // Validate that at least one action flag is set
    if (!flag_directoryScan && !flag_identify && !flag_hexDump && !flag_reverseHex 
        && !checksum_MD5 && !checksum_SHA1 && !checksum_SHA256 
        && !checksum_SHA512 && !checksum_SHA224 && !checksum_SHA384 
        && !flag_stringExtract) {
            
            fprintf(stderr,
                "usage: hexDumper [-h] [-i] [-B] [-r] [-C] [-L] [-S]\n"
                "                 [-M] [-1] [-2] [-3] [-4] [-5] [-d dir] [-o outfile] <file>\n");

        if (b_file) {
            bfile_close(b_file);
        } 

        if (output != stdout) {
            fclose(output);
        }
        return EXIT_FAILURE;
    }

    if (flag_directoryScan) {
        scan_directory(dir_name, output);
    }

    if (flag_identify) {
        uint8_t buf[16];
        size_t  n = fread(buf, 1, 16, input);

        // use sigdb if signatures were loaded, else fall back to scanner.c
        if (sigdb_count() > 0) {
            fprintf(output, "Type: %s\n", sigdb_identify(buf, n));
        } else {
            rewind(input);
            identify_file(input, output);
        }
        rewind(input);
    }

    if (flag_hexDump) {
        dump_hex(input, output, flag_LowerCase, flag_Compact, flag_hexByteOutput);
        rewind(input);
    }

    if (flag_reverseHex) {
        reverse_dump(input, output);
        rewind(input);
    }

    if (checksum_MD5) {
        checksum_flag_value = 1;
        print_checksum(input, checksum_flag_value, output);
        bfile_seek(b_file, 0);
    }

    if (checksum_SHA1) {
        checksum_flag_value = 2;
        print_checksum(input, checksum_flag_value, output);
        bfile_seek(b_file, 0);
    }

    if (checksum_SHA256) {
        checksum_flag_value = 3;
        print_checksum(input, checksum_flag_value, output);
        bfile_seek(b_file, 0);
    }

    if (checksum_SHA512) {
        checksum_flag_value = 4;
        print_checksum(input, checksum_flag_value, output);
        bfile_seek(b_file, 0);
    }

    if (checksum_SHA224) {
        checksum_flag_value = 5;
        print_checksum(input, checksum_flag_value, output);
        bfile_seek(b_file, 0);
    }

    if (checksum_SHA384) {
        checksum_flag_value = 6;
        print_checksum(input, checksum_flag_value, output);
        bfile_seek(b_file, 0);
    }

    if (flag_stringExtract) {
        extract_strings(input, output);
    }

    if (b_file) {
        bfile_close(b_file);
    }

    if (output != stdout) {
        fclose(output);
    }

    return EXIT_SUCCESS;
} 
