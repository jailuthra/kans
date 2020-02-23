/**
 * @file
 * Kompress
 */

/* TODO check if file is empty */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include "kans.h"

void write_header(FILE *fp, struct ANSCtx *a)
{
    uint8_t byte;
    uint16_t fs;
    int n = 0, i, j, k;
    /* Write identifying hex code 0xA105 */
    putc(0xA1, fp);
    putc(0x05, fp);
    /* Write number of symbols */
    for (i=0; i<ALPHLEN; i++) {
        if (a->ftable[i].f > 0) {
            n++;
        }
    }
    byte = n; fwrite(&byte, sizeof(uint8_t), 1, fp);
    /* Write final state of encoder */
    fwrite(&a->state, sizeof(uint32_t), 1, fp);
    /* Writing the frequency table */
    for (i=0; i<ALPHLEN; i++) {
        if (a->ftable[i].f > 0) {
            /* Write ascii code */
            byte = i;
            fwrite(&byte, sizeof(uint8_t), 1, fp);
            /* Write fs */
            fs = (uint16_t) a->ftable[i].f;
            fwrite(&fs, sizeof(uint16_t), 1, fp);
        }
    }
}

/** Create a frequency table from the input stream.
 *
 * @param[in] fp Pointer to input stream
 * @param[out] freq Frequency table
 */
void create_freq_table(FILE *fp, struct ftnode *freq)
{
    uint8_t byte;
    /* set initial frequencies to 0 */
    for (int i = 0; i < ALPHLEN; i++) {
        freq[i].n = 0;
    }
    uint64_t count = 0;
    /* update frequencies of each ascii value from file */
    while (fread(&byte, sizeof(uint8_t), 1, fp) == 1) {
        freq[byte].n += 1;
        count += 1;
    }
    /* use frequencies to assign a portion of the working range */
    for (int i = 0; i < ALPHLEN; i++) {
        freq[i].f = (uint32_t) (ANS_RANGE * ((double)freq[i].n / count));
        if (freq[i].n > 0 && freq[i].f == 0) {
            freq[i].f = 1;
        }
        freq[i].maxX = MAX_X * freq[i].f - 1;
        freq[i].c = ((i > 0) ? (freq[i-1].c + freq[i-1].f) : 0);
    }
}

/** Prepare ANS tables for encoding.
 * @param[in] fp Pointer to input stream
 * @param[out] a ANS encoder/decoder context
 */
void prepare_ans(FILE *fp, struct ANSCtx *a)
{
    a->ftable = malloc(sizeof(struct ftnode) * ALPHLEN);
    create_freq_table(fp, a->ftable);
    a->b = STREAM_RANGE;
    /*for (int i = 0; i < ALPHLEN; i++) {
        if (a->ftable[i].n > 0) {
            printf("%02x: %d, %d\n", i, a->ftable[i].f, a->ftable[i].c);
        }
    }*/
}

int main(int argc, char *argv[])
{
    FILE *fp, *kans;
    int i;
    if (argc == 3) {
        if (!(kans = fopen(argv[1], "wb"))) {
            fprintf(stderr, "Couldn't open file %s\n", argv[1]);
            exit(EXIT_FAILURE);
        }
        if (!(fp = fopen(argv[2], "r"))) {
            fprintf(stderr, "Couldn't open file %s\n", argv[2]);
            exit(EXIT_FAILURE);
        }
    } else {
        printf("USAGE: %s <outfile.kans> <infile.txt>\n", argv[0]);
        exit(EXIT_SUCCESS);
    }
    struct ANSCtx *a = malloc(sizeof(struct ANSCtx));
    /* Prepare ANS from file */
    prepare_ans(fp, a);
    /* Re-read input file, write each symbol's huffcode to temp file */
    rewind(fp);
    a->state = 1UL;
    FILE *temp = tmpfile();
    uint8_t byte;
    uint16_t chunk;
    fseek(fp, -1, SEEK_END);
    while (1) {
        fread(&byte, sizeof(uint8_t), 1, fp);
        /*printf("s:0x%02x, x: 0x%08x\n", byte, a->state);*/
        while (a->state > a->ftable[byte].maxX) {
            /* STREAM */
            chunk = a->state % a->b;
            /*printf("STREAM 0x%04x\n", chunk);*/
            fwrite(&chunk, sizeof(uint16_t), 1, temp);
            a->state /= a->b;
        }
        ANSCompress(a, byte);
        /*printf("x': 0x%08x\n", a->state);*/
        uint8_t temp = ANSDecompress(a);
        /*printf("VERIFY x: 0x%08x, s: 0x%02x\n", a->state, temp);*/
        ANSCompress(a, byte);
        if (ftell(fp) <= 1) {
            break;
        } else {
            fseek(fp, -2, SEEK_CUR);
        }
    }
    fclose(fp);
    /* Write header to the output file */
    write_header(kans, a);
    /* Copy bitstream from tempfile to output file */
    rewind(temp);
    fseek(temp, -2, SEEK_END);
    while (1) {
        fread(&chunk, sizeof(uint16_t), 1, temp);
        fwrite(&chunk, sizeof(uint16_t), 1, kans);
        if (ftell(temp) <= 3) {
            break;
        } else {
            fseek(temp, -4, SEEK_CUR);
        }
    }
    fclose(temp);
    fclose(kans);
    return 0;
}

