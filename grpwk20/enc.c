#include <stdio.h>
#include <stdlib.h>
#include "grpwk20.h"

unsigned char rst = '0';

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

  unsigned char c1, c2, res1, res2, res3, check_marker = rst, marker = rst;
  int count = 0;
  for(int i=0; i<ORGDATA_LEN; i+=2){
    c1 = getc(ofp);
    c2 = getc(ofp);
    count++;
    
    switch( ( (c1 & 0x1) << 7) >> 6 | ( c2 & 0x1) ){
    case 0:
      res1 = BASE_C;
      res2 = BASE_A;
      res3 = BASE_G;
      break;
    case 1:
      res1 = BASE_G;
      res2 = BASE_C;
      res3 = BASE_T;      
      break;
    case 2:
      res1 = BASE_T;
      res2 = BASE_G;
      res3 = BASE_A;      
      break;
    case 3:
      res1 = BASE_A;
      res2 = BASE_T;
      res3 = BASE_C;      
      break;
    }
    if (count != 2){
      fputc(res1, efp);
      fputc(res2, efp);
      fputc(res3, efp);
      
      if ((res3 == marker)||(res1 == check_marker)){
        fputc(res1, efp);
        fputc(res2, efp);
        fputc(res3, efp);
      }else{
        fputc(res2, efp);
      }
      
      fputc(res1, efp);
      fputc(res2, efp);
      fputc(res3, efp);
    }else{
      for (int i = 0; i < 10;i++){
        fputc(res2, efp);
      }
      check_marker = marker;
      marker = res2;
      count = 0;
    }
  }
  res1 = '\n';
  fputc(res1, efp);
  
  
  fclose(ofp);
  fclose(efp);
  return(0);
}

int main(){
  enc();
  return(0);
}
