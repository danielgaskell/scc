@echo off

echo cpp.c
..\bin\cc -h 24576 -N "cpp" -o ..\bin\symbos\cpp.com cpp.c cpp-hash.c cpp-main.c cpp-token1.c cpp-token2.c
echo.
echo cc.c
..\bin\cc -N "cc" -DSYMBUILD -DBIN_PATH=256 -DMAX_PATH=256 -o ..\bin\symbos\cc.com cc.c
echo.
echo cc0.c
..\bin\cc -h 0 -N "cc0" -o ..\bin\symbos\cc0.com cc0.c
echo.
echo cc1.c
..\bin\cc -h 0 -N "cc1" -o ..\bin\symbos\cc1.com cc1.c
echo.
echo cc2.c
..\bin\cc -N "cc2" -o ..\bin\symbos\cc2.com cc2.c
echo.
echo as.c
..\bin\cc -N "as" -DSYMBUILD -o ..\bin\symbos\as.com as.c
echo.
echo ld.c
..\bin\cc -h 16384 -N "ld" -o ..\bin\symbos\ld.com ld.c
echo.
echo reloc.c
..\bin\cc -h 0 -N "reloc" -o ..\bin\symbos\reloc.com reloc.c
echo.
echo copt.c
..\bin\cc -N "copt" -o ..\bin\symbos\copt.com copt.c

copy /Y cc.hlp ..\bin\symbos\cc.hlp
