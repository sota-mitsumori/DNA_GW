#include <stdio.h>
#include <stdlib.h>
#include "grpwk24.h"

int syn(){
  FILE *efp;
  if((efp = fopen(ENCDATA, "r")) ==NULL){
    fprintf(stderr, "cannot open %s\n", ENCDATA);
    exit(1);
  }

  FILE *dfp;
  if((dfp = fopen(SYNDNA, "w")) ==NULL){
    fprintf(stderr, "cannot open %s\n", SYNDNA);
    exit(1);
  }

  int cnt=0;
  unsigned char c, res;
  while((c = getc(efp)) != '\n' && cnt < MAX_SYN_SIZE){
    switch(c){
    case BASE_A:
      res = BASE_A;
      break;
    case BASE_C:
      res = BASE_C;      
      break;
    case BASE_G:
      res = BASE_G;      
      break;
    case BASE_T:
      res = BASE_T;      
      break;
    default:
      res = BASE_A;
      break;
    }      
    fputc(res, dfp);
    cnt++;
  }
  res = '\n';
  fputc(res, dfp);
    
  fclose(efp);
  fclose(dfp);
  return(0);
}

int main(){
  syn();
  return(0);
}
