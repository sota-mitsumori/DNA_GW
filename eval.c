#include <stdio.h>
#include <stdlib.h>
#include "grpwk24.h"

int eval(){
  FILE *ofp;
  if((ofp = fopen(ORGDATA, "r")) ==NULL){
    fprintf(stderr, "cannot open %s\n", ORGDATA);
    exit(1);
  }

  FILE *dfp;
  if((dfp = fopen(DECDATA, "r")) ==NULL){
    fprintf(stderr, "cannot open %s\n", DECDATA);
    exit(1);
  }

  unsigned char c1 = 0, c2 = 0;
  int hd=0;
  while(1){
    if(c1 != '\n')
      c1 = getc(ofp);
    if(c2 != '\n')
      c2 = getc(dfp);
    if(c1 != c2) hd++;
    if(c1 == '\n' && c2 == '\n')
      break;
  }
  printf("hd=%d\n", hd);
    
  fclose(ofp);
  fclose(dfp);
  return(0);
}

int main(){
  eval();
  return(0);
}
