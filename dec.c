#include <stdio.h>
#include <stdlib.h>
#include <string.h>   
#include "grpwk24.h"

#define REPEAT   80
#define REPEAT_3 (REPEAT * 3)
#define BINARY   2

// We'll read up to 3 lines from seqdata (3 NP runs).
#define N_LINES       3

// Maximum length of one line in seqdata.
#define MAX_LINE_LEN  60000000

// Each line decodes into at most (ORGDATA_LEN/2)*2 bits,
// but here we only need large enough to hold the original line of A/C/G/T.
#define MAX_BITS      (ORGDATA_LEN)

// -------------------------------------------------------------------------
// decode_one_line():
//   - Uses your ORIGINAL chunk-based logic on a single line of A/C/G/T.
//   - Reads the line in blocks of REPEAT_3 bases, finds the "least frequent base" -> 2 bits.
//   - Fills outbits[] with '0'/'1' characters and returns how many bits were decoded.
//
static int decode_one_line(const char *linebuf, char *outbits)
{
    int encdata_size = (ORGDATA_LEN / BINARY) * REPEAT_3;
    int line_len = (int)strlen(linebuf);
    int pos = 0;       
    int bitcount = 0;  

    // We replicate your original for-loop structure
    for(int i = 0; i < encdata_size; i += REPEAT_3) {
        int countA = 0, countC = 0, countG = 0, countT = 0;
        int end   = 0;

        // Read REPEAT_3 bases
        for(int j = 0; j < REPEAT_3; j++){
            if(pos >= line_len){
                // no more data in this line
                end = 1;
                break;
            }

            unsigned char c = (unsigned char) linebuf[pos++];
            if(c == BASE_A){
                countA++;
            } else if(c == BASE_C){
                countC++;
            } else if(c == BASE_G){
                countG++;
            } else if(c == BASE_T){
                countT++;
            } else {
                // unexpected or newline => stop
                end = 1;
                break;
            }
        }

        if(end == 1){
            break;
        }

        // Find base with MIN count => that was "excluded" in enc.c
        int min_count = countA;
        unsigned char bits = 0; // A => 0
        if(countC < min_count){
            min_count = countC; bits = 1; // C => 01
        }
        if(countG < min_count){
            min_count = countG; bits = 2; // G => 10
        }
        if(countT < min_count){
            min_count = countT; bits = 3; // T => 11
        }

        // Write these 2 bits as ASCII '0'/'1'
        outbits[bitcount++] = (char)('0' + ((bits >> 1) & 1)); 
        outbits[bitcount++] = (char)('0' + ( bits       & 1));
    }

    return bitcount;
}

// -------------------------------------------------------------------------
int dec(){
    // 1) Read up to 3 lines from seqdata
    // 2) Do majority vote at each character position => build consensusLine
    // 3) Decode consensusLine using your original method
    // 4) Write bits to decdata

    FILE *sfp = fopen(SEQDATA, "r");
    if(!sfp){
        fprintf(stderr, "cannot open %s\n", SEQDATA);
        exit(1);
    }

    FILE *dfp = fopen(DECDATA, "w");
    if(!dfp){
        fprintf(stderr, "cannot open %s\n", DECDATA);
        exit(1);
    }

    static char linebuf[N_LINES][MAX_LINE_LEN]; 
    int line_len[N_LINES];  // how many characters we read in each line
    int lines_read = 0;

    // Read up to 3 lines
    for(int i=0; i<N_LINES; i++){
        if(!fgets(linebuf[i], MAX_LINE_LEN, sfp)){
            // no more lines
            break;
        }
        // strip trailing newline if present
        size_t ll = strlen(linebuf[i]);
        if(ll>0 && linebuf[i][ll-1] == '\n'){
            linebuf[i][ll-1] = '\0';
        }
        line_len[i] = (int)strlen(linebuf[i]);
        lines_read++;
    }

    fclose(sfp);

    if(lines_read == 0){
        // no lines => no decoding
        // just write empty line to DECDATA
        fputc('\n', dfp);
        fclose(dfp);
        return 0;
    }

    // 2) Build the "consensusLine" by majority vote across lines_read lines
    // First, find the minimal length among the lines
    int minLen = line_len[0];
    for(int i=1; i<lines_read; i++){
        if(line_len[i] < minLen){
            minLen = line_len[i];
        }
    }
    // We'll only do majority up to minLen positions

    // Prepare a buffer for the consensus line
    static char consensusLine[MAX_LINE_LEN];
    // We'll fill positions [0..minLen-1], then add '\0'
    
    for(int col=0; col<minLen; col++){
        // Count frequency of A, C, G, T among all lines
        int freqA=0, freqC=0, freqG=0, freqT=0;
        for(int i=0; i<lines_read; i++){
            char b = linebuf[i][col];
            if(b == BASE_A) freqA++;
            else if(b == BASE_C) freqC++;
            else if(b == BASE_G) freqG++;
            else if(b == BASE_T) freqT++;
            // if it's something else, we ignore it
        }

        // Pick the base with the highest frequency (A,C,G,T)
        // If there's a tie, we can pick the first in that tie.
        // For example:
        char bestBase = BASE_A;
        int bestFreq  = freqA;

        if(freqC > bestFreq){
            bestBase = BASE_C; bestFreq = freqC;
        }
        if(freqG > bestFreq){
            bestBase = BASE_G; bestFreq = freqG;
        }
        if(freqT > bestFreq){
            bestBase = BASE_T; bestFreq = freqT;
        }

        consensusLine[col] = bestBase;
    }
    // null-terminate
    consensusLine[minLen] = '\0';

    // 3) Decode the consensusLine using the original chunk-based method
    static char decbits[MAX_BITS+2];
    memset(decbits, 0, sizeof(decbits));

    int nbits = decode_one_line(consensusLine, decbits);

    // 4) Write these bits to DECDATA
    for(int i=0; i<nbits; i++){
        fputc(decbits[i], dfp);
    }
    fputc('\n', dfp);

    fclose(dfp);
    return 0;
}

int main(){
    dec();
    return 0;
}
