SETLOCAL
rem 压力测试，替换(1,1,100)中的100为测试数
rem url替换为http://127.0.0.1:5050/index.html

set url=http://127.0.0.1:5050/index.html
set cmd1= curl %url%

for /L %%i in (1,1,100) do %cmd1% 
ENDLOCAL
pause
