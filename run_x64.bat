x64\KCS_CUI -i "sample\sample1.json" "sample\sample2.json" -f 0 0 -n 1 -t 1 > log1_x64.txt
x64\KCS_CUI -i "sample\sample1.json" "sample\sample2.json" -f 0 0 -n 10000 -t 4 -o output1_x64.json > log2_x64.txt
x64\KCS_CUI -i "sample\3-5.json" "sample\3-5 high.map" -f 3 0 -n 10000 -t 4 -o output2_x64.json > log3_x64.txt
pause
