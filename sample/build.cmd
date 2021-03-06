@echo off
setlocal

echo Building 32-bit executable...
icc.exe -Q -Gm+ -Gd+ -Ge+ -G5 -Ti -C bla.c
ilink.std -NOL -DE -DB -M -E:2 -NOPACKC -BAS:0x10000 -ST:0x8000 -PM:VIO -O:bla.exe bla.obj
dllrname.exe /Q /N bla.exe CPPOM30=OS2OM30 DOSCALLS=NEWCALLS

echo Building 16-bit executable...
SET PATH=d:\ddk\base\tools;%PATH%
SET INCLUDE=d:\ddk\base\h
SET LIB=d:\ddk\base\lib
cl.exe /nologo /AS /Od /G2 /Zi /Zl /c bla.c
link.exe /NOL /BATCH /CO /NOD /MAP /EXE /ST:0x8000 /PM:VIO bla.obj,bla16.exe,,os2286.lib+slibcep.lib;
dllrname.exe /Q /N bla16.exe DOSCALLS=NEWCALLS

endlocal
