##########################################################################
# User configuration and firmware specific object files	
##########################################################################

OBJS =

OBJS += app_interfaces.o
OBJS += APP/SWD/SWD.o
OBJS += APP/JTAG/JTAG_TAP.o
OBJS += APP/GPIO/GPIO.o
OBJS += APP/PowerExt/PowerExt.o


LIBNAME=interfaces

##########################################################################
# GNU GCC compiler flags
##########################################################################
ROOT_PATH?= ..
INCLUDE_PATHS = -I$(ROOT_PATH) -I../core -I. -I ./APP 

include $(ROOT_PATH)/Makefile.inc

##########################################################################
# Actual work
##########################################################################

include $(ROOT_PATH)/Makefile.util
