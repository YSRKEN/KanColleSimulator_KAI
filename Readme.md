# KanColleSimulator

## 概要
艦これの戦闘をシミュレートします。

## 使い方
`usage: KCS_CUI -i input1.json input2.json [-f formation1 formation2] [-n times] [-t threads] [-o output.csv]`

- input1.jsonは自艦隊、input2.jsonは敵艦隊かマップのデータ
- 自艦隊には連合艦隊も許容されるが、敵艦隊やマップ内の艦隊は通常艦隊のみ
- formation1は自艦隊、formation2は敵艦隊の陣形。
　単縦陣・複縦陣・輪形陣・梯形陣・単横陣が0～4に対応する。  
　なお、省略するとどちらも単縦陣になる。
　(連合艦隊だと、第一～第四警戒航行序列が4・1・2・0に対応する)
- input2.jsonがマップのデータだと、formation2に書かれた陣形は無視される。  
　また、マップモードにおける自陣形は、昼戦or3隻以下だと単縦陣・  
　敵旗艦が潜水艦だと(開幕夜戦マス含めて)単横陣・  
　開幕夜戦マスは(対潜マスを除いて)formation1で指定した陣形になる
- timesは試行回数。値が1だと艦隊の詳細も表記される。略すと1になる
- threadsは実行スレッド数。略すとシングルスレッドで実行される
- output.csvを指定すると、結果を標準出力ではなくCSVに書き出すようになる。  
また、出力形式も標準出力のような感じ(まとめ表示)ではなく各回についての結果が書き出される

## 使用例
`KCS_CUI -i "sample\fleet sample1.json" "sample\fleet sample2.json" -f 0 1 -n 1`

`KCS_CUI -i "sample\fleet 1-5.json" "sample\map 1-5 high.json" -f 0 0 -n 10000 -t 2 -o output.csv`

`KCS_CUI -i "sample\fleet 2016冬E3甲輸送.json" "sample\map 2016冬E3甲輸送.json" -f 3 4 -n 100000 -t 4 -o output.csv`

## JSONデータの書式(艦隊編)
- UTF-8で保存してください(どうせASCII文字しか無いと思いますが)
- デッキビルダー形式を参考にしつつ、機能を拡張した
- 通常艦隊だと第1艦隊、連合艦隊だと第1・第2艦隊しか参照しない
- 「version」と同じ階層で「lv」(司令部レベル)を設置した  
(これが存在しない場合は司令部レベル120とする)
- 「version」と同じ階層で「type」(艦隊)を設置した。1なら通常艦隊、2なら連合艦隊  
(これが存在しない場合は通常艦隊とする)
- 「luck」と同じ階層で「cond」(cond値)を設置した  
(これが存在しない場合はcond値49とする)
- 「rf」と同じ階層で「rf_detail」(内部熟練度)を設置した  
(これが存在する場合は、rfの設定を上書きする)

## JSONデータの書式(マップ編)

## ソースコードの書式(C++編)
- 変数の型は、基本的にcstdintの型を使うようにする。また、unsignedを多用しないようにする
- 変数はスネークケースだが、メンバ変数は末尾に`_`を付ける
- 定数名はアッパーキャメルケースの先頭に`k`を付ける。CONST_NAMEとは書かない
- クラス名および関数名はアッパーキャメルケースとする
- getter・setterの接頭語にはそれぞれGet・Setを付けるようにする
- マクロは非推奨だが、用いる場合はMACRO_CODEのようにする

## オプションで変更可能な箇所(予定)
- 艦娘のレベルの最大値
- 索敵計算のメソッド
- 索敵フェイズの成功判定
- 触接開始率の計算方法(開始率および選択率)
- 空中戦における敵機の撃墜率
- 対空カットインにおける発動確率
- かばいが発生する確率、およびかばう艦の選定方法
- 命中率計算式
- フィット砲補正の計算式
- クリティカル発動率

## 注意点
- 補強増設には対応していません。
- 航空戦マス、および空襲戦マスには対応していません。
- 支援艦隊には対応していません。

## ライセンス
MITライセンスとします。

## 作者
- YSR https://github.com/YSRKEN
- yumetodo https://github.com/yumetodo
- Cranberries https://github.com/LoliGothick

## 参考資料
- ほっぽアルファのデータライブラリからships0209.csvとslotitems0209.csvを頂いた  
(ソフトウェア用に適宜改造している)  
https://github.com/koshian2/HoppoAlpha.DataLibrary/tree/master/HoppoAlpha.DataLibrary
- 艦種記号はこのページを参考にした  
http://uswarships.jounin.jp/sub4.htm
- usageの書式はこのページを参考にした  
http://www.fujlog.net/2014/04/command-line-getopt-usage-format.html
- Markdown記法はこのサイトに準じている  
http://tatesuke.github.io/KanTanMarkdown/ktm-full.html
- C++コードの書式は「Google C++ スタイルガイド」に準じている  
http://www.textdrop.net/google-styleguide-ja/cppguide.xml
- JSON読み込みにはpicojsonを使用した  
https://github.com/kazuho/picojson
- 艦これの仕様は通常wiki・検証wikiなどを参考にした  
http://wikiwiki.jp/kancolle/  
http://ja.kancolle.wikia.com/wiki/%E8%89%A6%E3%81%93%E3%82%8C_%E6%A4%9C%E8%A8%BCWiki  
https://github.com/andanteyk/ElectronicObserver/blob/master/ElectronicObserver/Other/Information/kcmemo.md#%E8%89%A6%E3%81%93%E3%82%8C%E3%81%AE%E4%BB%95%E6%A7%98%E3%81%AB%E9%96%A2%E3%81%99%E3%82%8B%E9%9B%91%E5%A4%9A%E3%81%AA%E3%83%A1%E3%83%A2
http://kancollecalc.web.fc2.com/damage_formula.html
http://bs-arekore.at.webry.info/201502/article_4.html
https://twitter.com/Xe_UCH/status/700169825121804288
