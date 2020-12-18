#include <stdio.h>
#include <stdlib.h>
#include "grpwk20.h"

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

  unsigned char c1, c2, res1, res2, res3;
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
    if (count != 3){
      fputc(res1, efp);
      fputc(res2, efp);
      fputc(res3, efp);
    }else{
      int j = 0;
      //if (i%5==0){j = 1;}
      for (;j < 11;j++){
        fputc(res2, efp);
      }
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
