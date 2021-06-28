#ifndef __FONTOPT_H
#define __FONTOPT_H

#include "stdbool.h"
#include "stdint.h"
#include "string.h"
#include "usart.h"
#include "tablesign.h"

#define FONT_DATA_SIZE 12288 //10976//12288//4096//12288

#define FONT_129_SIZE  18500//18432//18432//4096//12288

#define BlackPicBlockSize 1600

#define PIC_ROW_SIZE 100

/*定义屏幕大小参数*/
#define   TABLEROW       100    //像素实际为8*100
#define   TABLECOLUMN    480

/*黑白红色文字基础值*/
#define   A_BLACKTEXTBASE  0x00000000
#define   A_WHITETEXTBASE  0x0000C000
#define   A_REDTEXTBASE    0x00018000
#define   A_PICADDRBASE    0x00024000

#define   B_BLACKTEXTBASE  0x00048000
#define   B_WHITETEXTBASE  0x00054000
#define   B_REDTEXTBASE    0x00060000
#define   B_PICADDRBASE    0x0006C000

#define   P_PICADDRBASE    0x00090000

typedef struct BlackPicBlock_t{

    uint8_t data[BlackPicBlockSize];
    
}BlackPicBlock;

typedef struct Font_t{
    uint8_t data[FONT_DATA_SIZE];
    uint8_t charwidth[30];//100 每条信息最大字节数
    uint8_t fontSize ; // ????
    uint8_t width;
    uint8_t realwidth;//实际宽度
    uint8_t numb; // ???
    uint8_t netstate;
    uint16_t offset; // ??
    uint8_t  font_format; //格式 楷体 黑体
    uint8_t  char_format; //asscii 字体类型
    uint8_t  font_color;//颜色
    uint16_t init_row;//行
    uint8_t  columns_start;//列
    uint8_t  DataEnable;
    uint8_t  Model_Black_Red;
    uint8_t  Model_White_Red;
}Fonts;


typedef struct Font192_t{

    uint8_t data[FONT_129_SIZE];
    uint8_t charwidth[30];//100 每条信息最大字节数
    uint8_t fontSize ; // ????
    uint8_t width;
    uint8_t realwidth;//实际宽度
    uint8_t numb; // ???
    uint16_t offset; // ??
    uint8_t  font_format; //格式 楷体 黑体
    uint8_t  char_format; //asscii 字体类型
    uint8_t  font_color;//颜色
    uint16_t init_row;//行
    uint8_t  columns_start;//列
    uint8_t  DataEnable;
    uint8_t  Model_Black_Red;
    uint8_t  Model_White_Red;
}Fonts192;

/*所需要的相关参数定义结构体*/
typedef struct Fontparam_t{
  uint8_t   Texttableface;  //桌牌面
  uint16_t  Textlocat; //文字位置首位置坐标
  uint8_t   TextFont;  //文字字体
  uint8_t   Textsize;  //文字大小
  uint8_t   Textcolour;  //文字颜色
  uint8_t   Textcount;   //文字个数
  uint8_t   Textdata[60];  //文字内容
  uint8_t   Textbackpic;   //背景图编号
  TableDisplayType eTextandPic;   //是否只是显示图片
}Fontsparam;


void InputTexttoFlash(Fontsparam fontdata);

void TableSectorErase(uint32_t saddr,uint8_t num);

int pic_input_init(void);

#endif



