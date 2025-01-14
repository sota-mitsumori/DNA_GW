#include <stdio.h>
#include <stdlib.h>
#include "grpwk24.h"
#define REPEAT 100

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

  unsigned char c1, c2, res;
  for(int i=0; i<ORGDATA_LEN; i+=2){
    c1 = getc(ofp);
    c2 = getc(ofp);
    
    switch( ( (c1 & 0x1) << 7) >> 6 | ( c2 & 0x1) ){
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

    // 決まった塩基以外の塩基3種を入れる (塩基が連続しない順にしている)
    if (res == BASE_A) {
        for (int j=0; j<REPEAT; j++){
            fputc(BASE_C, efp);
            fputc(BASE_G, efp);
            fputc(BASE_T, efp);
        }
    }else if (res == BASE_C) {
        for (int j=0; j<REPEAT; j++){
            fputc(BASE_A, efp);
            fputc(BASE_G, efp);
            fputc(BASE_T, efp);
        }
    }else if (res == BASE_G) {
        for (int j=0; j<REPEAT; j++){
            fputc(BASE_A, efp);
            fputc(BASE_C, efp);
            fputc(BASE_T, efp);
        }
    }else if (res == BASE_T) {
        for (int j=0; j<REPEAT; j++){
            fputc(BASE_A, efp);
            fputc(BASE_C, efp);
            fputc(BASE_G, efp);
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
