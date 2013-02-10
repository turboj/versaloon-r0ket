##########################################################################
# User configuration and firmware specific object files	
##########################################################################

OBJS =

OBJS += usbconfig.o
OBJS += usbhid.o
OBJS += usbmsc.o

LIBNAME=usb

##########################################################################
# GNU GCC compiler flags
##########################################################################
ROOT_PATH?= ..
INCLUDE_PATHS = -I$(ROOT_PATH) -I../core -I.

include $(ROOT_PATH)/Makefile.inc

##########################################################################
# Actual work
##########################################################################

include $(ROOT_PATH)/Makefile.util
