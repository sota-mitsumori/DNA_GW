#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grpwk24.h"
#include <math.h>
#define list_size 16667
#define str_size 13

/*変更途中コード途中保管*/
struct Element{
    int key;
    char str[str_size];
};

int comparekey(const void *a, const void *b){
  const struct Element *elemA = (const struct Element *)a;
  const struct Element *elemB = (const struct Element *)b;
  return elemA->key - elemB->key; // 昇順ソート
};


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

  unsigned char buffer[25];    
  int index[16];
  int Hamming_1[7];
  int Hamming_2[7];
  int Hamming_3[7];
  char decode[list_size][12] = {0};
  struct Element decode_fail[list_size];
  memset(decode_fail, 0, sizeof(decode_fail));
  int fail_index = 0; // decode_failのインデックス


  unsigned char res;
  for (int i = 0; i < list_size; i++) {
    for (int j = 0; j < 25; j++) {
        unsigned char c = getc(sfp);
        if (c == EOF) {
            break;
        }
        buffer[j] = c;
    }

    for (int i=2;i<10;i++) {
      char x = buffer[i];
      switch(x){
        case BASE_A:
          index[2*(i-2)] = 0;
          index[2*(i-2)+1] = 0;
          break;
        case BASE_C:
          index[2*(i-2)] = 0;
          index[2*(i-2)+1] = 1;   
          break;
        case BASE_G:
          index[2*(i-2)] = 1;
          index[2*(i-2)+1] = 0;   
          break;
        case BASE_T:
          index[2*(i-2)] = 1;
          index[2*(i-2)+1] = 1;    
          break;
      }

    }
    int decimal_value = 0;

    // 二進数を10進数に変換
    for (int j = 0; j < 16; j++) {
      decimal_value = (decimal_value << 1) | index[j];
    }
    // for (int  j= 0; j < 16; j++) {
    //     decimal_value += index[j] * (int)pow(2, 15 - j);
    // }

    // mod 12 を計算
    int result_mod_12 = decimal_value / 12;
    int result_mod_12_ans = decimal_value % 12;


    //Humming_1
    char D1[2] = {buffer[10],buffer[11]};
    char P1[3] = {buffer[12],buffer[13],buffer[14]};

    for (int j=0;j<2;j++) {
      char x = D1[j];
      switch(x){
        case BASE_A:
          Hamming_1[2*j] = 0;
          Hamming_1[2*j+1] = 0;
          break;
        case BASE_C:
          Hamming_1[2*j] = 0;
          Hamming_1[2*j+1] = 1;   
          break;
        case BASE_G:
          Hamming_1[2*j] = 1;
          Hamming_1[2*j+1] = 0;   
          break;
        case BASE_T:
          Hamming_1[2*j] = 1;
          Hamming_1[2*j+1] = 1;    
          break;
      }
    }
    for (int j=0;j<3;j++) {
      char x1 = P1[j];
      switch(x1){
        case BASE_C:
          Hamming_1[j+4] = 1;
          break;
        case BASE_G:
          Hamming_1[j+4] = 0;   
          break;
      }
    }
    unsigned char errorPosition1 = (Hamming_1[6] << 2) | (Hamming_1[5] << 1) | Hamming_1[4];

    if (errorPosition1 != 0) {
        Hamming_1[errorPosition1 - 1] ^= 1;
    } else {
    }

     //Humming_2
    char D2[2] = {buffer[15],buffer[16]};
    char P2[3] = {buffer[17],buffer[18],buffer[19]};

    for (int j=0;j<2;j++) {
      char x = D2[j];
      switch(x){
        case BASE_A:
          Hamming_2[2*j] = 0;
          Hamming_2[2*j+1] = 0;
          break;
        case BASE_C:
          Hamming_2[2*j] = 0;
          Hamming_2[2*j+1] = 1;   
          break;
        case BASE_G:
          Hamming_2[2*j] = 1;
          Hamming_2[2*j+1] = 0;   
          break;
        case BASE_T:
          Hamming_2[2*j] = 1;
          Hamming_2[2*j+1] = 1;    
          break;
      }
    }
    for (int j=0;j<3;j++) {
      char x1 = P2[j];
      switch(x1){
        case BASE_C:
          Hamming_2[j+4] = 1;
          break;
        case BASE_G:
          Hamming_2[j+4] = 0;   
          break;
      }
    }
    unsigned char errorPosition2 = (Hamming_2[6] << 2) | (Hamming_2[5] << 1) | Hamming_2[4];

    if (errorPosition2 != 0) {
        Hamming_2[errorPosition2 - 1] ^= 1;
    } else {
    }

     //Humming_3
    char D3[2] = {buffer[20],buffer[21]};
    char P3[3] = {buffer[22],buffer[23],buffer[24]};

    for (int j=0;j<2;j++) {
      char x = D3[j];
      switch(x){
        case BASE_A:
          Hamming_3[2*j] = 0;
          Hamming_3[2*j+1] = 0;
          break;
        case BASE_C:
          Hamming_3[2*j] = 0;
          Hamming_3[2*j+1] = 1;   
          break;
        case BASE_G:
          Hamming_3[2*j] = 1;
          Hamming_3[2*j+1] = 0;   
          break;
        case BASE_T:
          Hamming_3[2*j] = 1;
          Hamming_3[2*j+1] = 1;    
          break;
      }
    }
    for (int j=0;j<3;j++) {
      char x1 = P3[j];
      switch(x1){
        case BASE_C:
          Hamming_3[j+4] = 1;
          break;
        case BASE_G:
          Hamming_3[j+4] = 0;   
          break;
      }
    }
    unsigned char errorPosition3 = (Hamming_3[6] << 2) | (Hamming_3[5] << 1) | Hamming_3[4];

    if (errorPosition3 != 0) {
        Hamming_3[errorPosition3 - 1] ^= 1;
    } else {
    }

    int decode_number[12] = {Hamming_1[0],Hamming_1[1],Hamming_1[2],Hamming_1[3],
                            Hamming_2[0],Hamming_2[1],Hamming_2[2],Hamming_2[3],
                            Hamming_3[0],Hamming_3[1],Hamming_3[2],Hamming_3[3]};
    char str[12];  // 終端文字 \0 を含めて13文字分確保
    
    // 配列の各要素を文字 '0' または '1' に変換して str に格納
    for (int j = 0; j < 12; j++) {
        str[j] = decode_number[j] + '0';  // int を '0' と '1' の文字に変換
    }
    str[12] = '\0';


    if (result_mod_12_ans == 0 && decode[result_mod_12][0] == '\0') {
      for (int j = 0; j < 12; j++) {
        decode[result_mod_12][j] = str[j];  
      }
    } else{
      decode_fail[fail_index].key = result_mod_12;
      strcpy(decode_fail[fail_index].str, str);
      fail_index++;
    }
  }
  fail_index = 0;
 
  for (int i = 0; i < list_size; i++) {
      if (decode[i][0] == '\0') { // decodeの空白を発見
        strcpy(decode[i], decode_fail[fail_index].str);
        fail_index++;
      }
  }
  

  for (int i = 0; i < list_size; i++) {
    for (int j = 0; j < str_size; j++){
      res = decode[i][j];
      //'0'または'1'のみを書き込む
      if (res != '0' && res != '1') {
          res = '0'; // デフォルト値を設定（必要に応じて変更）
      }
      fputc(res, dfp);
    }
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