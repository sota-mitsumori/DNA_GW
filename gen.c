#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/random.h>
#include <sys/syscall.h>

#include "grpwk24.h"

unsigned char buf[ORGDATA_LEN];

int gen(){
  FILE *fp;
  if((fp = fopen(ORGDATA, "w")) ==NULL){
    fprintf(stderr, "cannot open %s\n", ORGDATA);
    exit(1);
  }

  size_t buf_size = sizeof(buf);
  if(-1 == (int)syscall(SYS_getrandom, buf, buf_size, GRND_NONBLOCK)) {
    perror ("getrandom");
    exit(1);
  }  

  char c;
  for(int i=0; i<ORGDATA_LEN; i++){
    switch(buf[i]&0x1){
    case 0:
      c = '0';
      break;
    case 1:
      c = '1';
      break;
    }
    fputc(c, fp);
  }
  c = '\n';
  fputc(c, fp);
  
  fclose(fp);    
  return(0);
}

int main(){
  gen();
  return(0);
}
