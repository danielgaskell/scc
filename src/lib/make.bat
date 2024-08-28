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
echo kernmult.s
..\..\bin\as kernmult.s

echo argv.c
..\..\bin\cc -Os -c argv.c
echo desktop.c
..\..\bin\cc -Os -c desktop.c
echo dir.c
..\..\bin\cc -Os -c dir.c
echo dirread.c
..\..\bin\cc -Os -c dirread.c
echo file.c
..\..\bin\cc -Os -c filebox.c
echo kernel.c
..\..\bin\cc -Os -c kernel.c
echo memsizex.c
..\..\bin\cc -Os -c memsizex.c
echo msgbox.c
..\..\bin\cc -Os -c msgbox.c
echo system.c
..\..\bin\cc -Os -c system.c

echo Building libsym.a...
ar rc libsym.a kernbank.o kernclip.o kerndev.o kernmem.o kernmult.o argv.o desktop.o dir.o dirread.o filebox.o kernel.o msgbox.o memsizex.o system.o

copy /Y crt0.o ..\..\lib\crt0.o
copy /Y libsym.a ..\..\lib\libsym.a
move crt0.o crt0.tmp
del /Q *.o
move crt0.tmp crt0.o
