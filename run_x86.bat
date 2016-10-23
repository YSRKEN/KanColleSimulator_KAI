x86\KCS_CUI -i "sample\sample1.json" "sample\sample2.json" -f 0 0 -n 1 -t 1 > log1.txt
x86\KCS_CUI -i "sample\sample1.json" "sample\sample2.json" -f 0 0 -n 100000 -t 8 -o output1.json > log2.txt
x86\KCS_CUI -i "sample\3-5.json" "sample\3-5 high.map" -f 3 0 -n 100000 -t 8 -o output2.json > log3.txt
pause
