@echo off

echo === Building crt0.s ===
..\..\bin\as crt0.s
copy /Y crt0.o ..\..\lib\crt0.o
