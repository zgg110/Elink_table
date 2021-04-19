#ifndef __TABLESIGN_H
#define __TABLESIGN_H

#include "stm32l4xx.h"
//#define TABLE_Power                               PB_3  //SPI selection 3/4 line
//
//#define TABLE_MOSI                                PB_15   //sda
//#define TABLE_MISO                                PB_14   //D/c
//#define TABLE_SCLK                                PB_13
//#define TABLEA_NSS                                PA_8   // csb
//#define TABLEB_NSS                                PB_12  // csb
//#define TABLE_BS                                  PA_1   // bs
//#define TABLEA_RES                                PA_10  //res
//
//#define TABLEB_RES                                PA_9
//#define TABLEA_BUSY                               PA_12 //busy
//#define TABLEB_BUSY                               PA_11 //busy

#define TABLEPOWON()                              HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET)  
#define TABLEPOWOFF()                             HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET)

#define TABLE_BS_HIGH()                           HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET)  
#define TABLE_BS_LOW()                            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET)

#define TABLEA_RES_HIGH()                         HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_SET)  
#define TABLEA_RES_LOW()                          HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET)

#define TABLEB_RES_HIGH()                         HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_SET)  
#define TABLEB_RES_LOW()                          HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_RESET)

#define TABLEA_BUSY()                             HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_12) 
#define TABLEB_BUSY()                             HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_11)

#define TABLEA_NSS_HIGH()                         HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET)
#define TABLEA_NSS_LOW()                          HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET)

#define TABLEB_NSS_HIGH()                         HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET)
#define TABLEB_NSS_LOW()                          HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET)

#define TABLE_DC_HIGH()                           HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET)
#define TABLE_DC_LOW()                            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET)

#define COMPANY_POSIT_SIZE  512

#define COMPANY_POSIT_BYTE  8

#define NAME_NAME_SIZE     2048

#define POSIT_SIZE    512

#define NAME_BYTE    16


typedef enum {
  TabFaceA=0,
  TabFaceB,
  TabFaceAB
}TableFace;

typedef enum {
  SendDATA,
  SendCmd
}DataType;

typedef enum {
  WBColorPic = 0,
  RedColorPic
}TableColorType;

/*区分只显示图片还是图片文字一起显示*/
typedef enum {
  Picnone = 0,           //无  
  OnlyPic,           //只显示图片
  TextAndPic    //文字与图片一起    
}TableDisplayType;


typedef enum {
  PSR     = 0x00,
  POWR    = 0x01,
  POF     = 0x02,
  PON     = 0x04,
  BTST    = 0x06,
  DSLP    = 0x07,
  DTM1    = 0x10,
  DRF     = 0x12, //Display Refresh
  DTM2    = 0x13, //Data Start Transmission 2
  DUSPI   = 0x15, //Dual SPI Mode
  CDI     = 0x50, //VCOM and Data interval Setting
  TCON    = 0x60,
  FLG     = 0x71,
  VDCS    = 0x82
}TableCmd_F;

int Table_HandWare_Init(FunctionalState sta);
void TableSignInit(TableFace fac);
void TableSignWriteByte(TableFace fac,DataType typ,uint8_t data);
void TableSignWriteData(TableFace fac,uint8_t *data,uint32_t datalen);
int CheckTableBusy(TableFace fac,uint32_t timeout);


void tablesignTask(void *argument);
void EventQueueInit(void);
void BLE_Queue_Event(uint8_t *dat, uint32_t datlen);
int CheckTableBusy(TableFace fac,uint32_t timeout);

void TableSignSeting(TableFace fac, uint8_t pict,TableDisplayType dis);

#endif

