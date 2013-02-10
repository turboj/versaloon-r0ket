#include "lpc134x.h"
#include "sysdefs.h"
#include "basic/basic.h"
#if CFG_USBMSC
#include "usb/usbmsc.h"
#endif
/* Missing from core */

#define set_MSP(x) \
	__asm(  "msr   MSP, %0\n" : : "r" (x) )

/* Jump back into bootcode, enable mass storage firmware flashing */

void ReinvokeISP(void) {
	uint32_t command[5]; // 
	command[0] = 57;

	/* Disable SYSTICK timer and interrupt before calling into ISP */
	SYSTICK_STCTRL &= ~(SYSTICK_STCTRL_ENABLE | SYSTICK_STCTRL_TICKINT);
	/* make sure USB clock is turned on before calling ISP */
	SCB_SYSAHBCLKCTRL |= 0x04000;
	/* make sure 32-bit Timer 1 is turned on before calling ISP */
	SCB_SYSAHBCLKCTRL |= 0x00400;
	/* make sure GPIO clock is turned on before calling ISP */
	SCB_SYSAHBCLKCTRL |= 0x00040;
	/* make sure IO configuration clock is turned on before calling ISP */
	SCB_SYSAHBCLKCTRL |= 0x10000;
	/* make sure AHB clock divider is 1:1 */
	SCB_SYSAHBCLKDIV = 1;

	/* as per 19.2.1 of lpc13xx user manual, clear this byte of ram
	   to avoid long delay before USB becomes active */
	*((uint32_t *)(0x10000054)) = 0x0;

	/* Set stack pointer to ROM value (reset default) This must be the last
	   piece of code executed before calling ISP, because most C expressions
	   and function returns will fail after the stack pointer is changed. */
	set_MSP(*(int*)0x1FFF0000);

	/* Enter ISP. We call "iap_entry" to enter ISP because the ISP entry is done
	   through the same command interface as IAP. */
	iap_entry(command, NULL);

	/* Alternative shorter direct assembler way: */
	/*
	__asm(
			"ldr  r0, =%0\n"
			"ldr  lr, =0x1fff1ff1\n"
			: : "i" (&command)
			);
	 */

	/* NOTREACHED */
}

void EnableWatchdog(uint32_t ms){
	/* Enable WDT clock */
	SCB_SYSAHBCLKCTRL |= 0x8000;

	SCB_PDRUNCFG &= ~(1);
	/* select internal RC */
	SCB_WDTCLKSEL = SCB_WDTCLKSEL_SOURCE_INTERNALOSC;
	/* enable */
	SCB_WDTCLKUEN = 0;
	SCB_WDTCLKUEN = 1;

#define INTERNAL_RC_HZ 12000000
	WDT_WDTC= ((INTERNAL_RC_HZ/4)/1000) *ms;

	SCB_WDTCLKDIV = 1;
	WDT_WDMOD =  WDT_WDMOD_WDRESET_ENABLED | WDT_WDMOD_WDEN_ENABLED;

	// frob watchdog once.
	WDT_WDFEED = WDT_WDFEED_FEED1;
	WDT_WDFEED = WDT_WDFEED_FEED2;
};

void ISPandReset(void){
#if CFG_USBMSC
    if(usbMSCenabled&USB_MSC_ENABLEFLAG){
        usbMSCOff();
        delayms(500);
    };
#endif
#if CFG_USBCDC
    if(1){
        USB_Connect(FALSE);
        delayms(500);
    };
#endif
    EnableWatchdog(1000*15);
    ReinvokeISP();
};

