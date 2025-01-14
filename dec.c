#include <stdio.h>
#include <stdlib.h>
#include <string.h>   // for strlen
#include "grpwk24.h"

#define REPEAT   50
#define REPEAT_3 (REPEAT * 3)
#define BINARY   2

// We'll handle exactly 3 lines from seqdata.
// If you have more, increase N_LINES or make idynamict .
#define N_LINES  5

// Maximum length of one line in seqdata.
// Must be at least enough to hold "encdata_size" characters, plus newline.
#define MAX_LINE_LEN  60000000

// Each line decodes into a certain number of bits = (ORGDATA_LEN/2)*2 characters
// Let's define a safe buffer for storing the decoded bits from one line.
#define MAX_BITS  (ORGDATA_LEN)

// ---------------------------------------------------------
// decode_one_line(): 
//   - Uses your ORIGINAL chunk-based logic, 
//   - But reads from a line buffer in memory instead of from a file pointer.
//   - Fills outbits[] with the decoded '0'/'1' characters.
//
//   Return value: how many bits we successfully decoded.
//
static int decode_one_line(const char *linebuf, char *outbits)
{
    // We'll parse linebuf using an index pos.
    // Whenever we read REPEAT_3 valid bases, we pick the min, produce 2 bits, etc.
    // We'll do up to encdata_size times.

    int encdata_size = (ORGDATA_LEN / BINARY) * REPEAT_3; 
    // This is how many bases we expect if there were zero insertions/deletions.

    int line_len = (int)strlen(linebuf);  // number of chars in line, ignoring final '\n' if present
    int pos = 0;                          // current index in linebuf
    int bitcount = 0;                     // how many bits we wrote to outbits

    // We replicate your original loop structure:
    // for i in [0..encdata_size..REPEAT_3], we read REPEAT_3 bases
    // But we must watch for pos >= line_len or newline.

    for(int i = 0; i < encdata_size; i += REPEAT_3) {
        int countA = 0, countC = 0, countG = 0, countT = 0;
        int end   = 0;  // if we can't read enough chars, we'll break

        // Read REPEAT_3 bases from linebuf
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
                // If there's an unexpected char (like '\n' or random),
                // we skip but note that we didn't get a valid base.
                // This is up to you if you want to break or skip.
                // We'll break if it's truly a newline or invalid.
                end = 1;
                break;
            }
        }

        if(end == 1){
            break;
        }

        // Find the base with the MIN count => that was "excluded" in enc.c
        int min_count = countA;
        unsigned char bits = 0; // assume A => 0
        if(countC < min_count){
            min_count = countC; bits = 1; // 01
        }
        if(countG < min_count){
            min_count = countG; bits = 2; // 10
        }
        if(countT < min_count){
            min_count = countT; bits = 3; // 11
        }

        // bits => 2 ASCII '0'/'1'
        outbits[bitcount++] = (char)('0' + ((bits >> 1) & 1)); 
        outbits[bitcount++] = (char)('0' + ( bits       & 1));
    }

    // Return how many bits we decoded
    return bitcount;
}

// ---------------------------------------------------------
int dec(){
    // We'll read up to 3 lines from SEQDATA.
    // For each line, we decode it (store into decbuf[i]).
    // Then we do a majority vote across decbuf[0..2].

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

    // We'll store each line in linebuf[i], decode into decbuf[i].
    static char linebuf[N_LINES][MAX_LINE_LEN];
    static char decbuf[N_LINES][MAX_BITS+2];  // each line's decoded bits
                                              // +2 for safety

    // Read up to N_LINES lines from seqdata
    int lines_read = 0;
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
        lines_read++;
    }

    // Now decode each line into decbuf[i]
    int bitcounts[N_LINES];
    for(int i=0; i<lines_read; i++){
        memset(decbuf[i], 0, sizeof(decbuf[i]));
        int bc = decode_one_line(linebuf[i], decbuf[i]);
        bitcounts[i] = bc;  // how many bits we got
    }

    // If we read fewer than 3 lines, or some lines are short, that's okay.
    // We'll do a final majority vote only up to the minimal length among them.
    // Because each line might decode a different number of bits.

    int min_bits = 999999999;
    for(int i=0; i<lines_read; i++){
        if(bitcounts[i] < min_bits){
            min_bits = bitcounts[i];
        }
    }
    if(min_bits == 999999999){
        min_bits = 0; // no lines read
    }

    // We'll do a majority across the first 'min_bits' positions
    // among lines_read lines. If lines_read<3, we only have 1 or 2 lines => do best you can.
    for(int pos=0; pos<min_bits; pos++){
        int count1 = 0; // how many '1' among the lines
        for(int i=0; i<lines_read; i++){
            if(decbuf[i][pos] == '1'){
                count1++;
            }
        }
        // If majority of lines have '1', we output '1'
        // "Majority" = count1 > lines_read/2
        int needed = (lines_read+1)/2;  // e.g. if lines_read=3 => needed=2
        if(count1 >= needed){
            fputc('1', dfp);
        } else {
            fputc('0', dfp);
        }
    }

    // end with newline
    fputc('\n', dfp);

    fclose(dfp);
    fclose(sfp);
    return 0;
}

int main(){
    dec();
    return 0;
}
