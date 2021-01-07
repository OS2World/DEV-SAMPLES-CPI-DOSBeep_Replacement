.SUFFIXES:
.SUFFIXES:.c .obj

.c.obj:
   $(CC) /Gm+ /Gd+ /Ge- /G5 /Ti /C newcalls.c

newcalls.dll: newcalls.obj
   -7 ilink.std /NOL /DE /DB /DLL /M /O:$@ /E:2 $** newfwd.def os2386.lib mmpm2.lib >out
   dllrname /Q /N $@ CPPOM30=OS2OM30

