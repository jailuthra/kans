/**
 * @file
 * Dekompress
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "kans.h"

void read_header(FILE *fp, struct ANSCtx *a)
{
    /* Read identifier code */
    uint8_t byte;
    uint16_t magic;
    fread(&byte, sizeof(uint8_t), 1, fp);
    magic = byte;
    fread(&byte, sizeof(uint8_t), 1, fp);
    magic <<= 8;
    magic |= byte;
    if (!(magic == 0xA105)) {
        fprintf(stderr, "File is not in KMP format\n");
        exit(EXIT_FAILURE);
    }
    /* Read number of symbols in table */
    fread(&byte, sizeof(uint8_t), 1, fp);
    int n = byte, i;
    /* Read final state */
    fread(&a->state, sizeof(uint32_t), 1, fp);
    uint8_t symb;
    uint16_t fs;
    /* Read frequency table */
    for (i=0; i<n; i++) {
        /* Read symbol code */
        fread(&byte, sizeof(uint8_t), 1, fp);
        symb = byte;
        /* Read symbol frequency */
        fread(&fs, sizeof(uint16_t), 1, fp);
        a->ftable[symb].f = fs;
    }
    /* Update frequency table */
    for (int i = 0; i < ALPHLEN; i++) {
        a->ftable[i].maxX = MAX_X * a->ftable[i].f - 1;
        a->ftable[i].c = ((i > 0) ? (a->ftable[i-1].c + a->ftable[i-1].f) : 0);
    }
}

int main(int argc, char *argv[])
{
    FILE *kmp, *fp = stdout;
    uint8_t byte, symb;
    uint16_t chunk;
    int i;
    if (argc == 2) {
        if (!(kmp = fopen(argv[1], "rb"))) {
            fprintf(stderr, "Couldn't open file %s\n", argv[1]);
            exit(EXIT_FAILURE);
        }
    } else {
        printf("USAGE: %s <infile.kmp>\n", argv[0]);
        exit(EXIT_SUCCESS);
    }
    struct ANSCtx *a = malloc(sizeof(struct ANSCtx));
    a->ftable = malloc(sizeof(struct ftnode) * ALPHLEN);
    a->b = STREAM_RANGE;
    /* set initial frequencies to 0 */
    for (int i = 0; i < ALPHLEN; i++) {
        a->ftable[i].n = 0;
    }
    /* read final state and frequency table from header */
    read_header(kmp, a);
    /* decode */
    uint8_t status = 1;
    while (a->state != 1) {
        uint8_t symb = ANSDecompress(a);
        fwrite(&symb, sizeof(uint8_t), 1, stdout);
        /*printf("s:0x%02x, x: 0x%08x\n", symb, a->state);*/
        while (status && a->state < STREAM_RANGE) {
            fread(&chunk, sizeof(uint16_t), 1, kmp);
            /*printf("STREAM 0x%04x\n", chunk);*/
            if (feof(kmp)) {
                status = 0;
                break;
            }
            a->state = (a->state << STREAM_BITS) + chunk;
        }
        /*printf("x': 0x%08x\n", a->state);*/
    }
    fclose(kmp);
    return 0;
}
