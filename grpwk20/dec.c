#include <stdio.h>
#include <stdlib.h>
#include "grpwk20.h"

unsigned char rst = '0';

void init(unsigned char* buff){
  for (int i = 0;i < 50;i++){
    buff[i] = rst;
  }
}

void write(int tail,unsigned char* table, FILE* dfp){
  int j = 0, write_count = 0;
  //unsigned char res;
  while (write_count < 3){
    if (table[j] == rst){
      j++;
      continue;
    }else{
      switch (table[j]){
        case BASE_A:
          //res = 0;
          fputc('0', dfp);
          fputc('0', dfp);
          write_count++;
        break;
        case BASE_C:
          //res = 1;
          fputc('0', dfp);
          fputc('1', dfp);
          write_count++;
        break;
        case BASE_G:
          //res = 2;
          fputc('1', dfp);
          fputc('0', dfp);
          write_count++;
        break;
        case BASE_T:
          //res = 3;
          fputc('1', dfp);
          fputc('1', dfp);
          write_count++;
        break;
      }
      j++;
    }
  }
  switch (table[tail+3]){
    case BASE_A:
      fputc('0', dfp);
      fputc('0', dfp);
    break;
    case BASE_C:
      fputc('0', dfp);
      fputc('1', dfp);
    break;
    case BASE_G:
      fputc('1', dfp);
      fputc('0', dfp);
    break;
    case BASE_T:
      fputc('1', dfp);
      fputc('1', dfp);
    break;
  }
}

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


  unsigned char* buff;
  unsigned char* table;
  buff = (unsigned char*)malloc(sizeof(unsigned char)*50);
  table = (unsigned char*)malloc(sizeof(unsigned char)*50);
  unsigned char c = rst, res, check_marker = rst, marker = rst,checked = '-', dismiss;
  int count = 0, lap = 0, tail = 0, number = 0, set = 0;
  
  init(buff);
  init(table);

  while ((c = getc(sfp)) != '\n'){
    lap++;
    if (set){
      if (c == dismiss){
        continue;
      }else{
        set = 0;
        dismiss = rst;
        number = 0;
      }
    }


    buff[number] = c;
    if (number > 4){
      if ((buff[number] == buff[number-1])&&(buff[number] == buff[number-2])&&(buff[number] == buff[number-3])&&(buff[number] == buff[number-4])){
        set = 1;
        check_marker = marker;
        marker = buff[number];
        dismiss = buff[number];
        //read process start here
        count = 0;
        init(table);
        tail = number - 5;
        for (int i = 0;i < 5;i++){
          buff[number-i] = rst;
        }
        table[tail+3] = marker;
        for (int i = 0; i < tail-1; i++){
          if (buff[i]==checked) continue;
          if ((buff[i]==BASE_A)&&(buff[i+1]==BASE_T)&&(buff[i+2]==BASE_C)){
            table[i] = BASE_T;
            count++;
            buff[i] = checked; buff[i+1] = checked; buff[i+2] = checked;
          }
          if ((buff[i]==BASE_C)&&(buff[i+1]==BASE_A)&&(buff[i+2]==BASE_G)){
            table[i] = BASE_A;
            count++;
            buff[i] = checked; buff[i+1] = checked; buff[i+2] = checked;
          }
          if ((buff[i]==BASE_G)&&(buff[i+1]==BASE_C)&&(buff[i+2]==BASE_T)){
            table[i] = BASE_C;
            count++;
            buff[i] = checked; buff[i+1] = checked; buff[i+2] = checked;
          }
          if ((buff[i]==BASE_T)&&(buff[i+1]==BASE_G)&&(buff[i+2]==BASE_A)){
            table[i] = BASE_G;
            count++;
            buff[i] = checked; buff[i+1] = checked; buff[i+2] = checked;
          }
        }
        
        switch (check_marker){
          case BASE_A:
            if ((buff[0]==BASE_T)&&(buff[1]==BASE_C)){
              table[0] = BASE_T;
              count++;
              buff[0] = checked; buff[1] = checked;
            }
          break;
          case BASE_C:
            if ((buff[0]==BASE_A)&&(buff[1]==BASE_G)){
              table[0] = BASE_A;
              count++;
              buff[0] = checked; buff[1] = checked;
            }
          break;
          case BASE_G:
            if ((buff[0]==BASE_C)&&(buff[1]==BASE_T)){
              table[0] = BASE_C;
              count++;
              buff[0] = checked; buff[1] = checked;
            }
          break;
          case BASE_T:
            if ((buff[0]==BASE_G)&&(buff[1]==BASE_A)){
              table[0] = BASE_G;
              count++;
              buff[0] = checked; buff[1] = checked;
            }
          break;
        }
        switch (marker){
          case BASE_A:
            if ((buff[tail]==BASE_G)&&(buff[tail-1]==BASE_T)){
              table[tail] = BASE_G;
              count++;
              buff[tail] = checked; buff[tail-1] = checked;
            }
          break;
          case BASE_C:
            if ((buff[tail]==BASE_T)&&(buff[tail-1]==BASE_A)){
              table[tail] = BASE_T;
              count++;
              buff[tail] = checked; buff[tail-1] = checked;
            }
          break;
          case BASE_G:
            if ((buff[tail]==BASE_A)&&(buff[tail-1]==BASE_C)){
              table[tail] = BASE_A;
              count++;
              buff[tail] = checked; buff[tail-1] = checked;
            }
          break;
          case BASE_T:
            if ((buff[tail]==BASE_C)&&(buff[tail-1]==BASE_T)){
              table[tail] = BASE_C;
              count++;
              buff[tail] = checked; buff[tail-1] = checked;
            }
          break;
        }
        
        if (count >= 3){
          //write process here
          write(tail, table, dfp);
          init(buff);
          number = 0;
          continue;
        }
        if ((buff[0]==checked)&&(buff[tail]==checked)){
          for (int i = 2; i < tail - 2;i++){
            if (buff[i]==checked) continue;
            if (((buff[i]==BASE_A)&&(buff[i+1]==BASE_T))||((buff[i]==BASE_A)&&(buff[i+1]==BASE_C))||((buff[i]==BASE_T)&&(buff[i+1]==BASE_C))){
              table[i] = BASE_T;
              count++;
              buff[i] = checked; buff[i+1] = checked;
              break;
            }
            if (((buff[i]==BASE_C)&&(buff[i+1]==BASE_A))||((buff[i]==BASE_C)&&(buff[i+1]==BASE_G))||((buff[i]==BASE_A)&&(buff[i+1]==BASE_G))){
              table[i] = BASE_A;
              count++;
              buff[i] = checked; buff[i+1] = checked;
              break;
            }
            if (((buff[i]==BASE_G)&&(buff[i+1]==BASE_C))||((buff[i]==BASE_C)&&(buff[i+1]==BASE_T))||((buff[i]==BASE_G)&&(buff[i+1]==BASE_T))){
              table[i] = BASE_C;
              count++;
              buff[i] = checked; buff[i+1] = checked;
              break;
            }
            if (((buff[i]==BASE_T)&&(buff[i+1]==BASE_G))||((buff[i]==BASE_G)&&(buff[i+1]==BASE_A))||((buff[i]==BASE_T)&&(buff[i+1]==BASE_A))){
              table[i] = BASE_G;
              count++;
              buff[i] = checked; buff[i+1] = checked;
              break;
            }
          }
          if (count >= 3){
              //write process here
              write(tail, table, dfp);
              init(buff);
              number = 0;
              continue;
            }
        }
        if ((buff[0]==checked)&&(buff[tail]!=checked)){
          for (int i = 1; i < tail;i++){
            if (buff[i]==checked) continue;
            if (((buff[i]==BASE_A)&&(buff[i+1]==BASE_T))||((buff[i]==BASE_A)&&(buff[i+1]==BASE_C))||((buff[i]==BASE_T)&&(buff[i+1]==BASE_C))){
              table[i] = BASE_T;
              count++;
              buff[i] = checked; buff[i+1] = checked;
            }
            if (((buff[i]==BASE_C)&&(buff[i+1]==BASE_A))||((buff[i]==BASE_C)&&(buff[i+1]==BASE_G))||((buff[i]==BASE_A)&&(buff[i+1]==BASE_G))){
              table[i] = BASE_A;
              count++;
              buff[i] = checked; buff[i+1] = checked;
            }
            if (((buff[i]==BASE_G)&&(buff[i+1]==BASE_C))||((buff[i]==BASE_C)&&(buff[i+1]==BASE_T))||((buff[i]==BASE_G)&&(buff[i+1]==BASE_T))){
              table[i] = BASE_C;
              count++;
              buff[i] = checked; buff[i+1] = checked;
            }
            if (((buff[i]==BASE_T)&&(buff[i+1]==BASE_G))||((buff[i]==BASE_G)&&(buff[i+1]==BASE_A))||((buff[i]==BASE_T)&&(buff[i+1]==BASE_A))){
              table[i] = BASE_G;
              count++;
              buff[i] = checked; buff[i+1] = checked;
            }
          }
          if (count >= 3){
            //write
            write(tail, table, dfp);
            init(buff);
            number = 0;
            continue;
          }
          if (((buff[tail]==BASE_A)&&(marker==BASE_T))||((buff[tail]==BASE_A)&&(marker==BASE_C))||((buff[tail]==BASE_T)&&(marker==BASE_C))){
            table[tail] = BASE_T;
            count++;
            buff[tail] = checked;
          }
          if (((buff[tail]==BASE_C)&&(marker==BASE_A))||((buff[tail]==BASE_C)&&(marker==BASE_G))||((buff[tail]==BASE_A)&&(marker==BASE_G))){
            table[tail] = BASE_A;
            count++;
            buff[tail] = checked;
          }
          if (((buff[tail]==BASE_G)&&(marker==BASE_C))||((buff[tail]==BASE_C)&&(marker==BASE_T))||((buff[tail]==BASE_G)&&(marker==BASE_T))){
            table[tail] = BASE_C;
            count++;
            buff[tail] = checked;
          }
          if (((buff[tail]==BASE_T)&&(marker==BASE_G))||((buff[tail]==BASE_G)&&(marker==BASE_A))||((buff[tail]==BASE_T)&&(marker==BASE_A))){
            table[tail] = BASE_G;
            count++;
            buff[tail] = checked;
          }
          if (count >= 3){
            //write
            write(tail, table, dfp);
            init(buff);
            number = 0;
            continue;
          }
        }
        if ((buff[0]!=checked)&&(buff[tail]==checked)){
          for (int i = 0; i < tail - 2;i++){
            if (buff[i]==checked) continue;
            if (((buff[i]==BASE_A)&&(buff[i+1]==BASE_T))||((buff[i]==BASE_A)&&(buff[i+1]==BASE_C))||((buff[i]==BASE_T)&&(buff[i+1]==BASE_C))){
              table[i] = BASE_T;
              count++;
              buff[i] = checked; buff[i+1] = checked;
            }
            if (((buff[i]==BASE_C)&&(buff[i+1]==BASE_A))||((buff[i]==BASE_C)&&(buff[i+1]==BASE_G))||((buff[i]==BASE_A)&&(buff[i+1]==BASE_G))){
              table[i] = BASE_A;
              count++;
              buff[i] = checked; buff[i+1] = checked;
            }
            if (((buff[i]==BASE_G)&&(buff[i+1]==BASE_C))||((buff[i]==BASE_C)&&(buff[i+1]==BASE_T))||((buff[i]==BASE_G)&&(buff[i+1]==BASE_T))){
              table[i] = BASE_C;
              count++;
              buff[i] = checked; buff[i+1] = checked;
            }
            if (((buff[i]==BASE_T)&&(buff[i+1]==BASE_G))||((buff[i]==BASE_G)&&(buff[i+1]==BASE_A))||((buff[i]==BASE_T)&&(buff[i+1]==BASE_A))){
              table[i] = BASE_G;
              count++;
              buff[i] = checked; buff[i+1] = checked;
            }
          }
          if (count >= 3){
            //write
            write(tail, table, dfp);
            init(buff);
            number = 0;
            continue;
          }
          if (((check_marker==BASE_A)&&(buff[0]==BASE_T))||((check_marker==BASE_A)&&(buff[0]==BASE_C))||((check_marker==BASE_T)&&(buff[0]==BASE_C))){
            table[0] = BASE_T;
            count++;
            buff[0] = checked;
          }
          if (((check_marker==BASE_C)&&(buff[0]==BASE_A))||((check_marker==BASE_C)&&(buff[0]==BASE_G))||((check_marker==BASE_A)&&(buff[0]==BASE_G))){
            table[0] = BASE_A;
            count++;
            buff[0] = checked;
          }
          if (((check_marker==BASE_G)&&(buff[0]==BASE_C))||((check_marker==BASE_C)&&(buff[0]==BASE_T))||((check_marker==BASE_G)&&(buff[0]==BASE_T))){
            table[0] = BASE_C;
            count++;
            buff[0] = checked;
          }
          if (((check_marker==BASE_T)&&(buff[0]==BASE_G))||((check_marker==BASE_G)&&(buff[0]==BASE_A))||((check_marker==BASE_T)&&(buff[0]==BASE_A))){
            table[0] = BASE_G;
            count++;
            buff[0] = checked;
          }
          if (count >= 3){
            //write
            write(tail, table, dfp);
            init(buff);
            number = 0;
            continue;
          }
        }
        if ((buff[0]!=checked)&&(buff[tail]!=checked)){
          for (int i = 0; i < tail;i++){
            if (buff[i]==checked) continue;
            if (((buff[i]==BASE_A)&&(buff[i+1]==BASE_T))||((buff[i]==BASE_A)&&(buff[i+1]==BASE_C))||((buff[i]==BASE_T)&&(buff[i+1]==BASE_C))){
              table[i] = BASE_T;
              count++;
              buff[i] = checked; buff[i+1] = checked;
            }
            if (((buff[i]==BASE_C)&&(buff[i+1]==BASE_A))||((buff[i]==BASE_C)&&(buff[i+1]==BASE_G))||((buff[i]==BASE_A)&&(buff[i+1]==BASE_G))){
              table[i] = BASE_A;
              count++;
              buff[i] = checked; buff[i+1] = checked;
            }
            if (((buff[i]==BASE_G)&&(buff[i+1]==BASE_C))||((buff[i]==BASE_C)&&(buff[i+1]==BASE_T))||((buff[i]==BASE_G)&&(buff[i+1]==BASE_T))){
              table[i] = BASE_C;
              count++;
              buff[i] = checked; buff[i+1] = checked;
            }
            if (((buff[i]==BASE_T)&&(buff[i+1]==BASE_G))||((buff[i]==BASE_G)&&(buff[i+1]==BASE_A))||((buff[i]==BASE_T)&&(buff[i+1]==BASE_A))){
              table[i] = BASE_G;
              count++;
              buff[i] = checked; buff[i+1] = checked;
            }
          }
          if (count >= 3){
            //write
            write(tail, table, dfp);
            init(buff);
            number = 0;
            continue;
          }
          if (((check_marker==BASE_A)&&(buff[0]==BASE_T))||((check_marker==BASE_A)&&(buff[0]==BASE_C))||((check_marker==BASE_T)&&(buff[0]==BASE_C))){
            table[0] = BASE_T;
            count++;
            buff[0] = checked;
          }
          if (((check_marker==BASE_C)&&(buff[0]==BASE_A))||((check_marker==BASE_C)&&(buff[0]==BASE_G))||((check_marker==BASE_A)&&(buff[0]==BASE_G))){
            table[0] = BASE_A;
            count++;
            buff[0] = checked;
          }
          if (((check_marker==BASE_G)&&(buff[0]==BASE_C))||((check_marker==BASE_C)&&(buff[0]==BASE_T))||((check_marker==BASE_G)&&(buff[0]==BASE_T))){
            table[0] = BASE_C;
            count++;
            buff[0] = checked;
          }
          if (((check_marker==BASE_T)&&(buff[0]==BASE_G))||((check_marker==BASE_G)&&(buff[0]==BASE_A))||((check_marker==BASE_T)&&(buff[0]==BASE_A))){
            table[0] = BASE_G;
            count++;
            buff[0] = checked;
          }
          if (((buff[tail]==BASE_A)&&(marker==BASE_T))||((buff[tail]==BASE_A)&&(marker==BASE_C))||((buff[tail]==BASE_T)&&(marker==BASE_C))){
            table[tail] = BASE_T;
            count++;
            buff[tail] = checked;
          }
          if (((buff[tail]==BASE_C)&&(marker==BASE_A))||((buff[tail]==BASE_C)&&(marker==BASE_G))||((buff[tail]==BASE_A)&&(marker==BASE_G))){
            table[tail] = BASE_A;
            count++;
            buff[tail] = checked;
          }
          if (((buff[tail]==BASE_G)&&(marker==BASE_C))||((buff[tail]==BASE_C)&&(marker==BASE_T))||((buff[tail]==BASE_G)&&(marker==BASE_T))){
            table[tail] = BASE_C;
            count++;
            buff[tail] = checked;
          }
          if (((buff[tail]==BASE_T)&&(marker==BASE_G))||((buff[tail]==BASE_G)&&(marker==BASE_A))||((buff[tail]==BASE_T)&&(marker==BASE_A))){
            table[tail] = BASE_G;
            count++;
            buff[tail] = checked;
          }
          if (count >= 3){
            //write
            write(tail, table, dfp);
            init(buff);
            number = 0;
            continue;
          }
        }
        for (int i = 0; i < tail; i++){
          if (count >= 3) break;
          switch (buff[i]){
            case BASE_A:
              table[i] = BASE_A;
                count++;
                buff[i] = checked;
            break;
            case BASE_C:
              table[i] = BASE_A;
                count++;
                buff[i] = checked;
            break;
            case BASE_G:
              table[i] = BASE_A;
                count++;
                buff[i] = checked;
            break;
            case BASE_T:
              table[i] = BASE_A;
                count++;
                buff[i] = checked;
            break;
          }
        }
        
        //write
        write(tail, table, dfp);
        init(buff);
        number = 0;
        continue;
        //read process finish here
        number = 0;
        continue;

      }
    }
    number++;

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
