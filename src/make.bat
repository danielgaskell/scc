@echo off

echo as.c
gcc as.c -o ..\bin\as.exe
echo cc.c
gcc cc.c -o ..\bin\cc.exe
echo cc0.c
gcc cc0.c -o ..\bin\cc0.exe
echo cc1.c
gcc cc1.c -o ..\bin\cc1.exe
echo cc2.c
gcc cc2.c -o ..\bin\cc2.exe
echo ld.c
gcc ld.c -o ..\bin\ld.exe
echo reloc.c
gcc reloc.c -o ..\bin\reloc.exe
echo copt.c
gcc copt.c -o ..\bin\copt.exe
copy /Y cc.hlp ..\bin\cc.hlp