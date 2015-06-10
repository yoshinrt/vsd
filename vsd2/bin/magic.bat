cd ..\exec_sram\Exe

echo zF15EF117*> F15E.txt
copy /b F15E.txt+vsd2.s19 vsd2_magic.txt
del F15E.txt
