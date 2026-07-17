#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <openssl/evp.h>

typedef uint8_t u8;

void print_checksum(FILE *input, int flagValue, FILE *output) {
    unsigned char buffer[4096];
    size_t bytes_read;
    EVP_MD_CTX *mdctx;
    const EVP_MD *md;
    unsigned char md_value[EVP_MAX_MD_SIZE];
    unsigned int md_len, i;

    switch(flagValue) {
        case 1:
            md = EVP_md5();
            fprintf(output, "MD5:    ");
            break;
        case 2:
            md = EVP_sha1();
            fprintf(output, "SHA1:   ");
            break;
        case 3:
            md = EVP_sha256();
            fprintf(output, "SHA256: ");
            break;
        case 4:
            md = EVP_sha512();
            fprintf(output, "SHA512: ");
            break;
        case 5:
            md = EVP_sha224();
            fprintf(output, "SHA224: ");
            break;
        case 6:
            md = EVP_sha384();
            fprintf(output, "SHA384: ");
            break;
        default:
            fprintf(stderr, "Unknown digest\n");
            return;
    }

    mdctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(mdctx, md, NULL);

    while ((bytes_read = fread(buffer, 1, sizeof(buffer), input)) > 0) {
        EVP_DigestUpdate(mdctx, buffer, bytes_read);
    }

    EVP_DigestFinal_ex(mdctx, md_value, &md_len);
    EVP_MD_CTX_free(mdctx);

    for (i = 0; i < md_len; i++) {
        fprintf(output, "%02x", md_value[i]);
    }
    
    fprintf(output, "\n");
    fflush(output);
} 
