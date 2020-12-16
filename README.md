# algo_grpwk_17
## Diff3+Marker9 説明
サンプルの符号化をもとに前後に2文字を加える


| orgdata |->| encdata |
|:---:|:---:|:---:| 
| 00 |A| CAG |
| 01 |C| GCT |
| 10 |G| TGA |
| 11 |T| ATC |

4文字(orgdata 8ビット)を1セットとします  
前3文字を表のように符号化します  
最後の4番目はそのまま9回書きます  
###### 例  
| orgdata | encdata |
|:---:|:---:| 
| 00101100 | CAGTGAATCAAAAAAAAA |
| 01110110 | GCTATCGCTGGGGGGGGG |
| 10101011 | TGATGATGATTTTTTTTT |
| 11000101 | ATCCAGGCTCCCCCCCCC |
