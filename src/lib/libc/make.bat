@echo off

dir /b *.c >files.lst
for /F "tokens=*" %%A in (files.lst) do (echo %%A & ..\..\..\bin\cc -Os -c %%A)

ar rc libc.a *.o
copy /Y libc.a ..\..\..\lib\libc.a
del /Q *.o
