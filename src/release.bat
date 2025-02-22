@echo off
call make.bat
call makesym.bat

echo Converting docs to .HLP...
python ..\doc\md2hlp.py ..\doc\index.md
python ..\doc\md2hlp.py ..\doc\symbos.md
python ..\doc\md2hlp.py ..\doc\syscall1.md
python ..\doc\md2hlp.py ..\doc\syscall2.md
python ..\doc\md2hlp.py ..\doc\graphics.md
python ..\doc\md2hlp.py ..\doc\porting.md
python ..\doc\md2hlp.py ..\doc\special.md
echo Converting docs to .PDF...
cd ..\doc
python md2pdf.py
cd ..\src

mkdir ..\release\win32
mkdir ..\release\win32\bin
mkdir ..\release\win32\doc
mkdir ..\release\win32\lib
mkdir ..\release\win32\lib\include
mkdir ..\release\win32\lib\include\sys
mkdir ..\release\win32\lib\include\symbos
mkdir ..\release\win32\sample
mkdir ..\release\symbos
mkdir ..\release\symbos\doc
mkdir ..\release\symbos\lib
mkdir ..\release\symbos\lib\include
mkdir ..\release\symbos\lib\include\sys
mkdir ..\release\symbos\lib\include\symbos
mkdir ..\release\symbos\sample
del /S /Q ..\release\*.*

copy /Y ..\bin\*.exe ..\release\win32\bin
copy /Y ..\bin\*.z80 ..\release\win32\bin
copy /Y ..\doc\*.txt ..\release\win32\doc
copy /Y ..\doc\*.md ..\release\win32\doc
copy /Y ..\doc\*.png ..\release\win32\doc
copy /Y ..\lib\*.a ..\release\win32\lib
copy /Y ..\lib\*.o ..\release\win32\lib
copy /Y ..\lib\include\*.h ..\release\win32\lib\include
copy /Y ..\lib\include\sys\*.h ..\release\win32\lib\include\sys
copy /Y ..\lib\include\symbos\*.h ..\release\win32\lib\include\symbos
copy /Y ..\sample\windemo.c ..\release\win32\sample
copy /Y ..\sample\threads.c ..\release\win32\sample
copy /Y ..\sample\basic.c ..\release\win32\sample
copy /Y ..\sample\makebas.bat ..\release\win32\sample
copy /Y ..\sample\calc.sgx ..\release\win32\sample
copy /Y ..\sample\calc16.sgx ..\release\win32\sample
copy /Y ..\sample\progcalc.c ..\release\win32\sample
copy /Y ..\sample\makecalc.bat ..\release\win32\sample
copy /Y ..\sample\ball.png ..\release\win32\sample
copy /Y ..\sample\ball.sgx ..\release\win32\sample
copy /Y ..\sample\gfxdemo.c ..\release\win32\sample
copy /Y ..\sample\makegfx.bat ..\release\win32\sample
copy /Y ..\sample\snddemo.c ..\release\win32\sample
copy /Y ..\sample\startup.spm ..\release\win32\sample

copy /Y install.txt ..\release\symbos
copy /Y ..\bin\symbos\*.exe ..\release\symbos
copy /Y ..\bin\symbos\*.com ..\release\symbos
copy /Y ..\bin\symbos\*.z80 ..\release\symbos
copy /Y ..\doc\apache.txt ..\release\symbos\doc\apache.txt
copy /Y ..\doc\artistic.txt ..\release\symbos\doc\artistic.txt
copy /Y ..\doc\cpp-lic.txt ..\release\symbos\doc\cpp-lic.txt
copy /Y ..\doc\gpl-3.0.txt ..\release\symbos\doc\gpl3.txt
copy /Y ..\doc\lgpl-3.0.txt ..\release\symbos\doc\lgpl3.txt
copy /Y ..\doc\*.hlp ..\release\symbos\doc
copy /Y ..\lib\*.a ..\release\symbos\lib
copy /Y ..\lib\*.o ..\release\symbos\lib
copy /Y ..\lib\include\*.h ..\release\symbos\lib\include
copy /Y ..\lib\include\sys\*.h ..\release\symbos\lib\include\sys
copy /Y ..\lib\include\symbos\*.h ..\release\symbos\lib\include\symbos
copy /Y ..\sample\windemo.c ..\release\symbos\sample
copy /Y ..\sample\threads.c ..\release\symbos\sample
copy /Y ..\sample\calc.sgx ..\release\symbos\sample
copy /Y ..\sample\calc16.sgx ..\release\symbos\sample
copy /Y ..\sample\progcalc.c ..\release\symbos\sample
copy /Y ..\sample\makecalc_native.bat ..\release\symbos\sample\makecalc.bat
copy /Y ..\sample\ball.png ..\release\symbos\sample
copy /Y ..\sample\ball.sgx ..\release\symbos\sample
copy /Y ..\sample\gfxdemo.c ..\release\symbos\sample
copy /Y ..\sample\makegfx_native.bat ..\release\symbos\sample\makegfx.bat
copy /Y ..\sample\snddemo.c ..\release\\symbos\sample
copy /Y ..\sample\startup.spm ..\release\\symbos\sample
