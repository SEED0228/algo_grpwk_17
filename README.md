# Group 17  
##### 目次
* [Diff2times の説明](#diff2times)  
* [Diff2+Marker11 等の説明あ](#diffmarker)
	* [Diff2+Marker11 の説明](#diffmarker)
	* [Diff{A}+Marker{B} シリーズ](#2.5)

<a id = "diff2times"></a>
## Diff2times の説明 
### <span id = "1">1. 符号化</span>  
まず，2進数と4塩基との対応関係は以下のようになる．  

| Binary Number | Base |
|:---:|:---:|
|00|A|
|01|C|
|10|G|
|11|T|

ここで注意すべきなのは，符号化は**4塩基**に対しての符号化であり，元の2進数に対しての符号化ではない．つまり，2進数のデータを4塩基からなるデータとみなして，その4塩基に対して符号化を行う．  
  
そこで，塩基の符号化は以下のようになる．  

|Base|Encoding|
|:---:|:---:|
|A|CAG|
|C|GCT|
|G|TGA|
|T|ATC|

### <span id = "2">2. enc.c について</span>
Dec での判定の都合上，塩基 **2** つで **1** セットとする．元のデータは 200000 ビットの数字だから 100000 個の塩基に変換することができ，50000 セットになる．ここで，1 セットの 2 つの塩基を X, Y ( X は Y の前にある)とする．  
X に対して，[1](#1) で説明した符号化を施し，符号化の結果を X の左右に書く．Y を Marker にするから，符号化せずにそのまま **8** 回書く．  

![](https://i.postimg.cc/sXtW1MrR/Xnip-2021-01-08-21-36-24.png)
###### 例

|Set|Encoding|
|:---:|:---:|
|AA|CAGACAGAAAAAAAA|
|CG|GCTCGCTGGGGGGGG|
|TC|ATCTATCCCCCCCCC|
|GT|TGAGTGATTTTTTTT|

これで enc の主な操作は終わった．ただこれだと一つ問題が生じてしまう．上の例の三つ目を見ると X は ATC ，Y が C で，ATC の C と Y の C が区別できなくなる．このままだとこの C が Marker とみなされてしまって，欠損となる．そこで，このような時にだけ符号化方法を変える．具体的に，元々は X の左右に符号化結果を追加するのだったが，この時，X は書かずに符号化結果を 3 回書く．  

![](https://i.postimg.cc/m2ScWXt4/Xnip-2021-01-08-21-37-46.png)
###### 例（修正）

|Set|Encoding|
|:---:|:---:|
|TC|ATCATCATCCCCCCCCC|
|CT|GCTGCTGCTTTTTTTTT|
|AG|CAGCAGCAGGGGGGGGG|

### <span id = "3">3. dec.c について</span>
dec では 2 つの配列 <span style="color:red">`table`</span> と <span style="color:red">`buff`</span> を持つ．この 2 つの配列の使い方は，現実での 2 つのテーブルと思ってください．<span style="color:red">`buff`</span> は作業台で，読み込んだ 1 セットをここに置いて，判定などの操作もここで行い，判定の結果はもう一つのテーブル <span style="color:red">`table`</span> に置く．<span style="color:red">`buff`</span> での処理が完了すると，<span style="color:red">`table`</span> 上のものを出力ファイルに書き込んでこのセットの処理が終わる．また，現実と同様に，1 セットの処理が完成すると，次の処理に備えるため <span style="color:red">`buff`</span> をクリアする．さらに，次のセットの処理を始める前に <span style="color:red">`table`</span> もクリアする．  

全体のワークフローは下のようになる．  

1. seqdata から次々と文字を読み込んで，作業台 <span style="color:red">`buff`</span> に格納する．  
2. 連続 4 文字が同じ文字なら Marker と判断し読み込みはいったん止めて判定処理に入る．この時作業台 <span style="color:red">`buff`</span> に 1 セットが格納されているはず．
3. <span style="color:red">`table`</span> をクリアする．
4. Marker は [2](#2) で話した Y だから <span style="color:red">`table`</span> に書き込む．
5. X を判定するので <span style="color:red">`buff`</span> の中の Marker 部分を消す．
6. 判定処理（[後述](#4)）を行い，判定した X を <span style="color:red">`table`</span> に書き込む．
7. X, Y が判定できたので <span style="color:red">`table`</span>  の内容（X と Y）を出力ファイル decdata に書き込む．
8. このセットの処理が終わったので <span style="color:red">`buff`</span> をクリアする．
9. 1 に戻って，seqdata がなくなるまで続く．

### <span id = "4">4. 判定方法</span>
###### ここから示すコードは説明のために簡略化したもので，実際のコードと異なることがある
<a id = "def"></a>
まず，以下のいくつかの変数と関数を定義する．  

```c
unsigned char rst = '0';//クリアする時に使うもの
unsigned char c;//seqdata から読み取った 1 文字
unsigned char check_marker;//一つ前のセットの Marker
unsigned char marker;//いま処理を行なっているセットの Marker
unsigned char dismiss;//Marker と同じ内容で，処理完了後次のセットを読み込む際前の Marker を読み込まれないように使う
int count;//カウント用
int tail;//buff[tail] はセットの末尾を示す
int number;//buff に読み込む際に使う
int set;//いま読み込み中なのか処理中なのか示すもの
int has_A, has_C, has_G, has_T;//A/C/G/T があるかどうか示すもの
void init(buff);//buff/table を初期化する，初期化とは全てに rst を代入する
void write(tail, table, dfp);//出力ファイルに書き込む関数， dfp は出力ファイル
```

ほとんどの処理はこのループ内で行う．以下，特に説明がない限り，このループ内のものを解説している．

```c
while((c = getc(sfp)) != '\n'){
	...
}
```

はじめに，`set` の値を判断し，次のセットの<ruby>頭部<rp>（</rp><rt>head</rt><rp>）</rp></ruby>（1番目の文字）を探す．

```c
if (set){
  if (c == dismiss){
    continue;
  }else{
    set = 0;
    dismiss = rst;
    number = 0;
  }
}
```

<ruby>頭部<rp>（</rp><rt>head</rt><rp>）</rp></ruby>が見つかったら，<ruby>作業台<rp>（</rp><rt>buff</rt><rp>）</rp></ruby>に格納する．

```c
buff[number] = c;
```

連続 4 文字が同じの場合，`set` を `1` にし，Marker を更新する．

```c
if (number > 4){
      if ((buff[number] == buff[number-1])&&(buff[number] == buff[number-2])&&(buff[number] == buff[number-3])/*&&(buff[number] == buff[number-4])*/){
        set = 1;
        check_marker = marker;
        marker = buff[number];
        dismiss = buff[number];
        //ここからは処理過程（後述）
        ...
	}
}    
```

判定処理を開始する前に，いくつか初期化操作を行う．

```c
count = 0;
has_A = 0, has_C = 0, has_G = 0, has_T = 0;
init(table);
tail = number - 5;//tail をセットの末尾に指すよう修正
//Marker を消す
for (int i = 0; i < 5; i++){
  buff[number - i] = rst;
}
```

Marker は Y なので <span style="color:red">`table`</span> に書き込む．

```c
table[tail+3] = marker;
```

<span style="color:red">`buff`</span> を走査し，内容を確認する．

```c
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

`has_A`, `has_C`, `has_G`, `has_T` の和で条件分岐する．和は `3` の時，もう判定できるので <span style="color:red">`table`</span> に書き込み処理を終わる．

```c
if ((has_A + has_C + has_G + has_T)==3){
  if (has_A == 0){
    table[0] = BASE_C;
    write(tail, table, dfp);
    init(buff);
    number = 0;
    continue;
  }
  if (has_C == 0){
    table[0] = BASE_G;
    write(tail, table, dfp);
    init(buff);
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

`has_A`, `has_C`, `has_G`, `has_T` の和は `2` の場合を考える．  
<ruby>頭部<rp>（</rp><rt>head</rt><rp>）</rp></ruby>が一つ前の Marker と同じの場合を仮定して `check_marker` を含んで完全一致比較する．発見したら書き込んで終了．

```c
if ((has_A + has_C + has_G + has_T)==3){
	...(略)
}else if ((has_A + has_C + has_G + has_T)==2){
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
if (count >= 1){
write(tail, table, dfp);
init(buff);
number = 0;
continue;
}
```

同じ操作を Marker に対しても行う．

```c
switch (marker){
    case BASE_A:
      if ((buff[tail]==BASE_G)&&(buff[tail-1]==BASE_T)){
        table[tail] = BASE_G;
        count++;
      }
    break;
    case BASE_C:
      if ((buff[tail]==BASE_T)&&(buff[tail-1]==BASE_A)){
        table[tail] = BASE_T;
        count++;
      }
    break;
    case BASE_G:
      if ((buff[tail]==BASE_A)&&(buff[tail-1]==BASE_C)){
        table[tail] = BASE_A;
        count++;
      }
    break;
    case BASE_T:
      if ((buff[tail]==BASE_C)&&(buff[tail-1]==BASE_G)){
        table[tail] = BASE_C;
        count++;
      }
    break;
  }
  if (count >= 1){
    write(tail, table, dfp);
    init(buff);
    number = 0;
    continue;
  }
```

ここまで来た場合，<ruby>頭部<rp>（</rp><rt>head</rt><rp>）</rp></ruby>に対し部分一致比較を行う．

```c
if (((buff[0]==BASE_A)&&(buff[1]==BASE_T))||((buff[0]==BASE_A)&&(buff[1]==BASE_C))||((buff[0]==BASE_T)&&(buff[1]==BASE_C))){
    table[0] = BASE_T;
    count++;
    
    
  }
  if (((buff[0]==BASE_C)&&(buff[1]==BASE_A))||((buff[0]==BASE_C)&&(buff[1]==BASE_G))||((buff[0]==BASE_A)&&(buff[1]==BASE_G))){
    table[0] = BASE_A;
    count++;
    
    
  }
  if (((buff[0]==BASE_G)&&(buff[1]==BASE_C))||((buff[0]==BASE_C)&&(buff[1]==BASE_T))||((buff[0]==BASE_G)&&(buff[1]==BASE_T))){
    table[0] = BASE_C;
    count++;
    
    
  }
  if (((buff[0]==BASE_T)&&(buff[1]==BASE_G))||((buff[0]==BASE_G)&&(buff[1]==BASE_A))||((buff[0]==BASE_T)&&(buff[1]==BASE_A))){
    table[0] = BASE_G;
    count++;
    
    
  }
  if (count >= 1){
  write(tail, table, dfp);
  init(buff);
  number = 0;
  continue;
  }
```

さらに，`check_marker` と `marker` を含んで部分一致比較する．

```c
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
  if (count >= 1){
    write(tail, table, dfp);
    init(buff);
    number = 0;
    continue;
  }
  if (((buff[tail]==BASE_A)&&(marker==BASE_T))||((buff[tail]==BASE_A)&&(marker==BASE_C))||((buff[tail]==BASE_T)&&(marker==BASE_C))){
    table[tail] = BASE_T;
    count++;
    
  }
  if (((buff[tail]==BASE_C)&&(marker==BASE_A))||((buff[tail]==BASE_C)&&(marker==BASE_G))||((buff[tail]==BASE_A)&&(marker==BASE_G))){
    table[tail] = BASE_A;
    count++;
    
  }
  if (((buff[tail]==BASE_G)&&(marker==BASE_C))||((buff[tail]==BASE_C)&&(marker==BASE_T))||((buff[tail]==BASE_G)&&(marker==BASE_T))){
    table[tail] = BASE_C;
    count++;
    
  }
  if (((buff[tail]==BASE_T)&&(marker==BASE_G))||((buff[tail]==BASE_G)&&(marker==BASE_A))||((buff[tail]==BASE_T)&&(marker==BASE_A))){
    table[tail] = BASE_G;
    count++;
    
  }
  if (count >= 1){
    write(tail, table, dfp);
    init(buff);
    number = 0;
    continue;
  }
```

これでも判定できなかった場合，読み取った値をそのまま書き込んで終了．

```c
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
```

`has_A`, `has_C`, `has_G`, `has_T` の和は `1` の場合を考える．  
和が `2` の時と同じく部分一致比較を行い，それで終わらなければ読み取った値をそのまま書き込んで終了．この部分は前述と同じなので省略する．  

これで処理過程が終わった．

<a id = "diffmarker"></a>
## Diff2+Marker11 等の説明 
### <span id = "2.1">1. 符号化</span>  
符号化は [Diff2times](#1) と同じなのでここで省略する．ただし，塩基 **3** つで 1 セットにするから，最後の 1 セットは塩基 **2** つしかない．このセットだけ，Marker を書かない（そもそも X, Y しかなく Z がない）.

### <span id = "2.2">2. enc.c について</span>
塩基 **3** つ（左から XYZ とする）で **1** セットとする．X，Y は直接[符号化した文字列](#1)を書き込む．Z は Marker なのでそのまま **11** 回書く．  

![](https://i.postimg.cc/pV9Th934/Xnip-2021-01-10-01-34-32.png)  
###### 例

|Set|Encoding|
|:---:|:---:|
|AAC|CAGCAGCCCCCCCCCCC|
|CGT|GCTTGATTTTTTTTTTT|
|GTC|TGAATCCCCCCCCCCCC|
|CAT|GCTCAGTTTTTTTTTTT|

### <span id = "2.3">3. dec.c について</span>
これもまた [Diff2times](#3) と同じ 2 つの配列 <span style="color:red">`table`</span> と <span style="color:red">`buff`</span> を持つ．  

全体のワークフローは以下のようになる．  

1. seqdata から次々と文字を読み込んで，作業台 <span style="color:red">`buff`</span> に格納する．  
2. 連続 **6** 文字が同じ文字なら Marker と判断し読み込みはいったん止めて判定処理に入る．この時作業台 <span style="color:red">`buff`</span> に 1 セットが格納されているはず．
3. <span style="color:red">`table`</span> をクリアする．
4. Marker を <span style="color:red">`table`</span> に書き込む．
5. X，Y を判定するので <span style="color:red">`buff`</span> の中の Marker 部分を消す．
6. [判定処理](#2.4)を行い，判定した X，Y を <span style="color:red">`table`</span> に書き込む．
7. X, Y, Z が判定できたので <span style="color:red">`table`</span>  の内容（X, Y, Z）を出力ファイル decdata に書き込む．
8. このセットの処理が終わったので <span style="color:red">`buff`</span> をクリアする．
9. 1 に戻って，seqdata がなくなるまで続く．

### <span id = "2.4">4. 判定方法</span>
###### ここから示すコードは説明のために簡略化したもので，実際のコードと異なることがある
以下のいくつかの変数と関数を定義する．なお，[Diff2times](#def) と同じ変数と関数の説明は省略する．  

```c
unsigned char checked = '-';//処理済みの文字に記号で示すためのもの，現実で消し線を引くと同じ感じ
unsigned char rst = '0';
unsigned char c;
unsigned char check_marker;
unsigned char marker;
unsigned char dismiss;
int count;
int tail;
int number;
int set;
void init(buff);//初期化
void write(tail, table, dfp);//書き込む
```

判定方法は [Diff2times](#4) と枠組みが同じなので，判定処理の部分だけ解説する．  

まず初期化操作を行い，Marker を <span style="color:red">`table`</span> に書き込む.

```c
count = 0;
init(table);
tail = number - 6;
for (int i = 0;i < 6;i++){
  buff[number-i] = rst;
}
table[tail+3] = marker;
```

<span style="color:red">`buff`</span> の内容を先頭から確認し，完全一致でマッチする．一つ見つかったらマッチした部分を `checked` に換え，この操作を停止する．

```c
for (int i = 0; i < tail-1; i++){
  if (buff[i]==checked) continue;
  if ((buff[i]==BASE_A)&&(buff[i+1]==BASE_T)&&(buff[i+2]==BASE_C)){
    table[i] = BASE_T;
    count++;
    buff[i] = checked; buff[i+1] = checked; buff[i+2] = checked;
    break;
  }
  if ((buff[i]==BASE_C)&&(buff[i+1]==BASE_A)&&(buff[i+2]==BASE_G)){
    table[i] = BASE_A;
    count++;
    buff[i] = checked; buff[i+1] = checked; buff[i+2] = checked;
    break;
  }
  if ((buff[i]==BASE_G)&&(buff[i+1]==BASE_C)&&(buff[i+2]==BASE_T)){
    table[i] = BASE_C;
    count++;
    buff[i] = checked; buff[i+1] = checked; buff[i+2] = checked;
    break;
  }
  if ((buff[i]==BASE_T)&&(buff[i+1]==BASE_G)&&(buff[i+2]==BASE_A)){
    table[i] = BASE_G;
    count++;
    buff[i] = checked; buff[i+1] = checked; buff[i+2] = checked;
    break;
  }
}
```

次に末尾から先頭へ内容を確認し，完全一致でマッチする．

```c
for (int i = 2; i < tail; i++){
  if (buff[tail-i]==checked) continue;
  if ((buff[tail-i]==BASE_A)&&(buff[tail-i+1]==BASE_T)&&(buff[tail-i+2]==BASE_C)){
    table[tail-i] = BASE_T;
    count++;
    buff[tail-i] = checked; buff[tail-i+1] = checked; buff[tail-i+2] = checked;
    break;
  }
  if ((buff[tail-i]==BASE_C)&&(buff[tail-i+1]==BASE_A)&&(buff[tail-i+2]==BASE_G)){
    table[tail-i] = BASE_A;
    count++;
    buff[tail-i] = checked; buff[tail-i+1] = checked; buff[tail-i+2] = checked;
    break;
  }
  if ((buff[tail-i]==BASE_G)&&(buff[tail-i+1]==BASE_C)&&(buff[tail-i+2]==BASE_T)){
    table[tail-i] = BASE_C;
    count++;
    buff[tail-i] = checked; buff[tail-i+1] = checked; buff[tail-i+2] = checked;
    break;
  }
  if ((buff[tail-i]==BASE_T)&&(buff[tail-i+1]==BASE_G)&&(buff[tail-i+2]==BASE_A)){
    table[tail-i] = BASE_G;
    count++;
    buff[tail-i] = checked; buff[tail-i+1] = checked; buff[tail-i+2] = checked;
    break;
  }
}
```

以上の 2 つの探索でもし両方とも成功したならば出力ファイルに書き込み処理を終了．

```c
if (count >= 2){
  write(tail, table, dfp);
  init(buff);
  number = 0;
  continue;
}
```

`check_marker` と先頭から2番目まで完全一致でマッチする．同じ操作を `marker` でも行う．

```c
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
    if ((buff[tail]==BASE_C)&&(buff[tail-1]==BASE_G)){
      table[tail] = BASE_C;
      count++;
      buff[tail] = checked; buff[tail-1] = checked;
    }
  break;
}
```

`count` の値，つまり今まで探索の成功回数で条件分岐する．  

`count` の値が `1` のとき，先頭から部分一致でマッチする． 

```c
if (count == 1){
  for (int i = 0; i < tail ;i++){
    if (count >= 2) break;
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
  ...
 }
```

`check_marker` と `marker` を含めて先頭と末尾をチェックする．  

```c
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
  if (count >= 2){
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
```

`count` が `0` のとき，先頭から部分一致でマッチし，さらに末尾からマッチする．どちらも 1 つ見つかったら停止する．

```c
if (count == 0){
  for (int i = 0; i < tail;i++){
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
  for (int i = 1; i < tail; i++){
    if (buff[tail-i]==checked) continue;
    if (((buff[tail-i]==BASE_A)&&(buff[tail-i+1]==BASE_T))||((buff[tail-i]==BASE_A)&&(buff[tail-i+1]==BASE_C))||((buff[tail-i]==BASE_T)&&(buff[tail-i+1]==BASE_C))){
      table[tail-i] = BASE_T;
      count++;
      buff[tail-i] = checked; buff[tail-i+1] = checked;
      break;
    }
    if (((buff[tail-i]==BASE_C)&&(buff[tail-i+1]==BASE_A))||((buff[tail-i]==BASE_C)&&(buff[tail-i+1]==BASE_G))||((buff[tail-i]==BASE_A)&&(buff[tail-i+1]==BASE_G))){
      table[tail-i] = BASE_A;
      count++;
      buff[tail-i] = checked; buff[tail-i+1] = checked;
      break;
    }
    if (((buff[tail-i]==BASE_G)&&(buff[tail-i+1]==BASE_C))||((buff[tail-i]==BASE_C)&&(buff[tail-i+1]==BASE_T))||((buff[tail-i]==BASE_G)&&(buff[tail-i+1]==BASE_T))){
      table[tail-i] = BASE_C;
      count++;
      buff[tail-i] = checked; buff[tail-i+1] = checked;
      break;
    }
    if (((buff[tail-i]==BASE_T)&&(buff[tail-i+1]==BASE_G))||((buff[tail-i]==BASE_G)&&(buff[tail-i+1]==BASE_A))||((buff[tail-i]==BASE_T)&&(buff[tail-i+1]==BASE_A))){
      table[tail-i] = BASE_G;
      count++;
      buff[tail-i] = checked; buff[tail-i+1] = checked;
      break;
    }
  }
  if (count >= 2){
    write(tail, table, dfp);
    init(buff);
    number = 0;
    continue;
  }
}
```

ここまで来た場合，先頭と末尾に読み込んだ内容をそのまま書き込んで終了．

```c
	int temp = count;
	for (int i = 0; i < tail; i++){
	  if (count != temp) break;
	  switch (buff[i]){
	    case BASE_A:
	      table[i] = BASE_A;
	        count++;
	        buff[i] = checked;
	    break;
	    case BASE_C:
	      table[i] = BASE_C;
	        count++;
	        buff[i] = checked;
	    break;
	    case BASE_G:
	      table[i] = BASE_G;
	        count++;
	        buff[i] = checked;
	    break;
	    case BASE_T:
	      table[i] = BASE_T;
	        count++;
	        buff[i] = checked;
	    break;
	  }
	}
	if (count >= 2){
	    write(tail, table, dfp);
	    init(buff);
	    number = 0;
	    continue;
	  }
	temp = count;
	for (int i = 0; i < tail; i++){
	  if (count != temp) break;
	  switch (buff[tail-i]){
	    case BASE_A:
	      table[tail-i] = BASE_A;
	        count++;
	        buff[tail-i] = checked;
	    break;
	    case BASE_C:
	      table[tail-i] = BASE_C;
	        count++;
	        buff[tail-i] = checked;
	    break;
	    case BASE_G:
	      table[tail-i] = BASE_G;
	        count++;
	        buff[tail-i] = checked;
	    break;
	    case BASE_T:
	      table[tail-i] = BASE_T;
	        count++;
	        buff[tail-i] = checked;
	    break;
	  }
	}
	write(tail, table, dfp);
	init(buff);
	number = 0;
	continue;
	//read process finish here
	number = 0;
	continue;
```

これで処理過程が完了したが，最後の 1 セットについては処理のループの終了後に判定する，内容は変わらないので省略する．

### <span id = "2.5">5. Diff{A}+Marker{B} シリーズ</span>
[Diff2+Marker11](#diffmarker) と同じ考え方で，Diff3+Marker9 などがある．これは名前の通り，塩基 **4** つで 1 セットとする． X<sub>1</sub> , X<sub>2</sub> , X<sub>3</sub> , Y とすると Y が Marker で，前に 3 つの塩基の符号化がある．判定方法は [Diff2+Marker11](#2.4) と少し異なるが，考え方は同じなので解説を省略する．
