.SUFFIXES:
.SUFFIXES:.c .obj

!IFDEF DEBUG
CFLAGS=/Ti
LFLAGS=/DE /DB /NOBAS
!ELSE
CFLAGS=/O
LFLAGS=/NOBAS
!ENDIF

.c.obj:
   $(CC) /Gm+ /Gd+ /Ge- /G5 $(CFLAGS) /C $<

newcalls.dll: newcalls.obj
   -7 ilink.std /NOL $(LFLAGS) /DLL /M /O:$@ /E:2 $** newfwd.def os2386.lib mmpm2.lib >out
   dllrname /Q /N $@ CPPOM30=OS2OM30

