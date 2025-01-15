#include <stdio.h>
#include <stdlib.h>
#include "grpwk24.h"
#define REPEAT 300
#define REPEAT_3 REPEAT * 3
#define BINARY 2

int dec(){
    FILE *sfp;
    if((sfp = fopen(SEQDATA, "r")) ==NULL){
        fprintf(stderr, "cannot open %s\n", SEQDATA);
        exit(1);
    }

    FILE *dfp;
    if((dfp = fopen(DECDATA, "w")) ==NULL){
        fprintf(stderr, "cannot open %s\n", DECDATA);
        exit(1);
    }

    unsigned char c, res;
    int encdata_size = ORGDATA_LEN / BINARY * (REPEAT_3);

    for (int i = 0; i < encdata_size; i += REPEAT_3) {
        int count_A = 0, count_T = 0, count_G = 0, count_C = 0;
        int end = 0;

        // 各塩基の数を計測
        for (int j = 0; j < REPEAT_3; j++) {
            c = getc(sfp);
            if (c == BASE_A) {
                count_A++;
            } else if (c == BASE_T) {
                count_T++;
            } else if (c == BASE_G) {
                count_G++;
            } else if (c == BASE_C) {
                count_C++;
            } 
            else {
                // c=="\n"の場合ループ終了
                printf("i=%d, j=%d, c=%d", i, j, c);
                end = 1;
                break;
            }
        }
        if (end == 1) {
            break;
        }

        // 一番数が少ない塩基を決定
        int min_count = count_A;
        if (count_T < min_count) {
            min_count = count_T;
        }
        if (count_G < min_count) {
            min_count = count_G;
        }
        if (count_C < min_count) {
            min_count = count_C;
        }

        // 一番数が少ない塩基を復元
        if (min_count == count_A) {
            res = 0;
        }else if (min_count == count_C) {
            res = 1;
        }else if (min_count == count_G) {
            res = 2;
        }else if (min_count == count_T) {
            res = 3;
        }
        fputc((res>>1)+'0', dfp);
        fputc((res&0x1)+'0', dfp);
    } 
    res = '\n';
    fputc(res, dfp);
        
    fclose(sfp);
    fclose(dfp);
    return(0);
}

int main(){
    dec();
    return(0);
}
