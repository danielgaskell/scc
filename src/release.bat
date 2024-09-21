@echo off
call make.bat
call makesym.bat

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
copy /Y ..\bin\*.hlp ..\release\win32\bin
copy /Y ..\bin\*.z80 ..\release\win32\bin
copy /Y ..\doc\*.md ..\release\win32\doc
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

copy /Y install.txt ..\release\symbos
copy /Y ..\bin\symbos\*.exe ..\release\symbos
copy /Y ..\bin\symbos\*.com ..\release\symbos
copy /Y ..\bin\symbos\*.hlp ..\release\symbos
copy /Y ..\bin\symbos\*.z80 ..\release\symbos
copy /Y ..\doc\*.md ..\release\symbos\doc
copy /Y ..\lib\*.a ..\release\symbos\lib
copy /Y ..\lib\*.o ..\release\symbos\lib
copy /Y ..\lib\include\*.h ..\release\symbos\lib\include
copy /Y ..\lib\include\sys\*.h ..\release\symbos\lib\include\sys
copy /Y ..\lib\include\symbos\*.h ..\release\symbos\lib\include\symbos
copy /Y ..\sample\windemo.c ..\release\symbos\sample
copy /Y ..\sample\threads.c ..\release\symbos\sample
copy /Y ..\sample\basic.c ..\release\symbos\sample
copy /Y ..\sample\makebas_native.bat ..\release\symbos\sample\makebas.bat
copy /Y ..\sample\calc.sgx ..\release\symbos\sample
copy /Y ..\sample\calc16.sgx ..\release\symbos\sample
copy /Y ..\sample\progcalc.c ..\release\symbos\sample
copy /Y ..\sample\makecalc_native.bat ..\release\symbos\sample\makecalc.bat
