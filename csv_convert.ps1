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
                        [string]$elem = $e
                        # Read-Host "続けるにはENTERキーを押して下さい"
                        if(($i -lt $need_double_quote_index.Length) -and ($need_double_quote_index[$i] -eq $j)){
                            $sb2 = New-Object System.Text.StringBuilder
                            $elem = $sb2.Append('"').Append($elem).Append('"').ToString()
                            ++$i
                        }else {
                            $elem = $elem -creplace '/', '.'
                        }
                        $dst = $sb.Append($elem).Append(',')
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
