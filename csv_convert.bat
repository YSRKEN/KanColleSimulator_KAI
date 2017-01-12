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

:line_convert
setlocal enabledelayedexpansion

set source_line_text=%~1

set prefix=^(,
set prefix=%~2%prefix%
set out=%~3

rem replace `/` to `.`
set source_line_text=%source_line_text:/=.%

rem `)` should be escaped by `^`
set postfix=,^)
rem concat strings
set re=%prefix%%source_line_text%%postfix%
echo %re%>>%out%
endlocal
exit /b 0

:error
echo "invalid argument detect." 1> &2
exit /b 1
endlocal