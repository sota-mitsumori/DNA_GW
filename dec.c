#include <stdio.h>
#include <stdlib.h>
#include "grpwk24.h"

#define REPEAT     100
#define REPEAT_3   (REPEAT * 3)
#define BINARY     2

// We will handle up to 3 lines from seqdata.
#define N_LINES        3

// Maximum number of characters in one line of seqdata.
#define MAX_LINE_LEN   60000000

// Each line decodes into up to (ORGDATA_LEN) bits (because ORGDATA_LEN/2 chunks * 2 bits each).
// We'll store those bits as ASCII '0'/'1' in a buffer of size up to ORGDATA_LEN + 1 for safety.
#define MAX_BITS       (ORGDATA_LEN)

// -------------------------------------------------------------------------
// A helper function to get the length of a C-string without using <string.h>
static int my_strlen(const char *s) {
    int n = 0;
    while(s[n] != '\0') {
        n++;
    }
    return n;
}

// -------------------------------------------------------------------------
// A helper to remove a trailing newline character (if present) at the end of a line.
static void remove_trailing_newline(char *s) {
    int len = my_strlen(s);
    if(len > 0 && s[len - 1] == '\n') {
        s[len - 1] = '\0';
    }
}

// -------------------------------------------------------------------------
// Clears (sets to '\0') an array of characters, up to `size`.
static void clear_buffer(char *buf, int size) {
    for(int i = 0; i < size; i++){
        buf[i] = '\0';
    }
}

// -------------------------------------------------------------------------
// decode_one_line:
//   Decodes a single line of A/C/G/T data using your original block-based method:
//     - For each chunk of REPEAT_3 bases, we pick the "least frequent base" => 2 bits
//     - Stop if we run out of valid bases or reach non-ACGT char.
//
//   linebuf = the entire line of seqdata (no trailing newline).
//   outbits = buffer to store decoded '0'/'1' bits.
//
//   Returns how many bits were successfully decoded.
//
static int decode_one_line(const char *linebuf, char *outbits)
{
    // Total number of 3*REPEAT blocks we expect = (ORGDATA_LEN/2) 
    // (because each chunk is 2 bytes => 2 bits => written as 3*REPEAT bases).
    // We decode in increments of REPEAT_3 from the line.
    int encdata_size = (ORGDATA_LEN / BINARY) * REPEAT_3;
    
    int line_len = 0;  // we'll find the length manually
    // measure length of linebuf
    while(linebuf[line_len] != '\0') {
        line_len++;
    }

    int pos = 0;        // current index into linebuf
    int bitcount = 0;   // how many bits we've written to outbits

    // i steps in increments of REPEAT_3 in terms of "ideal" blocks
    // but we just do this for a maximum of encdata_size / REPEAT_3 blocks.
    // We'll break early if we run out of data.
    for(int i = 0; i < encdata_size; i += REPEAT_3) {
        int countA = 0, countC = 0, countG = 0, countT = 0;
        int end = 0;

        // Read REPEAT_3 bases
        for(int j = 0; j < REPEAT_3; j++) {
            if(pos >= line_len) {
                // no more data
                end = 1;
                break;
            }
            unsigned char c = (unsigned char)linebuf[pos++];
            if(c == BASE_A) {
                countA++;
            } else if(c == BASE_C) {
                countC++;
            } else if(c == BASE_G) {
                countG++;
            } else if(c == BASE_T) {
                countT++;
            } else {
                // If we see something that's not A/C/G/T, break
                end = 1;
                break;
            }
        }

        if(end == 1) {
            break;  // done decoding this line
        }

        // Find the least frequent among A/C/G/T
        int min_count = countA;
        unsigned char bits = 0; // 0 => A => 00
        if(countC < min_count) {
            min_count = countC;
            bits = 1; // => 01
        }
        if(countG < min_count) {
            min_count = countG;
            bits = 2; // => 10
        }
        if(countT < min_count) {
            min_count = countT;
            bits = 3; // => 11
        }

        // Write two bits (as ASCII '0'/'1')
        // e.g. bits=2 (binary 10) => outbits[bitcount]='1'; outbits[bitcount+1]='0';
        outbits[bitcount++] = (char)('0' + ((bits >> 1) & 1));
        outbits[bitcount++] = (char)('0' + ( bits       & 1));
    }

    return bitcount;
}

// -------------------------------------------------------------------------
int dec(){
    FILE *sfp = fopen(SEQDATA, "r");
    if(!sfp) {
        fprintf(stderr, "cannot open %s\n", SEQDATA);
        exit(1);
    }

    FILE *dfp = fopen(DECDATA, "w");
    if(!dfp) {
        fprintf(stderr, "cannot open %s\n", DECDATA);
        exit(1);
    }

    // We'll store up to 3 lines from seqdata
    // linebuf[i] = the raw line read from file
    static char linebuf[N_LINES][MAX_LINE_LEN];
    // decbuf[i] = the decoded bits from that line
    static char decbuf[N_LINES][MAX_BITS + 1]; // +1 for safety

    // We'll track how many bits we decode from each line
    int bitcounts[N_LINES];
    // Initialize bitcounts to 0
    for(int i = 0; i < N_LINES; i++){
        bitcounts[i] = 0;
    }

    // Read up to 3 lines from seqdata
    int lines_read = 0;
    for(int i = 0; i < N_LINES; i++){
        // clear the line buffer
        clear_buffer(linebuf[i], MAX_LINE_LEN);

        // attempt to read a line
        if(!fgets(linebuf[i], MAX_LINE_LEN, sfp)) {
            // no more lines in file
            break;
        }

        // remove trailing newline if present
        remove_trailing_newline(linebuf[i]);

        // decode the line => bits
        clear_buffer(decbuf[i], MAX_BITS + 1);
        bitcounts[i] = decode_one_line(linebuf[i], decbuf[i]);

        lines_read++;
    }

    // Now we have lines_read lines decoded in decbuf[0..lines_read-1]
    // We'll do a bitwise majority across them.
    // We take the minimum bit length among them for the final output.
    int min_bits = 999999999;
    for(int i = 0; i < lines_read; i++){
        if(bitcounts[i] < min_bits){
            min_bits = bitcounts[i];
        }
    }
    if(min_bits == 999999999){
        min_bits = 0;  // means no lines were decoded
    }

    // For each position in [0..min_bits-1],
    // count how many of the lines have '1'
    for(int pos = 0; pos < min_bits; pos++){
        int count_ones = 0;
        for(int i = 0; i < lines_read; i++){
            if(decbuf[i][pos] == '1'){
                count_ones++;
            }
        }
        // If count_ones >= majority threshold, output '1', else '0'
        // majority threshold = (lines_read+1)/2
        int needed = (lines_read + 1) / 2;
        if(count_ones >= needed){
            fputc('1', dfp);
        } else {
            fputc('0', dfp);
        }
    }

    // end with a newline
    fputc('\n', dfp);

    fclose(dfp);
    fclose(sfp);
    return 0;
}

int main(){
    dec();
    return 0;
}
