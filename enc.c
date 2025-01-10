#include <stdio.h>
#include <stdlib.h>
#include "grpwk24.h"

#define BITS_per_block 50 // 25文字ブロックあたりのビット数(2bit * 25)
// #define NUM_BLOCKS 5000  // 25文字ブロックの数(2*10^5/50)
// #define CHUNK_SIZE 4 //処理するバイト数

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

  unsigned long long block_index = 0; // 25文字ブロックのインデックス
  unsigned char c1, c2, c3, c4, res;
  unsigned char bit_array[BITS_per_block] = {0}; //50bit分格納のための配列, 0で初期化

  for(int i=0; i<ORGDATA_LEN; i+=4){ // ORGDATA_LEN = 2 * 10 ^ 5で、頭から4bit取り出す
    c1 = getc(ofp);
    c2 = getc(ofp);
    c3 = getc(ofp);
    c4 = getc(ofp);

    // c1,c2,c3,c4の最下位ビットを配列に格納（最下位ビット以外は全て0）
    bit_array[0] = c1 & 0x1;
    bit_array[1] = c2 & 0x1;
    bit_array[2] = c3 & 0x1;
    bit_array[3] = c4 & 0x1;

    // 残りの46(50-4)ビット分の配列にインデックスを格納（インデックスの上位ビットから1ビットずつ）
    for (int j = 4; j < BITS_per_block; j++) {
        // 上位ビットから取得
        bit_array[j] = (block_index >> (BITS_per_block - 1 - j)) & 0x1;  
    }

    //2bitずつ、DNA塩基に変化して出力
    for (int j = 0; j < BITS_per_block; j += 2) {
      //2bitの値を計算
      unsigned char two_bits = (bit_array[j] << 1) | bit_array[j+1];
      switch(two_bits){
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
  
    block_index ++;

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
