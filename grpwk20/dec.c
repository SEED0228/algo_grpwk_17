#include <stdio.h>
#include <stdlib.h>
#include "grpwk20.h"

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

  FILE *dfp_buff1;
  if((dfp_buff1 = fopen("dfpbuff1", "w+")) ==NULL){
    fprintf(stderr, "cannot open %s\n", "dfpbuff1");
    exit(1);
  }

  FILE *dfp_buff2;
  if((dfp_buff2 = fopen("dfpbuff2", "w+")) ==NULL){
    fprintf(stderr, "cannot open %s\n", "dfpbuff2");
    exit(1);
  }

  FILE *dfp_buff3;
  if((dfp_buff3 = fopen("dfpbuff3", "w+")) ==NULL){
    fprintf(stderr, "cannot open %s\n", "dfpbuff3");
    exit(1);
  }

  FILE *dfp_buff4;
  if((dfp_buff4 = fopen("dfpbuff4", "w+")) ==NULL){
    fprintf(stderr, "cannot open %s\n", "dfpbuff4");
    exit(1);
  }

  FILE *dfp_buff5;
  if((dfp_buff5 = fopen("dfpbuff5", "w+")) ==NULL){
    fprintf(stderr, "cannot open %s\n", "dfpbuff5");
    exit(1);
  }

  unsigned char c, res, c_before;
  int count = 0, set = 0, beforeIsG_Or_T;
  int number;
  
  for (number = 0; number < 5;number++){
  count = 0;
  set = 0;
  while((c = getc(sfp)) != '\n'){
    if (count == 4){set = 1;}
    if (set){
      if (c == BASE_A){
        c_before = c;
        continue;
      }else if (c_before == BASE_A){
        set = 0;
        count = 0;
      }else {
        c_before = c;
        continue;
      }
    }else{
      if (c == BASE_A){
        for (int i = 0; i < 4 - count;i++){
          if (count % 2 == 0){ res = 1;}else{res = 0;}
          switch (number)
          {
          case 0:
            fputc((res&0x1)+'0', dfp_buff1);
            break;
          case 1:
            fputc((res&0x1)+'0', dfp_buff2);
            break;
          case 2:
            fputc((res&0x1)+'0', dfp_buff3);
            break;
          case 3:
            fputc((res&0x1)+'0', dfp_buff4);
            break;
          case 4:
            fputc((res&0x1)+'0', dfp_buff5);
            break;
          default:
            break;
          }
          
        }
        set = 1;
        c_before = c;
        continue;
      }
    }
    switch(c){
    /*
    case BASE_A:
      res = 0;
      break;
    case BASE_C:
      res = 1;      
      break;
      */
    case BASE_G:
      res = 0;
      beforeIsG_Or_T = 1;
      count++;
      break;
    case BASE_T:
      res = 1;  
      beforeIsG_Or_T = 1;
      count++;    
      break;
    default:
      break;
    }
    if ((c == BASE_C) && (beforeIsG_Or_T)){ 
      beforeIsG_Or_T = 0;
      c_before = c; 
      continue;
      }
    else if (c == BASE_C){
        if (count % 2 == 0){
          res = 0;
          count++;
        }else{
          c_before = c;
          continue;
        }
    }
    switch (number)
          {
          case 0:
            fputc((res&0x1)+'0', dfp_buff1);
            break;
          case 1:
            fputc((res&0x1)+'0', dfp_buff2);
            break;
          case 2:
            fputc((res&0x1)+'0', dfp_buff3);
            break;
          case 3:
            fputc((res&0x1)+'0', dfp_buff4);
            break;
          case 4:
            fputc((res&0x1)+'0', dfp_buff5);
            break;
          default:
            break;
          }

    c_before = c;
  }  
  res = '\n';
  switch (number)
          {
          case 0:
            fputc(res, dfp_buff1);
            break;
          case 1:
            fputc(res, dfp_buff2);
            break;
          case 2:
            fputc(res, dfp_buff3);
            break;
          case 3:
            fputc(res, dfp_buff4);
            break;
          case 4:
            fputc(res, dfp_buff5);
            break;
          default:
            break;
          }
  }

  int judge;
  unsigned char compare[5], diff;
  fseek(dfp_buff1,0L,SEEK_SET);
  fseek(dfp_buff2,0L,SEEK_SET);
  fseek(dfp_buff3,0L,SEEK_SET);
  fseek(dfp_buff4,0L,SEEK_SET);
  fseek(dfp_buff5,0L,SEEK_SET);
  while ((compare[0] = getc(dfp_buff1)) != '\n'){
    compare[1] = getc(dfp_buff2);
    compare[2] = getc(dfp_buff3);
    compare[3] = getc(dfp_buff4);
    compare[4] = getc(dfp_buff5);
    judge = 0;
    for (int i = 1; i < 5;i++){
      if (compare[i] == compare[0]){
        judge++;
        }else{
          diff = compare[i];
        }
    }
    if (judge > 1){
      fputc(compare[0],dfp);
    }else{
      fputc(diff,dfp);
      }
  }
  res = '\n';
  fputc(res,dfp);

  fclose(sfp);
  fclose(dfp);
  fclose(dfp_buff1);
  fclose(dfp_buff2);
  fclose(dfp_buff3);
  fclose(dfp_buff4);
  fclose(dfp_buff5);
  return(0);
}

int main(){
  dec();
  return(0);
}
