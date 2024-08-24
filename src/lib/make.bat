@echo off

echo crt0.s
..\..\bin\as crt0.s

echo kernbank.s
..\..\bin\as kernbank.s
echo kernclip.s
..\..\bin\as kernclip.s
echo kerndev.s
..\..\bin\as kerndev.s
echo kernmem.s
..\..\bin\as kernmem.s
echo kernmsg.s
..\..\bin\as kernmsg.s
echo kernmult.s
..\..\bin\as kernmult.s

echo argv.c
..\..\bin\cc -c argv.c
echo desktop.c
..\..\bin\cc -c desktop.c
echo dir.c
..\..\bin\cc -c dir.c
echo file.c
..\..\bin\cc -c file.c
echo filebox.c
..\..\bin\cc -c filebox.c
echo kernel.c
..\..\bin\cc -c kernel.c
echo msgbox.c
..\..\bin\cc -c msgbox.c
echo shell.c
..\..\bin\cc -c shell.c
echo system.c
..\..\bin\cc -c system.c

echo Building libsym.a...
ar rc libsym.a kernbank.o kernclip.o kerndev.o kernmem.o kernmsg.o kernmult.o argv.o desktop.o dir.o file.o filebox.o kernel.o msgbox.o shell.o system.o

copy /Y crt0.o ..\..\lib\crt0.o
copy /Y libsym.a ..\..\lib\libsym.a
move crt0.o crt0.tmp
del /Q *.o
move crt0.tmp crt0.o
