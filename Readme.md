﻿
# KanColleSimulator

[master](https://github.com/YSRKEN/KanColleSimulator_KAI) : [![Run Status](https://api.shippable.com/projects/577cc61f3be4f4faa56be960/badge?branch=master)](https://app.shippable.com/projects/577cc61f3be4f4faa56be960)
[develop](https://github.com/YSRKEN/KanColleSimulator_KAI/tree/develop): [![Run Status](https://api.shippable.com/projects/577cc61f3be4f4faa56be960/badge?branch=develop)](https://app.shippable.com/projects/577cc61f3be4f4faa56be960)

## 概要
艦これの戦闘をシミュレートします。  
※CUI版のexeファイル(KCS_CUI.exe)は、x64フォルダにある64bit版と、x86フォルダにある32bit版とがあります。

## 使い方(GUI編)
- 艦隊エディタ・マップエディタ・シミュレートと3つのタブに分かれています
- 艦隊エディタを開いている際は、艦隊データ(*.json)をメニューから読み込みorドラッグ＆ドロップできます
- マップエディタを開いている際は、マップデータ(*.map)をメニューから読み込みorドラッグ＆ドロップできます
- 運の初期値は-1(つまりデフォルト値)に設定されています
- 外部熟練度と内部熟練度は連動していて、片方を変更するともう片方も自動で変化します
- 後述するように、艦隊vsマップの場合、自陣形指定は(対潜マスを除く)開幕夜戦マス用で、敵陣形指定は無視されます

## 使い方(CUI編)
`usage: KCS_CUI -i input1.json input2.json|input2.map [-f formation1 formation2] [-n times] [-t threads] [-o output.json] [--result-json-prettify | --no-result-json-prettify]`

- input1.jsonは自艦隊、input2.json|input2.mapは敵艦隊かマップのデータ。  
後者がマップのデータならマップモード、そうでないなら通常モードと呼ぶ
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
- 結果は標準出力に書き出される。マップモードなら、道中の到達率・大破撤退率も出る
- output.jsonを指定すると、結果を標準出力だけではなくJSONに書き出すようになる。  
なお、マップモードでは、ボスマスでの結果のみJSONに書き出される
- --no-result-json-prettifyを指定すると、出力するJSONが整形されないようになる

## 使用例(CUI編)
`KCS_CUI -i "sample\sample1.json" "sample\sample2.json" -f 0 0 -n 1`

`KCS_CUI -i "sample\3-5.json" "sample\3-5 high.map" -f 0 0 -n 100000 -t 4 -o output.json`

## JSONデータの書式(艦隊編)
- UTF-8で保存してください(どうせASCII文字しか無いと思いますが)
- デッキビルダー形式を参考にしつつ、機能を拡張した
- 通常艦隊だと第1艦隊、連合艦隊だと第1・第2艦隊しか参照しない
- 「version」と同じ階層で「lv」(司令部レベル)を設置した  
(これが存在しない場合は司令部レベル120とする)
- 「version」と同じ階層で「type」(艦隊)を設置した。1なら通常艦隊、2～4なら連合艦隊  
(これが存在しない場合は通常艦隊とする)  
(連合艦隊は、空母機動部隊・水上打撃部隊・輸送護衛部隊がそれぞれ2～4に対応)
- 「luck」と同じ階層で「cond」(cond値)を設置した  
(これが存在しない場合はcond値49とする)
- 「rf」と同じ階層で「rf_detail」(内部熟練度)を設置した  
(これが存在する場合は、rfの設定を上書きする)

## JSONデータの書式(マップ編)
- UTF-8で保存してください(どうせASCII文字しか無いと思いますが)
- 「"version": "map"」は決まり文句です
- 「"position": [各マスについての情報の配列]」となっています
- 各マスについての情報は「"name":マス名」「"pattern"：艦隊の配列」「戦闘モード」からなります
- 艦隊の配列は、「"fleets":艦船IDの配列」「"form"：陣形」からなります
- マップに居る艦船の装備は初期装備です(ships.csvを参照)
- formは陣形で、上記におけるformationと意味は同じです
- modeは戦闘モードで、0が昼夜戦闘(通常ボスマスのみ)・1が昼戦のみ・2が開幕夜戦マスです
- 艦隊の配列は、マスに踏み込むたびにランダムで選択するためのものです

## ソースコードの書式(C++編)
- 変数はスネークケースだが、メンバ変数は末尾に`_`を付ける
- 定数名はアッパーキャメルケースの先頭に`k`を付ける。CONST_NAMEとは書かない
- クラス名および関数名はアッパーキャメルケースとする
- getter・setterの接頭語にはそれぞれGet・Setを付けるようにする
- マクロは非推奨だが、用いる場合はMACRO_CODEのようにする

## 注意点
- ダメコンおよび戦闘兵食・給油艦による補給には対応していません。
- したがって、マップモードでは大破した際必ず撤退します
- 護衛退避には対応していません
- 補強増設には対応していません
- 航空戦マス、および空襲戦マスには対応していません
- 支援艦隊には対応していません

## ライセンス
MITライセンスとします。

## 開発者
|名前       |GitHub                        |Twitter                      |
|:----------|:-----------------------------|:----------------------------|
|YSR        |https://github.com/YSRKEN     |https://twitter.com/YSRKEN   |
|yumetodo   |https://github.com/yumetodo   |https://twitter.com/yumetodo |
|Cranberries|https://github.com/LoliGothick|https://twitter.com/_EnumHack|
|sayurin    |https://github.com/sayurin    |https://twitter.com/haxe     |
|taba       |https://github.com/taba256    |https://twitter.com/__taba__ |

## clone 方法
```
git clone --recursive
```

もしくは

```
git clone
git submodule init
git submodule update
```

してください。submoduleを使用しています

## バージョン履歴
ChangeLog.mdを参照

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
https://twitter.com/Xe_UCH/status/705281106011029505  
http://ch.nicovideo.jp/biikame/blomaga/ar850895  
https://docs.google.com/spreadsheets/d/1O91DpCaHbjCZV2jy1GlMyqjwyWULdjK5kwhYALPFJGE/edit  
http://ch.nicovideo.jp/umya/blomaga/ar899278
https://twitter.com/noratako5/status/707526758434607104
- おのれSourceTree……勝手に特定拡張子を自動で除外するとは……  
http://tech.nitoyon.com/ja/blog/2013/04/05/sourcetree/
