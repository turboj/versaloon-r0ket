##########################################################################
# User configuration and firmware specific object files	
##########################################################################

OBJS =

OBJS += CommandProcessor.o
OBJS += USB_TO_SWD.o
OBJS += USB_TO_JTAG_RAW.o
OBJS += USB_TO_JTAG_HL.o
OBJS += USB_TO_JTAG_LL.o
OBJS += USB_TO_GPIO.o
OBJS += USB_TO_POWER.o
OBJS += USB_TO_XXX.o

LIBNAME=usbtoxxx

##########################################################################
# GNU GCC compiler flags
##########################################################################
ROOT_PATH?= ..
INCLUDE_PATHS = -I$(ROOT_PATH) -I../core -I. -I ../Interfaces -I ../Interfaces/APP

include $(ROOT_PATH)/Makefile.inc

##########################################################################
# Actual work
##########################################################################

include $(ROOT_PATH)/Makefile.util
