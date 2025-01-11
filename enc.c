#include <stdio.h>
#include <stdlib.h>
#include "grpwk24.h"


int enc(){
  FILE *ofp;
  if((ofp = fopen(ORGDATA, "r")) ==NULL){
    fprintf(stderr, "cannot open %s\n", ORGDATA);
    exit(1);
  }

  FILE *efp;
  if((efp = fopen(ENCDATA, "w")) ==NULL){
    fprintf(stderr, "cannot open %s\n", ENCDATA);
    exit(1);
  }

  unsigned char res;

  for(int i=0; i<ORGDATA_LEN; i+=12){
    
    for (int j=0;j<2;j++) {
      fputc(BASE_A, efp); // 文字数稼ぎ
    }
    unsigned char  str[46] = {0};
    for (int j = 0; j < 16; j++) {
          str[15 - j] = (i >> j) & 1; // jビット目を取得しリストに格納（MSBがリストの最初になるように逆順）
    }

    for (int j = 0; j < 16; j+=2) {
          switch( ( (str[j] & 0x1) << 7) >> 6 | ( str[j+1] & 0x1) ){
        case 0:
          res = BASE_A;
          break;
        case 1:
          res = BASE_C;      
          break;
        case 2:
          res = BASE_G;      
          break;
        case 3:
          res = BASE_T;      
          break;
        }
        fputc(res, efp);
    }

    
    for (int j = 16; j < 37; j+=7) {
      str[j] = getc(ofp);
      str[j+1] = getc(ofp);
      str[j+2] = getc(ofp);
      str[j+3] = getc(ofp);
      for (int k = j; k < j+4; k+=2) {
          switch( ( (str[k] & 0x1) << 7) >> 6 | ( str[k+1] & 0x1) ){
        case 0:
          res = BASE_A;
          break;
        case 1:
          res = BASE_C;      
          break;
        case 2:
          res = BASE_G;      
          break;
        case 3:
          res = BASE_T;      
          break;
        }
        fputc(res, efp);
    }

      str[j+4] = (str[j] & 1) ^ (str[j+1] & 1) ^ (str[j+3] & 1);
      str[j+5] = (str[j] & 1) ^ (str[j+2] & 1) ^ (str[j+3] & 1);
      str[j+6] = (str[j+1] & 1) ^ (str[j+2] & 1) ^ (str[j+3] & 1);

      for (int k = j+4; k < j+7; k++) {
        if (str[k] == 1) {
          res = BASE_C;
        } else {
          res = BASE_G;
        }
        fputc(res, efp);
      }
    }

  }
    
  res = '\n';
  fputc(res, efp);
  
  
  fclose(ofp);
  fclose(efp);
  return(0);
}

int main(){
  enc();
  return(0);
}