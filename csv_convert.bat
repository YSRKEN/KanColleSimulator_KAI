@echo off
rem @brief csv converter to parse csv at C-Preprocesser
rem @param %1 input file
rem @param %2 prefix
setlocal enabledelayedexpansion
if "%1." == "." goto error
if "%2." == "." goto error
if "%3." == "." goto error

set /a is_fisrst_line=1
for /f "delims=" %%s in (%1) do (
    if 1 == !is_fisrst_line! (
        set /a is_fisrst_line = 0
        echo //PREFIX,%%s,POSTFIX>%3
    ) else (
        call :line_convert "%%s" "%2" "%3"
    )
)
exit /b 0

rem @brief convert line convert
rem @param source_line_text source line text
rem @param prefix C-Preprocesser-Macro-Function name
rem @param out output file full path
:line_convert
setlocal enabledelayedexpansion

set source_line_text=%~1

set prefix=^(,
set prefix=%~2%prefix%
set out=%~3

rem replace `/` to `.`
set source_line_text=%source_line_text:/=.%

rem get first element
for /f "delims=, tokens=1" %%s in ("%source_line_text%") do set re=%%s
rem get second first element and concat
for /f "delims=, tokens=2" %%s in ("%source_line_text%") do set re=%re%,"%%s",

call :strlen "third_elem_pos" "%re%"
set /a third_elem_pos-=2

rem `)` should be escaped by `^`
set postfix=,^)
rem concat strings
set re=%prefix%%re%!source_line_text:~%third_elem_pos%!%postfix%
echo %re%>>%out%
endlocal
exit /b 0

:strlen
setlocal enabledelayedexpansion
set result_env_name=%~1
set str=%~2

set /a count=0

:strlen_loop
if not defined str goto strlen_loop_break
set str=%str:~1%
set /a count+=1

goto strlen_loop
:strlen_loop_break
endlocal && set /a %result_env_name%=%count% && exit /b 0

:error
echo "invalid argument detect." 1> &2
exit /b 1
endlocal