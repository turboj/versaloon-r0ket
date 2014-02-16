#include <sysinit.h>
#include <string.h>

#include "usbcdc/usb.h"
#include "usbcdc/usbcore.h"
#include "usbcdc/usbreg.h"
#include "usbcdc/cdc_buf.h"
#include "usbcdc/usbhw.h"
#include "usbcdc/cdcuser.h"


extern void WrCmd (uint32_t cmd);
extern uint32_t RdCmdDat (uint32_t cmd);

int usbVSHasData() {
  
  NVIC_DisableIRQ(USB_IRQn);
  WrCmd(CMD_SEL_EP_CLRI(6));
  uint32_t stat=RdCmdDat(DAT_SEL_EP_CLRI(6));
  NVIC_EnableIRQ(USB_IRQn);
  
  
  return stat &1;
}

uint32_t usbVSGetBytes(uint8_t * buf) {
  NVIC_DisableIRQ(USB_IRQn);
  uint32_t len=USB_ReadEP(VERSALOON_EP_OUT, buf);
  NVIC_EnableIRQ(USB_IRQn);
  return len;
  
}

int isVSWriteAvail(void) {
   NVIC_DisableIRQ(USB_IRQn);
  WrCmd(CMD_SEL_EP_CLRI(5));
   uint32_t stat=RdCmdDat(DAT_SEL_EP_CLRI(5));
   NVIC_EnableIRQ(USB_IRQn);
   return !(stat & 1) ;
}

void usbVSWriteBytes(uint8_t * buf, int nbytes) {
  NVIC_DisableIRQ(USB_IRQn);
  USB_WriteEP (VERSALOON_EP_IN,buf, nbytes);
  NVIC_EnableIRQ(USB_IRQn);
}
