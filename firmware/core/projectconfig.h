#ifndef __PROJECTCONFIG_H_
#define __PROJECTCONFIG_H_

#include "lpc134x.h"
#include "sysdefs.h"



/*=========================================================================
    CORE CPU SETTINGS
    -----------------------------------------------------------------------

    CFG_CPU_CCLK    Value is for reference only.  'core/cpu/cpu.c' must
                    be modified to change the clock speed, but the value
                    should be indicated here since CFG_CPU_CCLK is used by
                    other peripherals to determine timing.

    -----------------------------------------------------------------------*/
      #define CFG_CPU_CCLK                (72000000)  // 1 tick = 13.88nS
/*=========================================================================*/


/*=========================================================================
    SYSTICK TIMER
    -----------------------------------------------------------------------

    CFG_SYSTICK_DELAY_IN_MS   The number of milliseconds between each tick
                              of the systick timer.

    -----------------------------------------------------------------------*/
      #define CFG_SYSTICK_DELAY_IN_MS     (1)
/*=========================================================================*/


/*=========================================================================
    SSP
    -----------------------------------------------------------------------

    CFG_SSP0_SCKPIN_2_11      Indicates which pin should be used for SCK0
    CFG_SSP0_SCKPIN_0_6

    -----------------------------------------------------------------------*/
      #define CFG_SSP0_SCKPIN_2_11
/*=========================================================================*/

/*=========================================================================
    ON-BOARD LED
    -----------------------------------------------------------------------

    CFG_LED_PORT              The port for the on board LED
    CFG_LED_PIN               The pin for the on board LED
    CFG_LED_ON                The pin state to turn the LED on (0 = low, 1 = high)
    CFG_LED_OFF               The pin state to turn the LED off (0 = low, 1 = high)

    -----------------------------------------------------------------------*/
      #define CFG_LED_PORT                (1)
      #define CFG_LED_PIN                 (11)
      #define CFG_LED_ON                  (1)
      #define CFG_LED_OFF                 (0)

/*=========================================================================*/

/*=========================================================================
    SD CARD
    -----------------------------------------------------------------------

    CFG_HAVE_SDCARD           Indicates whether the SD Card code should be
                              included in the filesystem access code.
                              If the SD Card is enabled, the number of volumes
                              in filesystem/ffconf.h must be increased accordingly.

    -----------------------------------------------------------------------*/
      #define CFG_HAVE_SDCARD             (0)
/*=========================================================================*/

      #define CFG_USBMSC                 (0)

    //#define CFG_USB_VID                   (0x239A)
    //#define CFG_USB_PID                   (0x1002)
    #define CFG_USB_VID                   (0x0483)
    #define CFG_USB_PID                   (0xA038)
  

    #define CFG_USBCDC                  (1)
    #define CFG_USBCDC_BAUDRATE         (115200)
    #define CFG_USBCDC_INITTIMEOUT      (5000)
    #define CFG_USBCDC_BUFFERSIZE       (256)


/* you will need these for the UART */
#if 0
#define CFG_INTERFACE_UART (1)
#define CFG_UART_BAUDRATE           (115200)
#define CFG_UART_BUFSIZE            (512)
#endif

#ifdef USBSERIAL
#undef CFG_USBCDC
#undef CFG_USBMSC
#define CFG_USBCDC                 (1)
#define CFG_USBMSC                 (0)
#endif

#endif
