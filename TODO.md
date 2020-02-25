TODO items for EmuLegOs:

-  rename to emuBrickOS
-  Add LNP Windows support, not as easy as on Linux as the Windows LNP
   API is completely different from the RCX LNP API.  It may be possible
   to use the Linux one compiled under cygwin?
-  Better support for dsound, so that we get more then just a beep
-  investigate breaking emuLegOs.h into files with the same names as
   their LegOs counterparts



BUGS:

-  it has been reported that in legOS that if the main thread returns child threads continue.
   This behavior needs to be replicated in emuLegOS
-  potential bug in windows port.  It has been reported that on Windows XP the kill doesn't
   function.  This could be a windows or windows XP related issue?  I'll have to setup
   cygwin on a windows box and try it out.
