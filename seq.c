#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/random.h>
#include <sys/syscall.h>

#include "grpwk24.h"

#define BUF_LEN 100000
unsigned char buf[BUF_LEN];

int cost;

int getrandom(){
  size_t buf_size = sizeof(buf);
  if(-1 == (int)syscall(SYS_getrandom, buf, buf_size, GRND_NONBLOCK)) {
    perror ("getrandom");
    exit(1);
  }
  return(buf_size);
}

int np(int start, int len){
  FILE *dfp;
  if((dfp = fopen(SYNDNA, "r")) ==NULL){
    fprintf(stderr, "cannot open %s\n", SYNDNA);
    exit(1);
  }
  fseek(dfp, start, SEEK_SET);

  FILE *sfp;
  if((sfp = fopen(SEQDATA, "a")) ==NULL){
    fprintf(stderr, "cannot open %s\n", SEQDATA);
    exit(1);
  }

  if(len == 0)
    len = MAX_SYN_SIZE;

  int ins=0, del=0, rcnt = -1, cnt;
  unsigned char c1, c2 = BASE_N, r;
  for(cnt = 0; cnt<len && (c1 = getc(dfp)) != '\n'; cnt++){
    if(rcnt<1)
      rcnt = ((int)getrandom())-1;

    r = buf[rcnt--];
    if(c1 == c2){     
      if(r<8){
	putc(c1, sfp);
	putc(c1, sfp);
	ins++;
      }else if(r>=16){
	putc(c1, sfp);
      }else{
	del++;
      }
    }else{
      if(r==0){
	r = buf[rcnt--];
	if(r%2 == 0){
	  putc(c1, sfp);
	  putc(c1, sfp);	
	  ins++;
	}else{
	  del++;
	}
      }else{
	putc(c1, sfp);
      }
    }
    c2 = c1;
  }
  putc('\n', sfp);
  //  fprintf(stderr,"ins=%d, del=%d\n",ins, del);
  fclose(sfp);
  fclose(dfp);

  if(cnt%COST_UNIT==0){
    cost += (cnt/COST_UNIT)*NP_RUN_COST_PER_UNIT;
  }else{
    cost += (1 + cnt/COST_UNIT)*NP_RUN_COST_PER_UNIT;
  }  
  return(0);
}

int bs(int start, int ilen){
  FILE *dfp;
  if((dfp = fopen(SYNDNA, "r")) ==NULL){
    fprintf(stderr, "cannot open %s\n", SYNDNA);
    exit(1);
  }
    
  int tail, head, len, nor;
  fseek(dfp, 0L, SEEK_END);
  tail = ftell(dfp);
  fseek(dfp, start, SEEK_SET);
  head = ftell(dfp);
  len = tail-head-1;

  if(ilen < len && ilen != 0)
    len = ilen;

  nor = len/SR_SIZE;
  if(len%SR_SIZE!=0)
    nor++;
  len = nor*SR_SIZE;
  char *dat = (char*)malloc(sizeof(char)*(len));

  int i = 0, rcnt = -1, mut = 0;
  char c;
  unsigned int r;
  
  for(int cnt = 0; cnt<len && (c = getc(dfp)) != '\n'; cnt++){    
    if(rcnt < 1)
      rcnt = getrandom() - 2;
    
    r = buf[rcnt--];
    if(r%MUTATION_RATE == 0){
      r = buf[rcnt--];
      switch(r%4){
      case 0:
	c = BASE_A;
	break;	
      case 1:
	c = BASE_C;
	break;
      case 2:
	c = BASE_G;
	break;
      case 3:
	c = BASE_T;
	break;
      }
      mut++;
    }
    dat[i++]=c;
  }

  for(;i<len; i++){
    if(rcnt < 0)
      rcnt = getrandom() - 1;
    r = buf[rcnt--];
    switch(r%4){
    case 0:
      c = BASE_A;
      break;	
    case 1:
      c = BASE_C;
      break;
    case 2:
      c = BASE_G;
      break;
    case 3:
      c = BASE_T;
	break;
    }
    dat[i]=c;
  }
  //  fprintf(stderr, "%u\n", mut);
  
  unsigned int p1, p2;
  rcnt = -1;
  char tmp;
  for(i=0; i<SHUFFLE_RATIO*nor; i++){
    if(rcnt < 8)
      rcnt = getrandom() - 8;    
    r = buf[rcnt]<<24 | buf[rcnt+1]<<16 | buf[rcnt+2]<<8 | buf[rcnt+3];
    p1 = (r%nor)*SR_SIZE;
    rcnt -= 4;
    r = buf[rcnt]<<24 | buf[rcnt+1]<<16 | buf[rcnt+2]<<8 | buf[rcnt+3];
    p2 = (r%nor)*SR_SIZE;    
    rcnt -= 4;
    //    fprintf(stderr, "%u, %u\n", p1, p2);
    
    for(int j=0 ; j<SR_SIZE; j++){
      tmp = dat[p1];
      dat[p1] = dat[p2];
      dat[p2] = tmp;
      p1++; p2++;
    }    
  }

  FILE *sfp;
  if((sfp = fopen(SEQDATA, "a")) ==NULL){
    fprintf(stderr, "cannot open %s\n", SEQDATA);
    exit(1);
  }
  for(i=0; i<len; i++){
    putc(dat[i], sfp);
  }
  putc('\n', sfp);
  
  free(dat);
  fclose(dfp);
  fclose(sfp);

  if(len%COST_UNIT==0){
    cost += (len/COST_UNIT)*BS_RUN_COST_PER_UNIT;
  }else{
    cost += (1 + len/COST_UNIT)*BS_RUN_COST_PER_UNIT;
  }
  return(0);  
}

int main(int argc, char **argv){
  if(argc !=7){
    fprintf(stderr, "read <round_bs> <start_bs> <len_bs> <round_np> <start_np> <len_np>");
    exit(1);
  }

  int nbs = atoi(argv[1]), nnp = atoi(argv[4]);
  int sbs = atoi(argv[2]), snp = atoi(argv[5]);
  int lbs = atoi(argv[3]), lnp = atoi(argv[6]);  

  FILE *sfp;
  if((sfp = fopen(SEQDATA, "w")) ==NULL){
    fprintf(stderr, "cannot open %s\n", SEQDATA);
    exit(1);
  }
  fclose(sfp);

  cost = 0;
  for(int i=0; i<nbs;i++){
    fprintf(stderr,"run bs (%d)\n", i);
    bs(sbs, lbs);
  }
  
  for(int i=0; i<nnp;i++){
    fprintf(stderr,"run np (%d)\n", i);
    np(snp, lnp);
  }
  fprintf(stderr,"cost=%d\n", cost);
  return(0);
}
