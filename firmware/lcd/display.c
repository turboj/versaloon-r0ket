#include <string.h>

#include <display.h>
#include <sysdefs.h>
#include "lpc134x.h"
#include "core/ssp/ssp.h"
#include "gpio/gpio.h"
#include "basic/basic.h"
#include "basic/config.h"
#if CFG_USBMSC
#include "usb/usbmsc.h"
#endif

#define DISPLAY_N1200 0
#define DISPLAY_N1600 1

#define MODE 8 /* 8 or 16 */

#if MODE == 8
#define putpix(x) _helper_pixel8(x)
#define px_INIT_MODE 2
#define px_PACK(r,g,b) COLORPACK_RGB332(r,g,b)
#define px_type uint8_t
#else
#define putpix(x) _helper_pixel16(x)
#define px_INIT_MODE 5
#define px_PACK(r,g,b) COLORPACK_RGB565(r,g,b)
#define px_type uint16_t
#endif

/**************************************************************************/
/* Utility routines to manage nokia display */
/**************************************************************************/

uint8_t lcdBuffer[RESX*RESY_B];
uint32_t intstatus; // Caches USB interrupt state
                    // (need to disable MSC while displaying)
uint8_t displayType;

#define TYPE_CMD    0
#define TYPE_DATA   1

static void lcd_select() {
#if CFG_USBMSC
    if(usbMSCenabled){
        intstatus=USB_DEVINTEN;
        USB_DEVINTEN=0;
    };
#endif
    /* the LCD requires 9-Bit frames */
    uint32_t configReg = ( SSP_SSP0CR0_DSS_9BIT     // Data size = 9-bit
                  | SSP_SSP0CR0_FRF_SPI             // Frame format = SPI
                  | SSP_SSP0CR0_SCR_8);             // Serial clock rate = 8
    SSP_SSP0CR0 = configReg;
    gpioSetValue(RB_LCD_CS, 0);
}

static void lcd_deselect() {
    gpioSetValue(RB_LCD_CS, 1);
    /* reset the bus to 8-Bit frames that everyone else uses */
    uint32_t configReg = ( SSP_SSP0CR0_DSS_8BIT     // Data size = 8-bit
                  | SSP_SSP0CR0_FRF_SPI             // Frame format = SPI
                  | SSP_SSP0CR0_SCR_8);             // Serial clock rate = 8
    SSP_SSP0CR0 = configReg;
#if CFG_USBMSC
    if(usbMSCenabled){
        USB_DEVINTEN=intstatus;
    };
#endif
}

static void lcdWrite(uint8_t cd, uint8_t data) {
    uint16_t frame = 0x0;

    frame = cd << 8;
    frame |= data;

    while ((SSP_SSP0SR & (SSP_SSP0SR_TNF_NOTFULL | SSP_SSP0SR_BSY_BUSY)) != SSP_SSP0SR_TNF_NOTFULL);
    SSP_SSP0DR = frame;
    while ((SSP_SSP0SR & (SSP_SSP0SR_BSY_BUSY|SSP_SSP0SR_RNE_NOTEMPTY)) != SSP_SSP0SR_RNE_NOTEMPTY);
    /* clear the FIFO */
    frame = SSP_SSP0DR;
}

#define CS 2,1
#define SCK 2,11
#define SDA 0,9
#define RST 2,2

