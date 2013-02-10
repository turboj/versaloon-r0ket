
#ifndef __versausb_h
#define __versausb_h

// Versaloon USB functions

extern int usbVSHasData();
extern uint32_t usbVSGetBytes(uint8_t * buf) ;
extern int isVSWriteAvail(void) ;
extern void usbVSWriteBytes(uint8_t * buf, int nbytes);

#endif
