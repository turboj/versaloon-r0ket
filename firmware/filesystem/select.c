#include <string.h>
#include "sysdefs.h"
#include "lcd/lcd.h"
#include "lcd/fonts/smallfonts.h"
#include "lcd/print.h"
#include "filesystem/ff.h"
#include "basic/basic.h"

#define FLEN 13

/* if count is 0xff (-1) do not fill files and return the count instead */
int getFiles(char files[][FLEN], uint8_t count, uint16_t skip, const char *ext)
{
    DIR dir;                /* Directory object */
    FILINFO Finfo;
    FRESULT res;
    int pos = 0;
    int extlen = strlen(ext);
    res = f_opendir(&dir, "0:");
    if(res){
        //lcdPrint("OpenDir:"); lcdPrintln(f_get_rc_string(res)); lcdRefresh(); 
        return 0;
    };
    while(f_readdir(&dir, &Finfo) == FR_OK && Finfo.fname[0]){
        int len=strlen(Finfo.fname);

        if(len<extlen)
            continue;

        if( strcmp(Finfo.fname+len-extlen, ext) != 0)
            continue;

        if (Finfo.fattrib & AM_DIR)
            continue;

        if( skip>0 ){
            skip--;
            continue;
        };

        if(count != 0xff)
            strcpy(files[pos],Finfo.fname);
        pos++;
        if( pos == count )
            break;
    }
    return pos;
}

#define PERPAGE 7
int selectFile(char *filename, const char *extension)
{
    int skip = 0;
    char key;
    int selected = 0;
    int file_count = getFiles(NULL, 0xff, 0, extension);

    font=&Font_7x8;
    if(!file_count){
        lcdPrintln("No Files?");
        lcdRefresh();
        getInputWait();
        getInputWaitRelease();
        return -1;
    };
    while(1){
        char files[PERPAGE][FLEN];
        int count = getFiles(files, PERPAGE, skip, extension);

        if(count<PERPAGE && selected==count){
            skip--;
            continue;
        };
        
        redraw:
        lcdClear();
        lcdPrintln("Select file:");
        for(int i=0; i<count; i++){
            if( selected == i )
                lcdPrint("*");
            else
                lcdPrint(" ");
            lcdSetCrsrX(14);
            int dot=-1;
            for(int j=0;files[j];j++)
                if(files[i][j]=='.'){
                    files[i][j]=0;
                    dot=j;
                    break;
                };
            lcdPrintln(files[i]);
            if(dot>0)
                files[i][dot]='.';
        }
        lcdRefresh();
        key=getInputWaitRepeat();
        switch(key){
            case BTN_DOWN:
                if( selected < count-1 ){
                    selected++;
                    goto redraw;
                }else{
                    if(skip == file_count - PERPAGE) { // wrap to top
                        selected = 0;
                        skip = 0;
                    } else 
                        skip++;
                }
                break;
            case BTN_UP:
                if( selected > 0 ){
                    selected--;
                    goto redraw;
                }else{
                    if( skip > 0 ){
                        skip--;
                    } else { // wrap to bottom
                        skip = file_count - PERPAGE;
                        if(skip < 0) skip = 0;
                        selected = file_count - skip - 1;
                    }
                }
                break;
            case BTN_LEFT:
                getInputWaitRelease();
                return -1;
            case BTN_ENTER:
            case BTN_RIGHT:
                strcpy(filename, files[selected]);
                getInputWaitRelease();
                return 0;
        }
    }
}
