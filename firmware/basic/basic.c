
#include "basic.h"
#include "core/gpio/gpio.h"
#include "lcd/backlight.c"
#include "lcd/render.h"
#include "lcd/fonts/smallfonts.h"
#include "ecc.h"

void rbInit() {
    // TODO FIXME special port disable ? LEDs BTNs ?

    // prepare power
    // TODO FIXME more power init needed ? chrg + volt input ?
    // enable external vcc
    gpioSetDir(RB_PWR_GOOD, gpioDirection_Output);
    for (int i=0; i< 100; i++){
    	// Geiger module caps are too big
    	// and drain main caps => Brown out reset
    	// Use simple software PWM to fix this
       	for (int j=0; j<100; j++){
       		 if (j>i) gpioSetValue (RB_PWR_GOOD, 1); else
       			 gpioSetValue (RB_PWR_GOOD, 0);
       	}
    }
    gpioSetValue (RB_PWR_GOOD, 0); 

    // Disable USB Connect (we don't want USB by default)
    gpioSetDir(USB_CONNECT, gpioDirection_Output);
    gpioSetValue(USB_CONNECT, 1);

    static uint8_t ports[] = { RB_BTN0, RB_BTN1, RB_BTN2, RB_BTN3, RB_BTN4,
                        RB_LED0, RB_LED1, RB_LED2, 
                        RB_SPI_SS0, RB_SPI_SS1, RB_SPI_SS2,
                        RB_SPI_SS3, RB_SPI_SS4, RB_SPI_SS5,
                        RB_HB0, RB_HB1, RB_HB2,
                        RB_HB3, RB_HB4, RB_HB5};

    volatile uint32_t * regs[] = {&RB_BTN0_IO, &RB_BTN1_IO, &RB_BTN2_IO,
                                  &RB_BTN3_IO, &RB_BTN4_IO};

    int i = 0;
    while( i<10 ){
        gpioSetDir(ports[i], ports[i+1], gpioDirection_Input);
        gpioSetPullup(regs[i/2], gpioPullupMode_PullUp);
        i+=2;
    }

    // prepate chrg_stat
    gpioSetDir(RB_PWR_CHRG, gpioDirection_Input);
    gpioSetPullup (&RB_PWR_CHRG_IO, gpioPullupMode_PullUp);

    gpioSetDir(RB_LED3, gpioDirection_Input);
    IOCON_PIO1_11 = 0x41;

    // prepare LEDs
    IOCON_JTAG_TDI_PIO0_11 &= ~IOCON_JTAG_TDI_PIO0_11_FUNC_MASK;
    IOCON_JTAG_TDI_PIO0_11 |= IOCON_JTAG_TDI_PIO0_11_FUNC_GPIO; 

    while( i<16 ){
        gpioSetDir(ports[i],ports[i+1], gpioDirection_Output);
        gpioSetValue (ports[i], ports[i+1], 0);
        i+=2;
    }

    // Set LED3 to ?
    IOCON_PIO1_11 = 0x41;

    // prepare lcd
    // TODO FIXME more init needed ?
    gpioSetDir(RB_LCD_BL, gpioDirection_Output);
    gpioSetValue (RB_LCD_BL, 0); 

    // Set P0.0 to GPIO
    RB_PWR_LCDBL_IO&= ~RB_PWR_LCDBL_IO_FUNC_MASK;
    RB_PWR_LCDBL_IO|= RB_PWR_LCDBL_IO_FUNC_GPIO; 
    gpioSetDir(RB_PWR_LCDBL, gpioDirection_Input);
    gpioSetPullup(&RB_PWR_LCDBL_IO, gpioPullupMode_Inactive);

    // prepare I2C
    #ifdef __I2C_h
	i2cInit(I2CMASTER); // TODO FIXME via define ?
    #endif

    // prepare SPI/SS
    // TODO FIXME init miso/mosi/sck somehow ? 
    // prepare hackerbus
    while(i<sizeof(ports)){
        gpioSetDir(ports[i],ports[i+1], gpioDirection_Output);
        gpioSetValue (ports[i], ports[i+1], 1);
        i+=2;
    }

    // prepare BUSINT interrupt
    //gpioSetDir(RB_BUSINT, gpioDirection_Input);
    //gpioSetPullup (&RB_BUSINT_IO, gpioPullupMode_PullUp);
    //gpioSetInterrupt(RB_BUSINT, gpioInterruptSense_Edge, gpioInterruptEdge_Single, gpioInterruptEvent_ActiveLow);
    //gpioIntEnable(RB_BUSINT);

    //nrf_init();
    backlightInit();
    font=&Font_7x8;
}

#define WEAK_ALIAS(f) __attribute__ ((weak, alias (#f)));
void interrupt_undefined(void) {
}

void businterrupt(void)       WEAK_ALIAS(interrupt_undefined);


void PIOINT3_IRQHandler(void) {    
    if (gpioIntStatus(RB_BUSINT)) {
        gpioIntClear(RB_BUSINT);
        businterrupt();
    }
}


void notimplemented(void)
{
    while(1);
}
