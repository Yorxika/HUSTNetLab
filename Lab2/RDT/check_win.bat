@echo off
:: appname 程序名
:: inputname 输入文件名
:: outputname 输出文件名
:: resultname 程序控制台输出重定向文件名

set appname=".\StopWait\Debug\StopWait.exe"
set inputname=".\input.txt"
set outputname0=".\output0.txt"
set outputname1=".\output1.txt"
set outputname2=".\output2.txt"
set outputname3=".\output3.txt"
set outputname4=".\output4.txt"
set outputname5=".\output5.txt"
set outputname6=".\output6.txt"
set outputname7=".\output7.txt"
set outputname8=".\output8.txt"
set outputname9=".\output9.txt"
set resultname=".\result.txt"

for /l %%i in (1,1,3) do (
   :: echo Test %appname% %%i:
   :: %appname% > %resultname% 2>&1
    fc /N %inputname% %outputname0%
    fc /N %inputname% %outputname1%
    fc /N %inputname% %outputname2%
    fc /N %inputname% %outputname3%
    fc /N %inputname% %outputname4%
    fc /N %inputname% %outputname5%
    fc /N %inputname% %outputname6%
    fc /N %inputname% %outputname7%
    fc /N %inputname% %outputname8%
    fc /N %inputname% %outputname9%
    pause
)
pause