uint8_t lcdRead(uint8_t data)
{
    uint32_t op211cache=IOCON_PIO2_11;
    uint32_t op09cache=IOCON_PIO0_9;
    uint32_t dircache=GPIO_GPIO2DIR;
    IOCON_PIO2_11=IOCON_PIO2_11_FUNC_GPIO|IOCON_PIO2_11_MODE_PULLUP;
    IOCON_PIO0_9=IOCON_PIO0_9_FUNC_GPIO|IOCON_PIO0_9_MODE_PULLUP;
    gpioSetDir(SCK, 1);

    uint8_t i;

    gpioSetDir(SDA, 1);
    gpioSetValue(SCK, 0);
    gpioSetValue(CS, 0);
    delayms(1);

    gpioSetValue(SDA, 0);
    gpioSetValue(SCK, 1);
    delayms(1);
    
    for(i=0; i<8; i++){
        gpioSetValue(SCK, 0);
        delayms(1);
        if( data & 0x80 )
            gpioSetValue(SDA, 1);
        else
            gpioSetValue(SDA, 0);
        data <<= 1;
        gpioSetValue(SCK, 1);
        delayms(1);
    }
    uint8_t ret = 0;

    gpioSetDir(SDA, 0);
    for(i=0; i<8; i++){
        gpioSetValue(SCK, 0);
        delayms(1);
        ret <<= 1;
        ret |= gpioGetValue(SDA);
        gpioSetValue(SCK, 1);
        delayms(1);
    }
    gpioSetValue(SCK, 0);

    gpioSetValue(CS, 1);
    gpioSetDir(SDA, 1);
    IOCON_PIO2_11=op211cache;
    IOCON_PIO0_9=op09cache;
    GPIO_GPIO2DIR=dircache;
    delayms(1);
    return ret;
}


void lcdInit(void) {
    int id;

    sspInit(0, sspClockPolarity_Low, sspClockPhase_RisingEdge);

    gpioSetValue(RB_LCD_CS, 1);
    gpioSetValue(RB_LCD_RST, 1);

    gpioSetDir(RB_LCD_CS, gpioDirection_Output);
    gpioSetDir(RB_LCD_RST, gpioDirection_Output);

    delayms(100);
    gpioSetValue(RB_LCD_RST, 0);
    delayms(100);
    gpioSetValue(RB_LCD_RST, 1);
    delayms(100);

    id=lcdRead(220); // ID3
    
    if(id==14)
        displayType=DISPLAY_N1600;
    else /* ID3 == 48 */
        displayType=DISPLAY_N1200;
    
/* Small Nokia 1200 LCD docs:
 *           clear/ set
 *  on       0xae / 0xaf
 *  invert   0xa6 / 0xa7
 *  mirror-x 0xA0 / 0xA1
 *  mirror-y 0xc7 / 0xc8
 *
 *  0x20+x contrast (0=black - 0x2e)
 *  0x40+x offset in rows from top (-0x7f)
 *  0x80+x contrast? (0=black -0x9f?)
 *  0xd0+x black lines from top? (-0xdf?)
 *
 */
    lcd_select();

    if(displayType==DISPLAY_N1200){
    	/* Decoded:
    	 * E2: Internal reset
    	 * AF: Display on/off: DON = 1
    	 * A1: undefined?
    	 * A4: all on/normal: DAL = 0
    	 * 2F: charge pump on/off: PC = 1
    	 * B0: set y address: Y[0-3] = 0
    	 * 10: set x address (upper bits): X[6-4] = 0
    	 */
        static uint8_t initseq[]= { 0xE2,0xAF, // Display ON
                             0xA1, // Mirror-X
                             0xA4, 0x2F, 0xB0, 0x10};
        int i = 0;
        while(i<sizeof(initseq)){
            lcdWrite(TYPE_CMD,initseq[i++]);
            delayms(5); // actually only needed after the first
        }
    }else{ /* displayType==DISPLAY_N1600 */
        static uint8_t initseq_d[] = {
        		/* The controller is a PCF8833 -
                   documentation can be found online.
        		 * CMD 01: Soft-reset
        		 * CMD 11: Sleep-out
        		 * CMD 29: Display ON
        		 * CMD 03: Booster voltage ON
        		 * CMD 13: Normal display mode
        		 * CMD 3A: interface pixel format 
        		 *   DAT 02  | 02:  8 bit/pixel (3:3:2)
        		 *           | 03: 12 bit/pixel (4:4:4)
        		 *           | 05: 16 bit/pixel (5:6:5)
        		 * CMD 2A: column address set
        		 *   DAT 1    : xs
        		 *   DAT 98-2 : xe
        		 * CMD 2B: page address set
        		 *   DAT 1    : ys
        		 *   DAT 70-2 : ye
        		 */
            0x01, 0x11, 0x29, 0x03, 0x13, 
            0x3A, px_INIT_MODE, 
            0x2A, 1, 98-2, 
            0x2B, 1, 70-2
        };
        uint16_t initseq_c = ~ 0x4BF; // 0b0000 0100 1011 1111
                                      // read from right to left,
                                      // commands:1, data:0
        int i = 0;
        while(i<sizeof(initseq_d)){
            lcdWrite(initseq_c&1, initseq_d[i++]);
            initseq_c = initseq_c >> 1;
        }
    }
    lcd_deselect();
}

