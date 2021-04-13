#ifndef __DATACONFIG_H
#define __DATACONFIG_H

// #include "tablesign.h"
#include "cmsis_os.h"


/*设备是否为第一次启动*/
typedef enum{
  /*出厂状态*/
  FACTORYSTA,
  /*用户使用状态*/
  USESTA
}DEVFACTORY;

/*外部事件分类*/
typedef enum {
  BLEEVENT=1,
  LORAEVENT=2
}EXTEVENT;

/*图片传输状态标志位*/
typedef enum{
  /*正在传输*/
  pictraning=0,
  /*传输完成*/
  pictfinish,
}PICTRANSTAT;

/*设置是否进入了等待图片传输完成状态*/
typedef enum{
  /*需要等待*/
  WAITPIC=0,
  /*不需要等待*/
  NOWAITPIC
}WAITPICSTAT;

/*自检设备步骤*/
typedef enum{
  /*FLASH检测*/
  flashcheck = 0,
  /*字库检测*/
  gt60m2check,
  /*BLE检测*/
  blecheck,
  /*LoRa检测*/
  loracheck,
  /*电池电量*/
  batterycheck
}CHECKDEV;

/*解析字体参数*/
/*****数据包类型分类*****/
typedef enum {
  /*A面显示文字指令*/
  TextA=0x01,
  /*A显示成功\失败后的应答*/
  TextcallA=0x02,
  /*A面远程控制指令*/
  CmdA=0x11,
  /*A面远程控制指令应答*/
  CmdcallA=0x12,
  /*A面显示图片指令*/
  PictureA=0x21,
  /*A面图片指令应答*/
  PicturecallA=0x22,
  /*心跳包*/
  Heartdata=0x31,
  /*B面显示文字指令*/
  TextB=0x03,
  /*B显示成功\失败后的应答*/
  TextcallB=0x04,
  /*B面远程控制指令*/
  CmdB=0x13,
  /*B面远程控制指令应答*/
  CmdcallB=0x14,
  /*B面显示图片指令*/
  PictureB=0x23,
  /*B面图片指令应答*/
  PicturecallB=0x24,
  /*AB面显示文字指令*/
  TextAB=0x05,
  /*AB显示成功\失败后的应答*/
  TextcallAB=0x06,
  /*AB面远程控制指令*/
  CmdAB=0x15,
  /*AB面远程控制指令应答*/
  CmdcallAB=0x16,
  /*AB面显示图片指令*/
  PictureAB=0x25,
  /*AB面图片指令应答*/
  PicturecallAB=0x26,
  /*A面投字与图片*/
  TPictureA=0x41,  
  /*B面投字与图片*/
  TPictureB=0x43,   
  /*AB面投字与图片*/
  TPictureAB=0x45,   
}DataType_f;

/*****查询与设置指令集*****/
typedef enum {
  /*设备版本号指令*/
  Dversion = 01,
  /*设备信号强度查询*/
  Drssi = 02,
  /*电量查询指令*/
  Dbattery = 03,
  /*设备ID号指令*/
  DevID = 04,
  /*其他指令后面添加*/
}CmdType_f;

/*****查询或设置指令分类****/
typedef enum {
  /*查询指令*/
  Watchcmd = 0xD0,
  /*设置指令*/
  Setcmd = 0xC0,
}Workcmd_f;

/*****显示文字命令字节相关类型*****/
typedef enum {
  /*文字位置*/
  Startpoint=0xE1,
  /*文字大小*/
  Textsize =0xE2,
  /*文字字体*/
  Textfont =0xE3,
  /*文字内容*/
  Textcontent=0xE4,
  /*文字颜色*/
  Textcolor=0xE5
}CMDType_f;


/*设备属性是否正常*/
typedef enum{
    DEVNORMALLY,
    DEVEEROR
}DEVRUNSTAT;

/*单个设备自检参数*/
typedef struct 
{
/*设备属性*/
    DEVRUNSTAT devstatus;    
/*设备参数*/
    uint8_t paramdata[25];
}DevParamStat;

/*设备参数分类*/
typedef struct {
/*W25Q128外部Flash芯片*/    
    DevParamStat wFlash;
/*GT60M2字库芯片*/
    DevParamStat gT60M2Dev;
/*BLE蓝牙模组状态*/    
    DevParamStat eBLEDev;
/*LoRaWAN模组状态*/    
    DevParamStat eLoRaDev;
}DevTypeParam_t;


/******定义蓝牙设备相关状态与参数******/
typedef struct 
{
  /* 设备状态 */
  uint8_t  BLESTAT;
  /* MAC地址 */
  uint8_t  BLEMAC[15];
  /* 信号强度 */
}BleDev_Sta;

/******定义LoRa设备相关状态与参数******/
typedef struct 
{
  /* 设备状态 */
  uint8_t  LORASTAT;
  /* MAC地址 */
  uint8_t  LORAEUI[15];
  /* 信号强度 */

}LoRaDev_Sta;

/*图片参数*/
typedef struct {
  TableFace tfac;
  uint8_t  tdata[1124];
  uint32_t tdatlen;
}picDataParm;


void ext_eventTask(void *argument);

/***************************************************************
 * 分析蓝牙相关协议函数赋值相关显示文字参数
 * *************************************************************/
void Analyze_Wirle_Data(uint8_t *dat, uint32_t datlen);

/***************************************************************
 * 初始化串口等触发事件
 * *************************************************************/
int GetParamQueue_Init(void);

/***************************************************************
 * 接收到的编号与实际字体编号进行转换
 * *************************************************************/
uint8_t Swit_Rev_Vectype(uint8_t val);

/***************************************************************
 * 校验接收数据是否正确
 * *************************************************************/
int Check_Data_CRC(uint8_t *rdata, uint32_t rlen);

void BLE_Answer_Type(void);

uint32_t Get_pic_Addr(TableFace fac, uint8_t pictil, TableColorType col);

int PicTransQueue_Init(void);

osStatus Getstat_Queue_Event(PICTRANSTAT *stat);

void selfcheckTask(void *argument);

int PicDataQueue_Init(void);

osStatus_t PutPicData_Queue_Event(picDataParm *data);

osStatus GetPicData_Queue_Event(picDataParm *data);

osStatus GetPicDataING_Queue_Event(picDataParm *data);

int Picture_param_write(TableFace fac, uint8_t *dat, uint32_t datlen);

int FinishParamQueue_Init(void);

void Finishput_Queue_Event(void);

osStatus Finishget_Queue_Event(void);

#endif











