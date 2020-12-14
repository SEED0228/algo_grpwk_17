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

  unsigned char c1, res1, res2;
  int count = 0;
  for(int i=0; i<ORGDATA_LEN; i++){
    if (count == 16){
      fputc(BASE_A, efp);
      fputc(BASE_A, efp);
      fputc(BASE_A, efp);
      fputc(BASE_A, efp);
      fputc(BASE_A, efp);
      count = 0;
    }
    
    c1 = getc(ofp);
    //c2 = getc(ofp);
    
    switch( c1 & 0x1 ){
    case 0:
      res1 = BASE_G;
      res2 = BASE_C;
      break;
    case 1:
      res1 = BASE_T; 
      res2 = BASE_C;
      break;
    /*
    case 2:
      res = BASE_G;      
      break;
    case 3:
      res = BASE_T;      
      break;
      */
    }
    fputc(res1, efp);
    fputc(res2, efp);
    count++;
    
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
