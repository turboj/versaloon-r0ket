#ifndef __BASICCONFIG_H_
#define __BASICCONFIG_H_

int readConfig(void);
int saveConfig(void);
void applyConfig(void);
void enableConfig(char type,char enable);

struct CDESC {
    char *name;
    char value;
    char min;
    char max;
    unsigned disabled :1;
    unsigned type     :3;
};

#define CFG_TYPE_BASIC 0
#define CFG_TYPE_DEVEL 1
#define CFG_TYPE_FLAME 2
#define CFG_TYPE_GONE 3

#define MAXNICK 17
extern struct CDESC the_config[];
extern char nickname[];
extern char nickfont[];
extern char nickl0[];

#define GLOBALversion      (the_config[ 0].value)
#define GLOBALprivacy      (the_config[ 1].value)
#define GLOBALdaytrig      (the_config[ 2].value)
#define GLOBALdaytrighyst  (the_config[ 3].value)
#define GLOBALdayinvert    (the_config[ 4].value)
#define GLOBALlcdbacklight (the_config[ 5].value)
#define GLOBALlcdmirror    (the_config[ 6].value)
#define GLOBALlcdinvert    (the_config[ 7].value)
#define GLOBALlcdcontrast  (the_config[ 8].value)
#define GLOBALalivechk     (the_config[ 9].value)
#define GLOBALdevelmode    (the_config[10].value)
#define GLOBALflamemax     (the_config[11].value)
#define GLOBALflamemin     (the_config[12].value)
#define GLOBALflamespeed   (the_config[13].value)
#define GLOBALflamemaxw    (the_config[14].value)
#define GLOBALflameminw    (the_config[15].value)
#define GLOBALl0nick       (the_config[16].value)
#define GLOBALchargeled    (the_config[17].value)
#define GLOBALpositionleds (the_config[18].value)
#define GLOBALnickname     (nickname)
#define GLOBALnickfont     (nickfont)
#define GLOBALnickl0       (nickl0)

#define GLOBAL(x) GLOBAL ## x

#endif
