@echo off

echo as.c
gcc -m32 -static -static-libgcc -static-libstdc++ as.c -o ..\bin\as.exe
echo cc.c
gcc -m32 -static -static-libgcc -static-libstdc++ cc.c -o ..\bin\cc.exe
echo cc0.c
gcc -m32 -static -static-libgcc -static-libstdc++ cc0.c -o ..\bin\cc0.exe
echo cc1.c
gcc -m32 -static -static-libgcc -static-libstdc++ cc1.c -o ..\bin\cc1.exe
echo cc2.c
gcc -m32 -static -static-libgcc -static-libstdc++ cc2.c -o ..\bin\cc2.exe
echo ld.c
gcc -m32 -static -static-libgcc -static-libstdc++ ld.c -o ..\bin\ld.exe
echo reloc.c
gcc -m32 -static -static-libgcc -static-libstdc++ reloc.c -o ..\bin\reloc.exe
echo copt.c
gcc -m32 -static -static-libgcc -static-libstdc++ copt.c -o ..\bin\copt.exe
echo gfx2sgx.c
gcc -m32 -static -static-libgcc -static-libstdc++ gfx2sgx.c -o ..\bin\gfx2sgx.exe
echo sortmap.c
gcc -m32 -static -static-libgcc -static-libstdc++ sortmap.c -o ..\bin\sortmap.exe
