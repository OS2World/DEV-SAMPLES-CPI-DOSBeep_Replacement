/* REXX script */

TAB = d2c(9);
filename =newfwd.def

'@del /F 'newfwd.def' >NUL'

rc = lineout(filename,,1); /* overwrite */
rc = lineout(filename,'LIBRARY NEWCALLS INITINSTANCE TERMINSTANCE');
rc = lineout(filename,"DESCRIPTION 'Forwarder DLL to DOSCALLS, replacing DosBeep'");
rc = lineout(filename,'DATA MULTIPLE NONSHARED');
rc = lineout(filename,'');
rc = lineout(filename,'IMPORTS');
do n=1 to 49
  rc = lineout(filename,TAB'_undoc'n'=DOSCALLS.'n);
end
rc = lineout(filename,TAB'ORIGDOS16BEEP=DOSCALLS.50');
do n=51 to 285
  rc = lineout(filename,TAB'_undoc'n'=DOSCALLS.'n);
end
rc = lineout(filename,TAB'OrigDosBeep=DOSCALLS.286');
do n=287 to 1117
  rc = lineout(filename,TAB'_undoc'n'=DOSCALLS.'n);
end
n.1=9004;
n.2=9005;
n.3=9006;
n.4=9007;
n.5=9008;
n.6=9010;
n.7=9011;
n.8=9018;
do i=1 to 8
  rc = lineout(filename,TAB'_undoc'n.i'=DOSCALLS.'n.i);
end

rc = lineout(filename,'');
rc = lineout(filename,'EXPORTS');
do n=1 to 49
  rc = lineout(filename,TAB'_undoc'n' @'n);
end
rc = lineout(filename,TAB'DOS16BEEP @50');
do n=51 to 285
  rc = lineout(filename,TAB'_undoc'n' @'n);
end
rc = lineout(filename,TAB'DosBeep @286');
do n=287 to 1117
  rc = lineout(filename,TAB'_undoc'n' @'n);
end
n.1=9004;
n.2=9005;
n.3=9006;
n.4=9007;
n.5=9008;
n.6=9010;
n.7=9011;
n.8=9018;
do i=1 to 8
  rc = lineout(filename,TAB'_undoc'n.i' @'n.i);
end

rc = lineout(filename,'');
rc = lineout(filename);
return 0;

