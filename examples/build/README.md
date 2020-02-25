This sample has the same source code as the rover sample, but it is a better build process.  Also it handles building projects which contain multiple .c files.

This build process will by default build a emulated binary and a .lx file at the same time.  This helps keep your code honest so that you don't use functions not available on both platforms.  Of course you can still add emulated specific debugging code by including it in "#ifdef EMULEGOS" blocks.

To use this for your own project copy Makefile, common.mk, and emu.mk into the directory with your .c files.  Then modify common.mk appropriatly.  The other files should not need changes.

The following build targets are available.

make - this will build an x86 and .lx version of your code
make all - same as above, this is the default target
make emu - just build the emulated robot, not the .lx
make load - use dll to load program to RCX using LNPD
make bload - use dll to load program to RCX withought LNPD
make clean - will erase all genearted files exception for .lx
make realclean - same as above but includes .lx
