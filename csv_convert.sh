#!/bin/bash
# @param input_file input file name
# @param prefix C-Preprocesser-Macro-Function name
# @param need_double_quote_index...
function convert_csv(){
  # argument
  readonly input_file=$1
  readonly prefix=$2
  shift 2
  local need_double_quote_index=($@)

  readonly need_double_quote_index_len=${#need_double_quote_index[@]}
  #write BOM
  echo -en '\xef\xbb\xbf'
  local line_string
  local is_first_line=1
  while read -r line_string; do
    if (( 1==is_first_line )); then
      is_first_line=0
      echo "//PREFIX,${line_string:0:-1},POSTFIX"
    else
        IFS_BACKUP=$IFS
        IFS=','
        elements=($line_string)
        IFS=$IFS_BACKUP
        local re="${prefix}(,"
        local i=0
        local j=0
        local e
        local tmp
        # debug print
        echo -en "converting id ${elements[0]}\r" >&2
        for e in "${elements[@]}"; do
          if (( i < need_double_quote_index_len && j == need_double_quote_index[i] )); then
            # ダブルクオートで囲う必要がある時
            re="${re}\"${e}\","
            (( i++ ))
          else
            tmp=$(echo "${e}" | sed -e 's/\//./')
            re="${re}${tmp},"
          fi
          (( j++ ))
        done
        echo "${re:0:-1},)"
    fi
  done < <(iconv -f cp932 -t UTF-8 "${input_file}")
}
echo "converting csv..."
echo "ships.csv:"
convert_csv './ships.csv' 'SHIP' 1 > 'KCS_CUI/source/ships_test.csv'
echo "slotitems.csv:"
convert_csv './slotitems.csv' 'WEAPON' 1 2 > 'KCS_CUI/source/slotitems_test.csv'
echo "done."