void lcdFill(char f){
    memset(lcdBuffer,f,RESX*RESY_B);
#if 0
    int x;
    for(x=0;x<RESX*RESY_B;x++) {
        lcdBuffer[x]=f;
    }
#endif
};

void lcdSetPixel(char x, char y, bool f){
    if (x<0 || x> RESX || y<0 || y > RESY)
        return;
    char y_byte = (RESY-(y+1)) / 8;
    char y_off = (RESY-(y+1)) % 8;
    char byte = lcdBuffer[y_byte*RESX+(RESX-(x+1))];
    if (f) {
        byte |= (1 << y_off);
    } else {
        byte &= ~(1 << y_off);
    }
    lcdBuffer[y_byte*RESX+(RESX-(x+1))] = byte;
}

bool lcdGetPixel(char x, char y){
    char y_byte = (RESY-(y+1)) / 8;
    char y_off = (RESY-(y+1)) % 8;
    char byte = lcdBuffer[y_byte*RESX+(RESX-(x+1))];
    return byte & (1 << y_off);
}

// Color display helper functions
static inline void _helper_pixel8(uint8_t color1){
    lcdWrite(TYPE_DATA, color1);
}

static void _helper_pixel16(uint16_t color){
    lcdWrite(TYPE_DATA,color>>8);
    lcdWrite(TYPE_DATA,color&0xFF);
}

#define COLORPACK_RGB565(r,g,b) (((r&0xF8) << 8) | ((g&0xFC)<<3) | ((b&0xF8) >> 3))
#define COLORPACK_RGB444(r,g,b) ( ((r&0xF0)<<4) | (g&0xF0) | ((b&0xF0)>>4) )
#define COLORPACK_RGB332(r,g,b) ( (((r>>5)&0x7)<<5) | (((g>>5)&0x7)<<2) | ((b>>6)&0x3) )

static const px_type COLOR_FG =   px_PACK(0x00, 0x00, 0x00);
static const px_type COLOR_BG =   px_PACK(0xff, 0xff, 0xff);

void lcdDisplay(void) {
    char byte;
    lcd_select();

    if(displayType==DISPLAY_N1200){
      lcdWrite(TYPE_CMD,0xB0);
      lcdWrite(TYPE_CMD,0x10);
      lcdWrite(TYPE_CMD,0x00);
      uint16_t i,page;
      for(page=0; page<RESY_B;page++) {
          for(i=0; i<RESX; i++) {
              if (GLOBAL(lcdmirror))
                  byte=lcdBuffer[page*RESX+RESX-1-(i)];
              else
                  byte=lcdBuffer[page*RESX+(i)];
  
              if (GLOBAL(lcdinvert))
                  byte=~byte;
      
              lcdWrite(TYPE_DATA,byte);
          }
      }
    } else { /* displayType==DISPLAY_N1600 */
      uint16_t x,y;
      bool px;
 
      lcdWrite(TYPE_CMD,0x2C);
  
      for(y=RESY;y>0;y--){
          for(x=RESX;x>0;x--){
              if(GLOBAL(lcdmirror))
                  px=lcdGetPixel(RESX-x,y-1);
              else
                  px=lcdGetPixel(x-1,y-1);

	      if((!px)^(!GLOBAL(lcdinvert))) {
		      putpix(COLOR_FG); /* foreground */
	      } else {
		      putpix(COLOR_BG); /* background */
	      }
          }
      }
    };
    lcd_deselect();
}

void lcdRefresh() __attribute__ ((weak, alias ("lcdDisplay")));

inline void lcdInvert(void) {
    GLOBAL(lcdinvert)=!GLOBAL(lcdinvert);
}

