#you shouldn't need to make any modification here, must modify common.mk

##
## legOS - the independent LEGO Mindstorms OS
## Makefile - allows you to keep the upper hand
## (c) 1998, 1999 by Markus L. Noga <markus@noga.de>    
##

# name for the legOS kernel
KERNEL=$(LEGOS_ROOT)/boot/legOS

include common.mk
PROGRAM=$(target).lx


DOBJECTS = $(CSRC:.c=.o)


all: $(PROGRAM) emu

include $(LEGOS_ROOT)/Makefile.common
include $(LEGOS_ROOT)/Makefile.user


# doc/html subdirectory
html:
	doxygen Doxyfile

depend:
	$(MAKEDEPEND) *.c > .depend

.depend:
	$(MAKEDEPEND) *.c > .depend

tag:
	ctags --format=1 *.c include/*.h include/*/*.h *.c

clean:	emuclean
	rm -rf .depend* *.o *.dis *.dis2 *~ *.bak *.tgz *.s tags *.ds1 *.ds2 *.dmap *.dcoff *.srec *.map *.coff

realclean: clean
	rm -f *.lx *.o

load:
	$(LNP_ROOT)/applications/dll -p$(PNUM) $(PROGRAM)

bload:
	$(LEGOS_ROOT)/util/dll -p$(PNUM) $(PROGRAM)

emu:
	gmake -f emu.mk

emuclean:
	gmake -f emu.mk clean

emurealclean:
	gmake -f emu.mk realclean
# depencencies
#
ifndef NODEPS
include .depend
endif
# DO NOT DELETE

