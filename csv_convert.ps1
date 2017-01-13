function Convert-CSV {
    param(
        [string]$input_file, [string]$prefix, [int[]]$need_double_quote_index, [string]$out_file
    )
    process{
        $is_first_line = 1
        Get-Content $input_file | foreach {
            # bind
            $line_string = $_
            switch ($is_first_line) {
                1 {
                    $is_first_line = 0
                    "//PREFIX,$line_string,POSTFIX" | out-file -encoding:utf8 $out_file
                }
                Default {
                    $elements = $line_string.Split(',')
                    $sb = New-Object System.Text.StringBuilder
                    $dst = $sb.Append($prefix).Append('(,')
                    $i = 0
                    $j = 0
                    foreach ($e in $elements) {
                        if(($i -lt $need_double_quote_index.Length) -and ($need_double_quote_index[$i] -eq $j)){
                            # ダブルクオートで囲う必要がある時
                            $dst = $sb.Append('"').Append($e).Append('"').Append(',')
                            ++$i
                        }else {
                            $e = $e -creplace '/', '.'
                            $dst = $sb.Append($e).Append(',')
                        }
                        ++$j
                    }
                    # remove last `,`
                    --$sb.Length
                    $sb.ToString() + ',)' | Add-Content -encoding:utf8 $out_file
                }
            }
        }
    }
}
Convert-CSV '.\ships.csv' 'SHIP' @(,1) 'KCS_CUI/source/ships.csv'
Convert-CSV '.\slotitems.csv' 'WEAPON' @(1,2) 'KCS_CUI/source/slotitems.csv'
