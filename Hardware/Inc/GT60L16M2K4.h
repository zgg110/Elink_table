#ifndef _GT60L16M2K4_H
#define _GT60L16M2K4_H


//客户自己构造，从 addr地址中获取 len 长度的数据，存放在p数组中
//extern unsigned char r_dat_bat(unsigned long  address,unsigned int byte_long,unsigned char *p_arr);//客户构造函数
extern unsigned char r_dat_bat(unsigned long int address,unsigned char byte_long,unsigned char *p_arr);

#define ASCII_16X16_T      1
#define ASCII_16X16_A      2
#define ASCII_24X24_T      3
#define ASCII_24X24_A      4 
#define ASCII_32X32_T      5
#define ASCII_32X32_A      6

//点阵ASCII
unsigned char ASCII_GetData(unsigned char ASCIICode,unsigned long type,unsigned char*DZ_Data);//换取不等宽的ASCII字符


#define cr_zf_8X16         1	//方块体字体风格
#define cr_zf_16X16_T      2
#define cr_zf_16X16_A      3
#define cr_zf_8X16_F      4		//方块体字体风格

#define cr_zf_24X24_T      5
#define cr_zf_24X24_A      6
#define cr_zf_16X24_F      7	//方块体字体风格
#define cr_zf_24X24_S      8	//时钟体字体风格


#define cr_zf_24X32_T      9
#define cr_zf_24X32_A      10
#define cr_zf_16X32_F      11
#define cr_zf_24X32_S      12


#define cr_zf_40X48_T      13
#define cr_zf_40X48_A      14
#define cr_zf_24X48_F      15
#define cr_zf_40X48_S      16


#define cr_zf_40X64_T      17
#define cr_zf_40X64_A      18
#define cr_zf_32X64_F      19
#define cr_zf_48X64_S      20


#define cr_zf_32X32_TB     21	//自制可穿戴设备图标

//自制体
void get_cr_zf(unsigned char Sequence,unsigned long type,unsigned char*DZ_Data);

//16点阵的压缩中文
void hzbmp16X16( unsigned int code, unsigned int codeex, unsigned char size,unsigned char *buf);//换取16X16点GB18030汉字


/********************* 矢量公用部分 *********************/
//ASCII点阵, 字号: 8x16, 16x24, 16x32.
#define VEC_GBK_STY			0x1

//矢量中文
#define VEC_SONG_STY    	0x3	//宋体
#define VEC_BLACK_STY   	0x5	//黑体
#define VEC_FANGS_STY		0x6	//仿宋
#define VEC_KAI_STY			0x7	//楷体

//矢量ASCII
#define VEC_FX_ASCII_STY	0x8	//方斜
#define VEC_BX_ASCII_STY 	0x9	//白斜
#define VEC_CH_ASCII_STY	0xA	//长黑
#define VEC_YT_ASCII_STY	0xB	//圆头
#define VEC_DZ_ASCII_STY	0xC	//打字
#define VEC_MS_ASCII_STY	0xD	//美术
#define VEC_GD_ASCII_STY	0xE	//歌德
#define VEC_HZ_ASCII_STY	0x10//黑正	
#define VEC_BZ_ASCII_STY	0x11//白正
#define VEC_FT_ASCII_STY	0x12//方头
#define VEC_SX_ASCII_STY	0x13//手写


/******************* 两种调用模式配置 *******************/ 

/**
 * 方式一 VEC_ST_MODE : 通过使用声明VECFONT_ST结构体变量, 配置结构体信息,
 *   获取点阵数据到zk_buffer[]数组中.
 * 方式二 VEC_PARM_MODE : 通过指定参数进行调用, 获取点阵数据到pBits[]数组中.
 * ps: 两种方式可同时配置使用, 择一使用亦可.
*/
#define VEC_ST_MODE
#define VEC_PARM_MODE

/********************* 分割线 *********************/

#ifdef VEC_ST_MODE

    #define ZK_BUFFER_LEN   4608    //可修改大小, 约等于 字号*字号/8.

    typedef struct vecFont
    {
        unsigned long fontCode;		//字符编码中文:GB18030, ASCII/外文: unicode
        unsigned char type;
        unsigned char size;
        unsigned char thick;
        unsigned char zkBuffer[ZK_BUFFER_LEN];
    }VECFONT_ST;

    unsigned int get_font_st(VECFONT_ST * font_st);
#endif

#ifdef VEC_PARM_MODE
    
//unsigned int get_font(unsigned char *pBits,unsigned char sty,unsigned long fontCode,unsigned char width,unsigned char height, unsigned char thick);
//unsigned int get_font(unsigned char *pBits,unsigned char sty,unsigned char MSB,unsigned char LSB,unsigned char width,unsigned char height, unsigned char thick);
void get_font(unsigned char *pBits,unsigned char sty,unsigned char msb,unsigned char lsb,unsigned char len,unsigned char wth,unsigned char thick);

#endif
/********************* 矢量区域结束 *********************/

//转码表
unsigned int U2GB18030(unsigned int UN_CODE);//unicode 转GB18030的函数

unsigned int BIG52GBK( unsigned char h,unsigned char l);//BIG5转GBK的函数


//字符效果
//灰度处理
void Gray_Process(unsigned char *OutPutData ,int width,int High,unsigned char Grade);
//斜体处理
unsigned int italic(unsigned char *pInData,unsigned char *pOutData,unsigned char width,unsigned char high);
//勾边处理
void font_hollow(unsigned char *pInData,unsigned char *pOutData,unsigned char high,unsigned char width);

#endif
