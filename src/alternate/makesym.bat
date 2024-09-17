@echo off

echo cpp.c
..\..\bin\cc -h 24576 -N "cpp" -DSYMBUILD -o ..\..\bin\symbos\alternate\cpp.exe cpp.c cpp-hash.c cpp-main.c cpp-token1.c cpp-token2.c
