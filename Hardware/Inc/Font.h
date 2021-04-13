
#ifndef _FONT_H
#define _FONT_H


extern unsigned char r_dat(unsigned long int address);
extern unsigned char r_dat_bat(unsigned long int address,unsigned char byte_long,unsigned char *p_arr);

void get_font(unsigned char *pBits,unsigned char sty,unsigned char msb,unsigned char lsb,unsigned char len,unsigned char wth,unsigned char thick);

extern unsigned char pBits[192*192/8]; /* 192X192*/
extern unsigned char pram[6];

void GT60M2_Reset_Cmd(void);
#endif