void lcdSetContrast(int c) {
    lcd_select();
    if(displayType==DISPLAY_N1200){
        if(c<0x1F)
            lcdWrite(TYPE_CMD,0x80+c);
    }else{ /* displayType==DISPLAY_N1600 */
        if(c<0x40) {
            lcdWrite(TYPE_CMD,0x25);
            lcdWrite(TYPE_DATA,4*c);
        };
    }
    lcd_deselect();
};

void lcdSetInvert(int c) {
    lcd_select();
     /* it doesn't harm N1600, save space */
//  if(displayType==DISPLAY_N1200)
        lcdWrite(TYPE_CMD,(c&1)+0xa6);
    lcd_deselect();
};

/* deprecated */
void __attribute__((__deprecated__)) lcdToggleFlag(int flag) {
    if(flag==LCD_MIRRORX)
        GLOBAL(lcdmirror)=!GLOBAL(lcdmirror);
    if(flag==LCD_INVERTED)
        GLOBAL(lcdinvert)=!GLOBAL(lcdinvert);
}

void lcdShiftH(bool right, bool wrap) {
	uint8_t tmp;
	for (int yb = 0; yb<RESY_B; yb++) {
		if (right) {
			tmp = lcdBuffer[yb*RESX];
			memmove(lcdBuffer + yb*RESX,lcdBuffer + yb*RESX+1 ,RESX-1);
            lcdBuffer[yb*RESX+(RESX-1)] = wrap?tmp:0;
		} else {
			tmp = lcdBuffer[yb*RESX+(RESX-1)];
			memmove(lcdBuffer + yb*RESX+1,lcdBuffer + yb*RESX ,RESX-1);
			lcdBuffer[yb*RESX] = wrap?tmp:0;
		}
	}
}

void lcdShiftV8(bool up, bool wrap) {
	uint8_t tmp[RESX];
	if (!up) {
		if (wrap)
            memmove(tmp, lcdBuffer, RESX);
        else
            memset(tmp,0,RESX);
		memmove(lcdBuffer,lcdBuffer+RESX ,RESX*(RESY_B-1));
		memmove(lcdBuffer+RESX*(RESY_B-1),tmp,RESX);
	} else {
		if (wrap)
            memmove(tmp, lcdBuffer+RESX*(RESY_B-1), RESX);
        else
            memset(tmp,0,RESX);
		memmove(lcdBuffer+RESX,lcdBuffer ,RESX*(RESY_B-1));
		memmove(lcdBuffer,tmp,RESX);
	}
}

void lcdShiftV(bool up, bool wrap) {
	uint8_t tmp[RESX];
	if (up) {
		if (wrap) 
            memmove(tmp,lcdBuffer+((RESY_B-1)*RESX),RESX);
        else
            memset(tmp,0,RESX);
		for (int x = 0; x<RESX; x++){
			for (int y = RESY_B-1; y > 0; y--){
				lcdBuffer[x+(y*RESX)] = (lcdBuffer[x+(y*RESX)] << 1) |( lcdBuffer[x+((y-1)*RESX)] >> 7);
			}
			lcdBuffer[x] = ( lcdBuffer[x] << 1) | ((tmp[x]>>3)&1);
		}
				
	} else {
		if (wrap)
            memmove(tmp,lcdBuffer,RESX);
        else
            memset(tmp,0,RESX);
		for (int x = 0; x<RESX; x++){
			for (int y = 0; y < (RESY_B-1); y++){
				lcdBuffer[x+(y*RESX)] = (lcdBuffer[x+(y*RESX)] >> 1) |( lcdBuffer[x+((y+1)*RESX)] << 7);
			}
			lcdBuffer[x+((RESY_B-1)*RESX)] = ( lcdBuffer[x+((RESY_B-1)*RESX)] >> 1) | ((tmp[x]<<3)&8);
		}	
	}
}	

void lcdShift(int x, int y, bool wrap) {
	bool dir=true;

    if(x<0){
        dir=false;
        x=-x;
    };

    while(x-->0)
        lcdShiftH(dir, wrap);

    if(y<0){
        dir=false;
        y=-y;
    }else{
        dir=true;
    };

    while(y>=8){
        y-=8;
        lcdShiftV8(dir, wrap);
    };

    while(y-->0)
        lcdShiftV(dir, wrap);
}

