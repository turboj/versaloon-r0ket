#include <sysinit.h>

#include "core/cpu/cpu.h"
#include "core/pmu/pmu.h"

#include "basic/basic.h"
#include "basic/config.h"
#include "basic/idle.h"
#include "lcd/render.h"
#include "lcd/print.h"
#include "lcd/lcd.h"
#include "lcd/fonts/orbitron14.h"
#include "usbcdc/usb.h"
#include "usbcdc/usbcore.h"
#include "usbcdc/usbhw.h"
#include "usbcdc/cdcuser.h"
#include "usbcdc/util.h"
#include "usbcdc/versausb.h"


#include "Interfaces/app_cfg.h"

//#include "Interfaces/app_interfaces.h"

#include "USB_TO_XXX/CommandProcessor.h"

extern void USB_TO_XXX_Init(uint8_t *poll_buff);

#ifndef CRP_VALUE
#define CRP_VALUE 0x0  // ANY non-magic value disables CRP
#endif

__attribute__ ((used, section("crp"))) const uint32_t the_crp=CRP_VALUE;

/**************************************************************************/

void wrapper(void);

volatile extern uint8_t  USB_Configuration;
volatile uint32_t rep_len = 0;
volatile uint32_t count_out = 0;
volatile uint32_t cmd_len = 0;
static uint32_t pkt_ctr=0;
static uint32_t pktSecond=0;



uint8_t  lcdBuffer[USB_DATA_BUFF_SIZE];
uint8_t asyn_rx_buf[ASYN_DATA_BUFF_SIZE];

uint8_t * const buffer_out = &lcdBuffer;



void tick_default(void) {
    incTimer();
}

void tick_wrapper(void){
  tick_default();
}

static uint8_t cursec=0;

static void process_USB(void) {
  uint32_t pkg_len=0;
  
  if (getSeconds() &1) {
    if (!cursec) {
      cursec=1;
      gpioSetValue(RB_LED0,1);
       
    }
  } else {
    if (cursec) {
    gpioSetValue(RB_LED0,0);

    }
  }

  if ((getSeconds()- pktSecond)>10) {
	  cmd_len=0;count_out=0;
	  pktSecond=getSeconds();
  }
  
  if (!count_out) gpioSetValue(RB_LED2,0);
  
  if (!USB_Configuration) return; // no USB
  if (usbVSHasData()) {
     
    pkg_len=usbVSGetBytes(buffer_out+count_out);
   
  }
  if(pkg_len)
	{
          gpioSetValue(RB_LED2,1);
	  pktSecond=getSeconds();
		if(!count_out)
		{
			// first package
			if(buffer_out[0] <= VERSALOON_COMMON_CMD_END)
			{
				// Common Commands
				if(buffer_out[0] == VERSALOON_WRITE_OFFLINE_DATA)
				{
					cmd_len = buffer_out[1] + ((uint16_t)buffer_out[2] << 8) + 7;
				}
				else
				{
					cmd_len = pkg_len;
				}
			}

			else if(buffer_out[0] <= VERSALOON_USB_TO_XXX_CMD_END)
			{
				// USB_TO_XXX Support
				cmd_len = buffer_out[1] + ((uint16_t)buffer_out[2] << 8);
			}

		}
		count_out += pkg_len;
		


		// all data received?
		pkg_len = cmd_len;
		if(count_out >= pkg_len)
		{
			cmd_len |= 0x80000000;
                  pkt_ctr++;
		}
	}
  
  if(cmd_len & 0x80000000)
	{
		// A valid USB package has been received
		LED_USB_ON();
		
		ProcessCommand(&buffer_out[0], cmd_len & 0xFFFF);
		if(rep_len > 0)
		{
			// indicate reply data is valid
			rep_len |= 0x80000000;
		}
		else
		{
			// no data to send, set cmd_len to 0
			cmd_len = 0;
		}
		count_out = 0;				// set USB receive pointer to 0
              if(rep_len & 0x80000000)	// there is valid data to be sent to PC
		{
                  // send command
                  
                  int len=rep_len & 0xFFFF;
                  uint32_t ofs=0;
                  int  diff=0;
                  while (len>0) {
                    diff=(len>64 ? 64:len);
                    
                    while (!isVSWriteAvail()) ;
                    usbVSWriteBytes(buffer_out + ofs, diff);
                    ofs+=diff;
                    len -=diff;
                  }
                  if (diff==64) {
                    usbVSWriteBytes(buffer_out + ofs, 0); // Zero Packet
                  }
                  
            // reset command length and reply length for next command
			
		}
                cmd_len = 0;
                rep_len = 0;
		
		LED_USB_OFF();
	}
}


extern uint16_t max_char_buffer;


void main(void) {
    // Configure cpu and mandatory peripherals
    cpuInit();                                // Configure the CPU
// we do it later
//    systickInit(CFG_SYSTICK_DELAY_IN_MS);     // Start systick timer
// cpuInit already calls this
//    gpioInit();                               // Enable GPIO 
    pmuInit();                                // Configure power management
    adcInit();                                // Config adc pins to save power

    // initialise basic badge functions
    rbInit();

    initUUID(); // Cache UUID values.

  
    lcdInit(); // display
    GLOBAL(daytrig)=200;
    GLOBAL(lcdbacklight)=200;
    systickInit(SYSTICKSPEED);

    lcdFill(0);
    lcdDisplay();
    
    switch(getInputRaw()){
        case BTN_ENTER:
            lcdPrint("ISP active");
            lcdRefresh();
            ReinvokeISP();
            break;
        case BTN_DOWN:
            lcdPrint("MSC not avail!");
            lcdRefresh();
            //usbMSCInit();
            while(1);
            break;
    };
    lcdPrintln("XXX init...");
    lcdDisplay();
    USB_TO_XXX_Init(asyn_rx_buf);
    lcdPrintln("USB init...");
    lcdDisplay();
    usbCDCInit();
    delayms(500);
    lcdClear();
    setIntFont(&Font_Orbitron14pt);
    lcdPrintln("Turbo  J");
    setIntFont(&Font_3x6);
    lcdPrintln("");
    lcdPrintln("P R O U D L Y  P R E S E N T S");
    lcdPrintln("");
    setIntFont(&Font_8x8Thin);
    lcdPrintln(" Versaloon");
    setIntFont(&Font_3x6);
    lcdPrintln("");
    setIntFont(&Font_8x8Thin);
    lcdPrintln("   R0ket");
    //lcdPrintInt(max_char_buffer);
    setIntFont(&Font_7x8);
    lcdRefresh();

    gpioSetValue(RB_LED0,1);
    while (1){
/*
      Note: Display is not available during JTAG operation, as MOSI/MISO are
      reassigned as GPIO
    */ 
        process_USB();
       
    }
}

int getrelease(void){
    return 0x0000010f;
};
