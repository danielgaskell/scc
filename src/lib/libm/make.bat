@echo off
Setlocal EnableDelayedExpansion

REM An important part of the fcc build chain is lorderz80, which orders
REM libraries by forward references so that ld can parse them sequentially.
REM This involves some convoluted shell scripting which has not yet been
REM ported to Windows. Run order.sh on a Linux box with fcc built to generate
REM the file libm.fil. If this file already exists (and no new forward refs
REM have been added), the build can proceed as normal on Windows. (This file
REM can also be updated manually - references should always come after other
REM object files that require them.)

echo === C files ===

del /Q *.o
dir /B *.c >files.lst
for /F "tokens=*" %%A in (files.lst) do (echo %%A & ..\..\..\bin\cc -Os -c %%A)

echo === Assembler files ===
dir /B *.s >files.lst
for /F "tokens=*" %%A in (files.lst) do (echo %%A & ..\..\..\bin\as %%A)

echo === Building libm.a ===
del /Q libm.a
set params=
for /F "tokens=*" %%A in (libm.fil) do set params=!params! %%A
ar rc libm.a %params%
REM ar rc libm.a *.o
copy /Y libm.a ..\..\..\lib\libm.a

REM del /Q *.o