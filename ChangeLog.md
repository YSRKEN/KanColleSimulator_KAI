### Ver.2.2.0
- 開幕対潜を実装した。
- 夜偵による命中率補正を試験的に実装した。
- 夜間特殊攻撃のクリティカル発生率を修正した。
- 雷撃戦命中率の式を修正した。
- ケッコン艦は大口径主砲のアンフィットが軽減されるので反映させた。
- 艦娘/深海棲艦/装備データを更新した。
- サンプルデータを追加した。
- 砲撃戦命中率の式を大幅に変更した。
- 「かばう」および「探照灯」の挙動を大幅に修正した。
- 対地特殊効果を、砲台小鬼および離島棲姫用に大幅改修した。
- その他コードのリファクタリングを多数実施した。

### Ver.2.1.1
- 艦娘・装備データを更新した。
- Bofors 40mm四連装機関砲が摩耶機銃と同じ効果を持つのでCUI版を修正した。
- picojsonをsubmoduleで管理するようにした。
- 実行時にDLLが足りない場合はそのまま終了せず、警告を出すようにした。
- OSのbit数に応じて自動的に使用するCUI版を選択するようにした。

### Ver.2.1.0
- 索敵計算式(33式)について、計算式を微修正した。  
- GUIの操作性を向上させた。  
- 駆逐イ級のように、「スロットがフルでは埋まっていない」深海棲艦を含むマップが読み込めない不具合を修正した。  
- csvやdllが足りていない状態でGUIの起動を試みた際、例外を出さず静かに終了させるようにした。  
- GUIにて、艦隊形式(通常艦隊および空母機動・水上打撃・輸送護衛部隊)の指定と保存データとの対応がおかしかったのを修正した。  

### Ver.2.0.0
- GUIをVisual C#で実装した。  
- コンパイル時にcsvのデータを取り込むので、CUI版だけ実行する際はcsvが不要になった。  
- DLLを静的にリンクするようにしたので、ランタイムのインストールが不要になった。  
- 種々のチューニングにより、演算速度が倍以上になった。  
- 索敵計算式は2-5式(秋)だけでなく33式も実装した。

### Ver.1.4.0
- 対空カットイン判定・艦隊防空ボーナスをキャッシュすることで処理速度を向上させた。
- また、constexprによりコンパイル時にCSVデータを読み込むことで、記述の明瞭化および高速化を果たした。

### Ver.1.3.0
- 謎のボトルネックを解消して、マップモードをきちんと並列動作するようにした。  
- また、燃料/弾薬消費に伴う補正をより正確にし、触接を修正し、夜間触接および水上戦闘機を実装した。  
- なお、艦船・装備データも一部更新されている。

### Ver.1.2.0
- マップモードを実装した。また、勝利判定についてのバグを修正した。

### Ver.1.1.1
- 勝利判定が色々とおかしかったので修正。また、艦船データおよびサンプルデータを更新した。

### Ver.1.1.0
- 各種関数を高速化することでVer.1.0.1より倍は速くなったはず。また、艦船・装備データを更新した。

### Ver.1.0.1
- 既に沈んだ相手に対して追い打ちをかけてしまうことがあったので修正。

### Ver.1.0.0
- 夜戦まで完成したので、とりあえずリリースした。
