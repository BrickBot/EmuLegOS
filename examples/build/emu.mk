#you shouldn't need to make any modification here, must modify common.mk

include common.mk

CSRC += $(MAINSRC)

include ${EMULEGOS_ROOT}/makefile

