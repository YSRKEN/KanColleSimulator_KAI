#!/bin/bash
# @param input_file input file name
# @param prefix C-Preprocesser-Macro-Function name
# @param need_double_quote_index...
function convert_csv(){
  # argument
  local -r input_file=$1
  local -r prefix=$2
  shift 2
  local need_double_quote_index=($@)

  local -r need_double_quote_index_len=${#need_double_quote_index[@]}
  #write BOM
  echo -en '\xef\xbb\xbf'
  local line_string
  local is_first_line=1
  while read -r line_string; do
    if (( 1==is_first_line )); then
      is_first_line=0
      echo "//PREFIX,${line_string:0:-1},POSTFIX"
    else
      local IFS_BACKUP=$IFS
      IFS=','
      local elements=($line_string)
      IFS=$IFS_BACKUP
      local elements_len=${#elements[@]}
      local i=0
      local j
      echo -en "\rconverting ${input_file}... id ${elements[0]}" >&2
      for (( j=0; j < elements_len; j++ )); do
        if (( i < need_double_quote_index_len && j == need_double_quote_index[i] )); then
          # ダブルクオートで囲う必要がある時
          elements[$j]="\"${elements[$j]}\""
          (( i++ ))
        else
          # elements[$j]=$(echo "${elements[$j]}" | sed -e 's/\//./g')
          elements[$j]=${elements[$j]//\//.}
        fi
      done
      IFS=, eval 'local re=${elements[*]}'
      echo "${prefix}(,${re},)"
      # echo "${prefix}(,$(IFS=,; echo "${elements[*]}"),)"
    fi
  done < <(iconv -f cp932 -t UTF-8 "${input_file}")
  echo -e "\rconverting ${input_file}..........done." >&2
}
echo "converting csv..."
convert_csv './ships.csv' 'SHIP' 1 > 'KCS_CUI/source/ships.csv'
convert_csv './slotitems.csv' 'WEAPON' 1 2 > 'KCS_CUI/source/slotitems.csv'
echo "done."
