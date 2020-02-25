#this file should be included by a project makefile which defines target and adds the
#source files to either CPPSRC or CSRC, then do a make on that makefile
#you must have EMULEGOS_ROOT defined in order to build
#on windows EMULEGOS_ROOT needs to be defined starting with the drive
#letter and not with /cygwin/c/emulegos or tcl won't handle it properly
#it can however have unix slashes so "c:/emulegos" is ok.  Be sure to verify
#that cygwin don't do fancy subsition no this value when you set it, on my
#system I needed to put the value in quotes, i.e.
#export EMULEGOS_ROOT="c:/emulegos"

#WINDOWS NOTE: on windows cygwin is required

CXX =	g++

#if you want to use LNP then this must be set to 1
#lnp functionality will be available, generally you would always want
#to leave this on, but you may need to turn it off if you don't have
#lnp on  your system, or are using a OS where it isn't available
#when changing this variable be sure to do a clean build
#the default value is OS dependent and is set below
EMU_LNP_SUPPORT := 


#OS dependent stuff
ifeq ($(OSTYPE), cygwin)
  #Windows, using cygwin
  #by default no LNP support on windows
  ifndef EMU_LNP_SUPPORT
    EMU_LNP_SUPPORT := 0
  endif

  CYGWIN_ROOT := c:/cygwin
  CYG_TCL_LIBRARY := $(CYGWIN_ROOT)/usr/share/tcl8.0
  CYG_TK_LIBRARY := $(CYGWIN_ROOT)/usr/share/tk8.0
  INCLS += -I/usr/include -I$(EMULEGOS_ROOT)/win
  LIBS += -L${CYGWIN_ROOT}/lib
  TCL_LIBS := -ltcl80 -ltk80
  EXE_EXT := .exe
else
  #not windows, so some form of Unix
  ifndef EMU_LNP_SUPPORT
    EMU_LNP_SUPPORT := 1
  endif

  LIBS += -L/usr/X11R6/lib -lX11 -ldl -lpthread
  TCL_LIBS := -ltcl -ltk
  EXE_EXT :=
endif

OPTS = 	-g -Wall -D_REENTRANT -DEMULEGOS -D_GNU_SOURCE \
	-DEMU_LNP_SUPPORT=$(EMU_LNP_SUPPORT)
INCLS += -I${PWD} -I${EMULEGOS_ROOT} -I${LNP_ROOT}/liblnp
LIBS += -L/lib ${TCL_LIBS} -lm


CPPSRC += \
	${EMULEGOS_ROOT}/Main.cpp \
	${EMULEGOS_ROOT}/RealWorld.cpp \
	${EMULEGOS_ROOT}/emuLegOs.cpp \
	${EMULEGOS_ROOT}/tkAppInit.cpp

CSRC +=  \
	${EMULEGOS_ROOT}/swmux.c

ifeq ($(EMU_LNP_SUPPORT), 1)
#lnp support compiled in	
CSRC += \
	${LNP_ROOT}/liblnp/stub.c
endif


CPPOBJS =$(CPPSRC:.cpp=_emu.o)
COBJS = $(CSRC:.c=_emu.o)

OBJS = $(COBJS) $(CPPOBJS)



default: $(target) $(target)_emu$(EXE_EXT)


$(target):
	echo export TCL_LIBRARY=$(CYG_TCL_LIBRARY) > $(target)
	echo export TK_LIBRARY=$(CYG_TK_LIBRARY) >> $(target)
	echo $(PWD)/$(target)_emu$(EXE_EXT) $$EMULEGOS_ROOT/rcx.tk $(emu_params) \$$* >> $(target)
	chmod +x $(target)


$(target)_emu$(EXE_EXT): $(OBJS)
	$(CXX) $(OPTS) $(OBJS) $(LIBS) -o $(target)_emu$(EXE_EXT)

# how to compile C++ source
%_emu.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLS) $(OPTS) -c $*.cpp -o $*_emu.o

%_emu.o: %.c
	$(CXX) $(CFLAGS) $(INCLS) $(OPTS) -c $*.c -o $*_emu.o



clean:		
		rm -f *.o $(COBJS) $(CPPOBJS) core \
		${target} ${target}_emu$(EXE_EXT)

realclean:
		rm -f ${EMULEGOS_ROOT}/*.o *.o \
		${target} ${target}_emu$(EXE_EXT) core
