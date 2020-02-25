1.2.5.1 03/01/03 Mark Falco
	- fixed namespace issues for use with the current version of gcc
	- added include for string.h
	- modified third party sensor drivers to match those submitted as patches to legOS
	- fixed yield to call pthread_test_cancel as it was supposed to be doing
	- added examples/build which shows how to build x86 and .lx at the same time
	- fixed yield call to call Sleep(0) on windows, and pthread_yield on
	  Unix, thanks to James Wilkinson for his help in debugging this
	
1.2.5.0 03/02/02 Mark Falco
	- added include for semaphore.h so user doesn't need to do it
	- added yield call
	- fixed light scaling to match legOS 0.2.5
	- added velocity code, this calculation is simpler then LegOS
		may need some work
	- added support for various third party sensors and multiplexors
		- swmux from www.techno-stuff.com
		- dirpd from www.techno-stuff.com
		- mux (active multiplexor) from www.mindsensors.com
		  this emulates the legOS driver I wrote for this 
		  multiplexor, availble in the patches section of
		  the legOS project at sourceforge
	- various other bug fixes which I forgot to write down
	- fixed lnp patch, it was a reverse patch before

1.2.4.2 08/25/01 Mark Falco
	- added windows support, cygwin is required
	- added callback handlers on motor updaters in rcx.tcl


1.2.4.1 08/21/01 Mark Falco
	- fixed run button shutdown when not using lnp

1.2.4.0 08/18/01 Mark Falco
	- removed direct ir stuff and replaced it with LNP.
	- added support most everything in the legos command reference

08/15/01 Mark Falco
	in the proccess of adding support for legos 0.2.x
	added multiplexor touch sensor support
	added dualirprox detector sensor support

5/17/99	Mark Falco
	new legOS functions: dir_read, dir_write, dir_flush, cputw
	modified functions: msleep - now calls usleep and Sleep on unix
	                    and windows respecitively.
			    wait_event - calls msleep(1) between each check,
			    old code was maxing out CPU
	new callbacks:	    setCallbackTxIr - called when dir_write is called
	                    setCallbackRxIr - called when dir_read is called
			    setCallbackFlushIr - called when dir_flush is
			    called
			    setCallbackUpdateExecStatus called when rcx_main 
			    is called, and when it exits
	new tools:	    roboroom.tcl - a chat room to connect multiple ir 
	                    clients
			    guiclient.tk - a ir client which allows the user
			    to send to all clients in the room
			    irclient.tcl - an ir client which connects to the
			    real ir tower so that your emu robot(s) can talk
			    to real robots
