# algo_grpwk_17
## Diff2times 説明
~~適当に説明しているので，一部細かいところは省略した~~  
**以下示したコードは説明のためのもので，実際とは異なる**  
**変な日本語ですみません，何かわからない時は私に聞いてください**  
Diff3+Marker と同じ符号化↓


| orgdata |->| encdata |
|:---:|:---:|:---:| 
| 00 |A| CAG |
| 01 |C| GCT |
| 10 |G| TGA |
| 11 |T| ATC |

2文字(orgdata 4ビット)を1セットとします  
前1文字は表のように符号化し，2回書きます  
後1文字はそのまま10回書きます  
###### 例  
| orgdata | encdata |
|:---:|:---:| 
| 0010 | CAGCAGGGGGGGGGGG |
| 0111 | GCTGCTTTTTTTTTTT |
| 1010 | TGATGAGGGGGGGGGG |
| 1100 | ATCATCAAAAAAAAAA | 
## 判定法
- 下のコードはイメージで，実際のコードではありません  
  
まず，seqdataから5連続で同じ文字だったら marker と判断し，その1セットをバッファ buff に格納します  
 ```C
 unsigned char* buff;
 int number;
 if ((buff[number] == buff[number-1])&&(buff[number] == buff[number-2])&&(buff[number] == buff[number-3])&&(buff[number] == buff[number-4])){
 ...
 }
 ```
 次に， marker を`table[tail+3]`（tail はセットの末尾)に書き込みます
 ```C
init(table);//table の初期化
tail = number - 5;//最後の1桁を示すよう修正
//marker を消す
for (int i = 0; i < 5; i++){
  buff[number - i] = rst;
}
table[tail+3] = marker;
 ```
 これで marker なしの1セットが完成した  
 次のステップで，このセット内の文字が何か調べる. A,C,T,G それぞれが見つかったら `has_A`,`has_C`,`has_G`,`has_T` を `1` にセットします
 ```C
 for (int i = 0; i < tail+1; i++){
  if (buff[i] == BASE_A){
    has_A = 1;
    continue;
  }
  if (buff[i] == BASE_C){
    has_C = 1;
    continue;
  }
  if (buff[i] == BASE_G){
    has_G = 1;
    continue;
  }
  if (buff[i] == BASE_T){
    has_T = 1;
    continue;
  }
}
 ```
 もし `has_A`,`has_C`,`has_G`,`has_T` の和は `3` であればもう判定できるので，`table[0]`に書き込み，このセットの処理が完了した．
 ```C
 if ((has_A + has_C + has_G + has_T)==3){
  if (has_A == 0){
  //A がないのは GCT, つまり C だとわかる， 下も同じ
    table[0] = BASE_C;
    write(tail, table, dfp);
    init(buff);
    number = 0;
    continue;
  }
  if (has_C == 0){
    table[0] = BASE_G;
    write(tail, table, dfp);
    init(buff);m
    number = 0;
    continue;
  }
  if (has_G == 0){
    table[0] = BASE_T;
    write(tail, table, dfp);
    init(buff);
    number = 0;
    continue;
  }
  if (has_T == 0){
    table[0] = BASE_A;
    write(tail, table, dfp);
    init(buff);
    number = 0;
    continue;
  }
}
 ```
 ここまで処理が終わらなければ， `has_A`,`has_C`,`has_G`,`has_T` の和は `3` 以下である場合だからまず `1` になるのがほぼないので `2` の時を考える．  
 この時まず考えるのは一つ前の marker と今の1番目の文字が同じの場合，この文字が抜けたので，marker を入れてチェックします
 ```C
 //一つ前の marker は check_marker
 switch (check_marker){
  case BASE_A:
    if ((buff[0]==BASE_T)&&(buff[1]==BASE_C)){
      table[0] = BASE_T;
      count++;

    }
  break;
  case BASE_C:
    if ((buff[0]==BASE_A)&&(buff[1]==BASE_G)){
      table[0] = BASE_A;
      count++;

    }
  break;
  case BASE_G:
    if ((buff[0]==BASE_C)&&(buff[1]==BASE_T)){
      table[0] = BASE_C;
      count++;

    }
  break;
  case BASE_T:
    if ((buff[0]==BASE_G)&&(buff[1]==BASE_A)){
      table[0] = BASE_G;
      count++;
    }
  break;
}
 ```
 同じように最後の文字と現在のセットの marker が同じの場合を考え，上と同じ処理をする．この処理のイメージは省略します．  
 ここでまだ終わらなければ，このセットが少なくとも同じ2文字が欠損したと考えられるので，前2文字，後2文字，`check_marker` と前1文字,後1文字と `marker` の4パターンでそれぞれチェックします．  
 ~~この処理のコードがめちゃめちゃ長いので `check_marker` と1文字目のチェックだけを下に示しますね，ほかは省略~~
 ```C
 if (((check_marker==BASE_A)&&(buff[0]==BASE_T))||((check_marker==BASE_A)&&(buff[0]==BASE_C))||((check_marker==BASE_T)&&(buff[0]==BASE_C))){
  table[0] = BASE_T;
  count++;
}
if (((check_marker==BASE_C)&&(buff[0]==BASE_A))||((check_marker==BASE_C)&&(buff[0]==BASE_G))||((check_marker==BASE_A)&&(buff[0]==BASE_G))){
  table[0] = BASE_A;
  count++;
}
if (((check_marker==BASE_G)&&(buff[0]==BASE_C))||((check_marker==BASE_C)&&(buff[0]==BASE_T))||((check_marker==BASE_G)&&(buff[0]==BASE_T))){
  table[0] = BASE_C;
  count++;
}
if (((check_marker==BASE_T)&&(buff[0]==BASE_G))||((check_marker==BASE_G)&&(buff[0]==BASE_A))||((check_marker==BASE_T)&&(buff[0]==BASE_A))){
  table[0] = BASE_G;
  count++;
}
//見つかったら書き込んで終了
if (count >= 1){
  write(tail, table, dfp);
  init(buff);
  number = 0;
  continue;
}
 ```
 ここに来てまだ終わらないということは，確率的にほぼないけど6文字中3,4文字が欠損してしまったということなので，もう判別できないから，読み込んだものをそのまま書き込みます．
 ```C
 switch (buff[0]){
    case BASE_A:
      table[0] = BASE_A;
      count++;
    break;
    case BASE_C:
      table[0] = BASE_C;
      count++;
    break;
    case BASE_G:
      table[0] = BASE_G;
      count++;
    break;
    case BASE_T:
      table[0] = BASE_T;
      count++;
    break;
  }
  write(tail, table, dfp);
  init(buff);
  number = 0;
  continue;
}
 ```
 これで判定処理が終わった．  
 ## 符号化の追加処理
 前述の marker と1番目，最後1文字と同じな場合の判定精度をあげるため，この時にだけ符号化を2回ではなく3回書きます  
 ```C
 //res1, res2, res3 は符号化後の3文字に対応する， CAG の場合 res1 -> C, res2 -> A, res3 -> G
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
 ```
