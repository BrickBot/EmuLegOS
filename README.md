EMULEGOS
========
An emulator for the LegOS operating system used on the Lego RCX.

Project Status: Tracking LegOS 0.2.5
	 - Currently all features from the legOS command reference
		http://legos.sourceforge.net/docs/CommandRef.html
	 - Additionally there are many other features from legOS that
		are supported.

LegOS code modifications:
------------------------
You will need to make a few small modifications to your existing legos program to make it compiler under either legos or emulegos.
1. add code to include emuLegOs.h when EMULEGOS is defined, and to not include the normal legos headers.

	   #ifdef EMULEGOS
	   #  include "emuLegOs.h"
	   #else
	   #  include <conio.h>
	   #  include <stdlib.h>
	   #  include <dsensor.h>
	   #endif

2. redefine the "main" functions as "rcx_main" when building under emulegos

	   #ifdef EMULEGOS
	   #  define RCXMAIN rcx_main
	   #else
	   #  define RCXMAIN main
	   #endif

	   int RCXMAIN(int argc, char **argv) {
		...

	See the sample rover program in the examples directory

Building your emulated robot:
----------------------------
1. Set the following environment variables
	     EMULEGOS_ROOT - root of the emulegos tree
	     LNP_ROOT - root of the LNP tree, if using LNP (see LNP note below)
2. Create project makefile, you can use examples/rover/Makefile
	   as a template.  You may define emu_params in your makefile to
	   a set of command line paramters to always pass to your emulated
	   program, this is usefull for setting the sensor types.
3. run make and you will get the following
   1. in the EMULEGOS_ROOT directory a bunch of \*_emu.o files which make up the emulator
   2. in the build directory a bunch of \*_emu.o files which correspond to your source files.
   3. a binary with your project name and an _emu extention
   4. a shell script which will run your binary and pass in the
	required and supplied command line parameters
	I find it usefull to combine my legos project makefile with the
	makefile for the emulated code.  This way whenever you run make
	you get both a emulated version and the .lx version.  This also
	keeps you from accidently using functions which are not available
	under legos.  Check out examples/build for a build process which
	does this.

Partially implemented features:
------------------------------
Some features of legos are not fully implemented, but still have
limited support in the emulator.  For instance many of the lcd
functions which control the different sections like dots, and the
little man can be set but do not actually do anything.  Sound
support is also limited.  The entire dsound API is there, but
if you attempt to play any sound all you will hear is a beep, all
the timing and logic should be correct, just the ouput sound is not.

LNP note:
--------
LNP support for emulegos is complete.  It was quite easy to produce
as the LNP host library is basically identical to the RCX version.
The emulegos code adds thread safety so that you can use lnp from
multiple tasks just as you can on the RCX.  Currently EmuLegOs
recompiles the LNP library and links the .o files statically into
the emulator.

Patches to LNP:
--------------
In order for LNP to work with EmuLegOs you will need to apply two
patches to LNP.  The first is to stub.c which allows it to compile
cleanly as C++ code under gcc.  The second is a modification to
the lnpd dameon.c which make it possible for lnp pc clients to
communicate.  Both files will not negitively impact normal LNP
operations.  The patches and instructions on how to apply them can
be found in the lnp_patch directory under the EmuLegOs root dir.
	   
LNP support for EmuLegos is optional and can be removed by modifing
the EMU_LNP_SUPPORT variable in the main emulegos makefile.

Currently there is no LNP support for the emulator under windows
So EMU_LNP_SUPPORT is off by default on Windows machines.  See the
TODO file for a bit more information on adding Windows LNP support.


Windows Support:
---------------
Windows support is provided via cygwin which is available from
http://sources.redhat.com/cygwin
After installing cygwin you may need to modify the CYGWIN_ROOT
variable in the EmuLegOS makefile if you installed cygwin
somewhere other then c:/cygwin.  When setting the EMULEGOS_ROOT
variable under cygwin it should start with a drive letter and not
/cygdrive, so you would want "c:/emulegos" rather than
"/cygdrive/c/emulegos".  On my system I needed to surround the
value in double quotes to keep cygwin from silently replacing
the value with the /cygdrive stuff.


Command Line Parameters:
-----------------------
Here is a list of the command line parameters which emulegos programs recognize.

	[-sen1 val]  	specifies the type of sensor attached to port 1
	[-sen1a val] 		if using a active multiplexors this is for sub port
			a on the mux
	[-sen1b val]
	[-sen1c val]
	[-sen2(a,b,c) val] same as above, -sen2, -sen2a, ...
	[-sen3(a,b,c) val]
	[-ir]    	enables LNP
	[-port val] 	if LNP is enabled specifies the port for LNPD if
			not running on the default port, defaults to 7776 the LNPD default
	[-id val] 	if LNP is enabled specifies the LNPID for this robot,
			defaults to 8. NOTE: values are in decimal
	[-host val]	if LNP is enabled specifies the hostname of the
			machine running LNPD, default to localhost
	[-source val] 	specifies an additional Tcl script to load along
			with the RCX interface and API
	[-sensors]  	specifiying this option will give you a list
			of supported sensor types, i.e. what you can pass into
			-sen1, -sen2, ...
	[-bg]		allows you to change the background robot color, default is yellow

valid sensor types recognized by the -sen parameters are:

        raw
        touch
        swmux - www.techno-stuff.com (switch multiplexor)
        mux - www.mindsensors.com (active multiplexor)
        light
        rotation
        dirpd - www.techno-stuff.com
        velocity - rotation senor with velocity driver

GUI Notes:
----------
Here are some features of the GUI which may not be obvious:
1. Right clicking on any touch sensor or button will keep it depressed, left clicking will cause it to pop back up
2. clicking on the sensor type will bring up a drop down so you can change it at run time this is not particuallry usefull since it's easier to specify the type on the command line, but it allows you to switch to see the raw value easily.
3. if you type a value into the raw sensor window then you must hit enter for it to be set
4. the velocity sensor can be bound to a motor port so that the speed and direction will vary with the direction and power of the motor
5. the velocity sensor can have a polarity set so that it works in reverse.
6. clicking the "off" button exit the emulator, but I guess you've probably already noticed this
