#include "stdlib.h"
#include "stdint.h"
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "task.h"
#include "FontOpt.h"
#include "tablesign.h"
#include "dataconfig.h"
#include "tablesign.h"
//#include "flashConfig.h"
#include "GT60M2.h"
#include "w25q128.h"
#include "blecomm.h"
#include "mbcrc.h"
//#include "battery_adc.h"

/*默认配置参数*/
Fontsparam fontdata1={1,820,VEC_SONG_STY,192,1,2,NULL};

/*设备自检参数集合定义*/
DevTypeParam_t eDevTypeParam;

/*定义是否需要等待图片传输完成*/
WAITPICSTAT eWaitPicSta = NOWAITPIC;
/*当显示为实时显示时记录要显示的指令传输给图片进行最后的显示*/
uint8_t eTextandPicFlag = 0;

/*同时投文字与图片标志位*/
uint8_t PicandTextflag = 0;

uint8_t FrameMaxCont = 0;

uint8_t nextaddrcount = NULL;

/*标志之前是否传入了文字*/
uint8_t eTableDisplayType = 0;

/*压缩图片接收为最后一包标志位*/
uint8_t comPicFinishFlag = 0;

extern uint8_t sign;

extern uint8_t BLEUart2RxData[1124];
extern uint32_t BLEUart2RxCnt;

// osThreadId_t tablesignTaskHandle;
// const osThreadAttr_t tablesignTask_attributes = {
//   .name = "tablesignTask",
//   .priority = (osPriority_t) osPriorityNormal,
//   .stack_size = 128 * 12
// };

// osThreadId_t extevenTaskHandle;
// const osThreadAttr_t extevenTask_attributes = {
//   .name = "extevenTask",
//   .priority = (osPriority_t) osPriorityNormal,
//   .stack_size = 128 * 12
// };


/*产生事件待处理队列队列，蓝牙串口接收队列*/
osMessageQueueId_t QueueEventHandle;
const osMessageQueueAttr_t QueueEvent_attributes = {
  .name = "QueueEvent"
};

/*创建文字参数队列，用于执行整面文字布局*/
osMessageQueueId_t GetParamHandle;
const osMessageQueueAttr_t GetParam_attributes = {
  .name = "GetParam"
};

/*图片传输状态队列*/
osMessageQueueId_t PicTransHandle;
const osMessageQueueAttr_t PicTrans_attributes = {
  .name = "PicTransStat"
};

/*图片数据队列*/
osMessageQueueId_t PicDataHandle;
const osMessageQueueAttr_t PicData_attributes = {
  .name = "PicDataStat"
};

/*投屏整体完成队列*/
osMessageQueueId_t FinishParamHandle;
const osMessageQueueAttr_t FinishParam_attributes = {
  .name = "FinishParam"
};

// /*定义互斥信号量用于锁定接收蓝牙数据的锁死*/
// osMutexDef( Mutex );
// osMutexId mutex;

/******************************************************************
 * 函数为初始化队列参数 用于存储文字内容
 * 
 * 返回队列创建是否成功状态   返回1 为创建失败   返回0 为创建成功
 * ****************************************************************/
int GetParamQueue_Init(void)
{
  // Create a queue capable of containing 300 uint8_t values.
  GetParamHandle = osMessageQueueNew (20, sizeof(Fontsparam), &GetParam_attributes);
  if( GetParamHandle == NULL )
  {
    return 1;// Queue was not created and must not be used.  //可以打开中断或者初始化对应串口
  }     
  return 0;
}


/*****************************************************************
 * 
 * 将参数集推入队列中
 * 
 * ***************************************************************/
void GetParam_Queue_Event(Fontsparam *pardata)
{
  osMessageQueuePut(GetParamHandle, pardata, NULL, 100);
}


/********************************************************************************
 * 
 * 删除获取参数队列
 * 
 * ******************************************************************************/
osStatus_t ParamQueueDelet(void)
{
  user_main_info("Param Queue Delete success!");
  return osMessageQueueDelete(GetParamHandle);
}


/******************************************************************
 * 函数为初始化队列参数 用于表示已经显示整体完成
 * 
 * 返回队列创建是否成功状态   返回1 为创建失败   返回0 为创建成功
 * ****************************************************************/
int FinishParamQueue_Init(void)
{
  // Create a queue capable of containing 300 uint8_t values.
  FinishParamHandle = osMessageQueueNew(2, sizeof(uint8_t), &FinishParam_attributes);
  if( FinishParamHandle == NULL )
  {
    return 1;// Queue was not created and must not be used.  //可以打开中断或者初始化对应串口
  }     
  return 0;
}

/*****************************************************************
 * 
 * 将参数集推入队列中
 * 
 * ***************************************************************/
void Finishput_Queue_Event(void)
{
  uint8_t sta = 1;
  osMessageQueuePut(FinishParamHandle, &sta, NULL, 100);
}


osStatus Finishget_Queue_Event(void)
{
  uint8_t estat;
  return osMessageQueueGet(GetParamHandle, &estat, NULL, portMAX_DELAY);
}



/******************************************************************
 *
 *解析文字命令段协议函数
 *
*********************************************************************/
int Text_param_set(Fontsparam *fonts,uint8_t *dat, uint32_t datlen)
{
  switch((CMDType_f)dat[0]){
  case Startpoint:
    fonts->Textlocat = (dat[2]<<8)|dat[3];
    nextaddrcount = 4;   //将指针向后推移到对应位置
    break;
  case Textsize:
    fonts->Textsize = dat[2];
    nextaddrcount = 3;
    break;
  case Textfont:
    fonts->TextFont = dat[2];
   
    nextaddrcount = 3;
    break;
  case Textcontent:
    fonts->Textcount = dat[2]/2; //文字内容个数
    memcpy(fonts->Textdata,&dat[3],dat[2]);
    nextaddrcount = (dat[2]+3);
    break;
  case Textcolor:
    fonts->Textcolour = dat[2];    
    nextaddrcount = 3;
    break;
  default:
    nextaddrcount = NULL;
    return 1;
  }
  return 0;
}


/******************************************************************
 *
 *判断属于第几波参数进行参数赋值 将所有参数放在临时空间内 并赋值给队列
 *
*********************************************************************/
int Text_param_count(DataType_f dtype, uint8_t *dat, uint32_t datlen,Fontsparam *param)
{
   uint32_t edatadd = 0;   
   uint16_t edatalen = 0;  //定义字符字节长度
   Fontsparam Fparam;
   //if(param[*(dat+1)-1] == NULL) memcpy(&param[*(dat+1)-1], &fontdata1, sizeof(Fontsparam));
   /*获取在设备中为第几行数据*/
   edatadd = *(dat+1)-1;
   
   memset(&param[edatadd],0,sizeof(Fontsparam));
   /*获取屏幕面*/
   param[edatadd].Texttableface = dtype;
   user_main_info("Text display face,inputdata:0x%02x",param[edatadd].Texttableface);
   /*获取对应参数*/
   /*获取总字符长度*/
   edatalen = (uint16_t)((dat[2]<<8)|dat[3]);

   param[edatadd].Textbackpic = dat[4];                      //图片编号

   if(param[edatadd].Textbackpic > 200)
   {
     user_main_error("Text Picture location error,inputdata:0x%04x",param[edatadd].Textbackpic);
     return 1;
   }
   else
   {
     user_main_info("Text Picture location is rigth,inputdata:0x%04x",param[edatadd].Textbackpic);
   }
      
   param[edatadd].Textlocat = (dat[5]<<8)|dat[6];            //显示位置
   user_main_info("Text location,inputdata:0x%04x",param[edatadd].Textlocat);

   if((dat[7] != 32) && (dat[7] != 48) && (dat[7] != 64) && (dat[7] != 80) && (dat[7] != 96) && (dat[7] != 112) && (dat[7] != 128) && (dat[7] != 144) && (dat[7] != 192))
   {
     user_main_error("Text Size error,inputdata:0x%02x",dat[7]);
     return 1;
   }
   else
   {
     param[edatadd].Textsize = dat[7];                         //文字大小
     user_main_info("Text Size is rigth,inputdata:0x%02x",param[edatadd].Textsize);
   }
      
   if(param[edatadd].TextFont > 4)
   {
     user_main_error("Text Font error,inputdata:0x%02x",dat[8]);
     return 1;     
   }
   else
   {
     param[edatadd].TextFont = Swit_Rev_Vectype(dat[8]);       //转换为的文字格式
     user_main_info("Text Font is rigth,inputdata:0x%02x",param[edatadd].TextFont);
   }
        
   if(param[edatadd].Textcolour > 2)
   {
     user_main_error("Text Color error,inputdata:0x%02x",dat[9]);
     return 1;
   }
   else
   {
     param[edatadd].Textcolour = dat[9];                       //显示颜色     
     user_main_info("Text Color is rigth,inputdata:0x%02x",param[edatadd].Textcolour);
   }
          
   param[edatadd].Textcount = (uint8_t)(edatalen-6)/2;      //文字内容个数
   user_main_info("Text Count,inputdata:0x%02x",param[edatadd].Textcount);
   memcpy(&param[edatadd].Textdata,&dat[10],edatalen-6);    //获取文字内容
   
   memcpy(&Fparam,&param[edatadd],sizeof(Fontsparam));
   /*将数据放到对应的队列中*/
   if(GetParamHandle != NULL)
    GetParam_Queue_Event(&Fparam);
   /*判断最大段数*/
  //  FrameMaxCont = FrameMaxCont > *(dat+1) ? FrameMaxCont : *(dat+1);   
   nextaddrcount = edatalen + 4;   //获取跳转指针计数
  return 0;  
}

/******************************************************************
 *
 *字体分配数值转换
 *
*********************************************************************/
uint8_t Swit_Rev_Vectype(uint8_t val)
{
  switch(val){
    case 1:
      return VEC_SONG_STY;
    case 2:
      return VEC_BLACK_STY;
    case 3:
      return VEC_FANGS_STY;
    case 4:
      return VEC_KAI_STY;
    default: 
      return VEC_SONG_STY;
  }
}


/******************************************************************
 *
 *包头以及CRC数据校验 并归零CRC位
 *
*********************************************************************/
int Check_Data_CRC(uint8_t *rdata, uint32_t rlen)
{
  uint16_t eCRC;

  eCRC = usMBCRC16( rdata, rlen-3 );
  if((rdata[0]==0xFF) && (eCRC==(rdata[rlen-2]<<8)|rdata[rlen-3]))
  {
    rdata[rlen-3] = 0;
    rdata[rlen-2] = 0;
    return 0;   //校验成功
  }
  else
    return 1;   //校验失败
}


/******************************************************************
 *文字数据处理
 *申请动态内存添加多行文字参数
 *
*********************************************************************/
void Text_param_write(uint8_t *dat, uint32_t datlen)
{
    Fontsparam *fontdata;
    Fontsparam fdata;
    uint8_t pface,plist;   //图片编号

    pface = *(dat+1);
    plist = *(dat+8);   
    /*在获取文字数据之前 擦除写字区域*/ 
    if(pface == 0x01)               
      TableSectorErase(A_BLACKTEXTBASE,36);  
    else if(pface == 0x03) 
      TableSectorErase(B_BLACKTEXTBASE,36);
    else
      TableSectorErase(A_BLACKTEXTBASE,36);      
    /*动态获取相关投字参数*/
    fontdata=(Fontsparam*)pvPortMalloc(sizeof(Fontsparam)*11);
    memset(fontdata,0,sizeof(Fontsparam)*11);    
    dat += 4;
    datlen -= 4;    
    while(!Text_param_count((DataType_f)pface,dat,datlen,fontdata))
    {
      dat += nextaddrcount; //指针后移
      datlen -= nextaddrcount; //获取剩余字节个数
      if(datlen <= 3)
      {
        PicandTextflag=1;
        /*写入结束数据*/
        memset(&fdata,0,sizeof(Fontsparam));
        // memcpy(&fdata,&fontdata[0],sizeof(Fontsparam));
        // fdata.Textcount = 0;
        if(plist != 0)
        {
          fdata.eTextandPic = TextAndPic;
          fdata.Texttableface = pface;
          fdata.Textbackpic = plist;  
          GetParam_Queue_Event(&fdata);
        }
        else
        {
          eTextandPicFlag = pface;
        }
        break;
      }
    }
    // /*逐个结构体加入队列等待被使用*/
    // for(uint8_t i=0;i<FrameMaxCont;i++)
    //   GetParam_Queue_Event(&fontdata[i]);
    vPortFree(fontdata);
}

/******************************************************************
 * 函数为初始化图片传输状态队列参数 
 * 
 * 返回队列创建是否成功状态   返回1 为创建失败   返回0 为创建成功
 * ****************************************************************/
int PicTransQueue_Init(void)
{
  // Create a queue capable of containing 300 uint8_t values.
  PicTransHandle = osMessageQueueNew (50, sizeof(uint8_t), &PicTrans_attributes);
  if( PicTransHandle == NULL )
  {
    return 1;// Queue was not created and must not be used.  //可以打开中断或者初始化对应串口
  }     
  return 0;
}

/*****************************************************************
 * 
 * 将传输状态参数集推入队列中
 * 
 * ***************************************************************/
void Putstat_Queue_Event(PICTRANSTAT *stat)
{
  osMessageQueuePut(PicTransHandle, stat, NULL, 100);
}


/*****************************************************************
 * 
 * 获取传输状态参数
 * 
 * ***************************************************************/
osStatus Getstat_Queue_Event(PICTRANSTAT *stat)
{
  return osMessageQueueGet(PicTransHandle, stat, NULL, 4000);
}


/******************************************************************
 * 函数为初始化图片传输数据队列函数
 * 
 * 返回队列创建是否成功状态   返回1 为创建失败   返回0 为创建成功
 * ****************************************************************/
int PicDataQueue_Init(void)
{
  // Create a queue capable of containing 300 uint8_t values.
  PicDataHandle = osMessageQueueNew (3, sizeof(picDataParm), &PicData_attributes);
  if( PicDataHandle == NULL )
  {
    return 1;// Queue was not created and must not be used.  //可以打开中断或者初始化对应串口
  }     
  return 0;
}

/*****************************************************************
 * 
 * 将传输数据推入队列中
 * 
 * ***************************************************************/
osStatus_t PutPicData_Queue_Event(picDataParm *data)
{
  return osMessageQueuePut(PicDataHandle, data, NULL, 100);
}



/*****************************************************************
 * 
 * 获取图片数据
 * 
 * ***************************************************************/
osStatus GetPicData_Queue_Event(picDataParm *data)
{
  return osMessageQueueGet(PicDataHandle, data, NULL, portMAX_DELAY);
}

osStatus GetPicDataING_Queue_Event(picDataParm *data)
{
  return osMessageQueueGet(PicDataHandle, data, NULL, 1800);
}

/*****************************************************************
 * 
 * 跳过文字直接传输图片函数
 * 
 * ***************************************************************/
void Only_Pic_Display(uint8_t pfac,uint8_t plit)
{
  Fontsparam pdata;

  /*写入结束数据*/
  memset(&pdata,0,sizeof(Fontsparam));

  pdata.eTextandPic = OnlyPic;
  pdata.Texttableface = pfac;
  pdata.Textbackpic = plit;
  pdata.Textlocat = 0;
  pdata.Textcount = 0;
  GetParam_Queue_Event(&pdata);
}

/*****************************************************************
 * 
 * 设置文字与图片同时传输
 * 
 * ***************************************************************/
void Text_Pic_Display(uint8_t pfac,uint8_t plit)
{
  Fontsparam pdata;

  /*写入结束数据*/
  memset(&pdata,0,sizeof(Fontsparam));

  pdata.eTextandPic = TextAndPic;
  pdata.Texttableface = pfac;
  pdata.Textbackpic = plit;
  pdata.Textlocat = 0;
  pdata.Textcount = 0;
  GetParam_Queue_Event(&pdata);
}

/******************************************************************
 *图片数据处理
 *将接收到的图片数据拆分并写入FLASH中
 *
*********************************************************************/
int Picture_param_write(TableFace fac, uint8_t *dat, uint32_t datlen)
{
  PICTRANSTAT picstat;
  
  static uint32_t firstaddr;
  static uint32_t nextaddr;   //下一包数据地址的位置
  static uint16_t ecount;
  // uint16_t tokendata;   
  uint8_t plist;   //图片编号
  uint8_t pcolor;  //图片颜色
  uint16_t pcnt;           //图片包当前包编码
  uint16_t datcont;  //跟随像素数据计数
  uint8_t nextmidval = 0;
  uint8_t picdata[1024]; //实际数据包

user_main_info("Get Data"); 
  /*获取token*/
  // tokendata = ((dat[2]<<8)|dat[3]);
  /*获取图片编号*/
  plist = dat[4];
  if(plist > 200)
  {
    user_main_error("Picture location error");
    return 1;
  }
  /*获取颜色层*/
  pcolor = dat[5];
  if(pcolor > 2)
  {
    user_main_error("Picture Color error");
    return 1;
  }  
  /*获取包序号*/
  pcnt = ((dat[6]<<8)|dat[7]); 
 
  /*获取像素数据的个数*/
  datcont = ((dat[8]<<8)|dat[9]);
  if(datcont > (datlen-11))
  {
    user_main_error("Picture Pix error, Data:0x%04x",datcont);
    return 1;
  } 

  /*根据验证信息写入到对应的FALSH地址中*/
  memcpy(picdata,&dat[10],datcont);
  if(datlen != 0)
  {
    /*擦除*/
    if(pcnt==1)
    {
      /*进入第一个计数*/
      ecount = 1;
      firstaddr=Get_pic_Addr(fac, plist,(TableColorType)(pcolor-1));
      nextaddr = firstaddr;
      /*擦除对应地址上的图片数据*/
      TableSectorErase(nextaddr,12);
      /*跳过文字显示图片*/
      if( PicandTextflag != 1)
      {
        PicandTextflag=1;
        if(dat[4] == 0)
          /*需要等待图片完成*/
          eWaitPicSta = WAITPIC;       
      }
    }
    else
    {
      if(++ecount != pcnt) return 1;
    }

    user_main_debug("wirte falsh!!");   
    /*方法一*/
    /*判断当前地址是否为256的整数倍*/
    if((nextaddr%256 > 0) && (datcont > (256-(nextaddr%256))))
    {
      // user_main_debug("wirte %d!!",256-(nextaddr%256));
      W25X_FLASH_PageWrite(&picdata[0],nextaddr,256-(nextaddr%256));
      // user_main_debug("wirte 1 finish!!");
      nextmidval = 256-(nextaddr%256);
      nextaddr += nextmidval;
      datcont -= nextmidval;
    }
    /*每256字节为一个单位，剩余的单独写入*/
    for(uint16_t n=0;n<(datcont/256);n++)
    {
    // if((256-(addr%256)) > fontdata.Textsize/8)
    // user_main_debug("wirte falsh!!");
      W25X_FLASH_PageWrite(&picdata[n*256+nextmidval],n*256+nextaddr,256);
    }
    if(datcont%256 > 0)
      W25X_FLASH_PageWrite(&picdata[datcont-(datcont%256)+nextmidval],(nextaddr+datcont)-(datcont%256),datcont%256);   //(nextaddr+datcont)-(datcont%256)  
    user_main_debug("wirte falsh finish!!");
    // /*每256字节为一个单位，剩余的单独写入*/
    // for(uint16_t n=0;n<(datcont/256);n++)
    // // if((256-(addr%256)) > fontdata.Textsize/8)
    //   W25X_FLASH_PageWrite(&picdata[n*256],n*256+nextaddr,256);
    // if(datcont%256 > 0)
    //   W25X_FLASH_PageWrite(&picdata[datcont-(datcont%256)],(nextaddr+datcont)-(datcont%256),datcont%256);   //(nextaddr+datcont)-(datcont%256)      
    /*方法二*/
    // for(uint16_t n=0;n<datcont;n++)
    // {
    //   W25X_FLASH_PageWrite(&picdata[n],nextaddr+n,1);
    //   osDelay(1);
    // }
    /*获取下一个数据包的开始位置*/
    nextaddr += datcont;
  }
   /*判断是否为最后一包数据*/
   user_main_info("eWaitPicSta = %d",eWaitPicSta);
  if(eWaitPicSta == WAITPIC)
  {
    user_main_info("comPicFinishFlag = %d",comPicFinishFlag);
    /*判断最后一包数据*/
    if(comPicFinishFlag == 1)  //(nextaddr-firstaddr)>=48000  (pcnt >= 47) && (pcolor == 0x02)
    {
      comPicFinishFlag = 0;
      if(eTableDisplayType == 0)
      {
        eTableDisplayType = 1;    
        Only_Pic_Display(*(dat+1),plist);
      }
      else
      {
        Text_Pic_Display(eTextandPicFlag,plist);
      }
      picstat = pictfinish;
    }
    else
      picstat = pictraning;
    Putstat_Queue_Event(&picstat);    
  }
  user_main_info("picter input to flash packet finish,picture number: %d, packet number: %d",plist,pcnt); 

  return 0;
}

/******************************************************************
 *
 *图片地址分配（根据数据中显示的是第几章图片）
 *参数：pictil 图片编号
 *
*********************************************************************/
uint32_t Get_pic_Addr(TableFace fac, uint8_t pictil, TableColorType col)
{
  if(pictil == 0)
  {
    if(fac == TabFaceB)
      return B_PICADDRBASE + (col*12*4096);
    else
      return A_PICADDRBASE + (col*12*4096);
  }
  else
  {
    return B_PICADDRBASE + (pictil*24*4096) + (col*12*4096);
  }
}

/*******************************************************************
 * 
 * 发送应答数据函数
 * 
 * *****************************************************************/
void BLE_Answer_Data(uint8_t *pData, uint16_t Size) 
{
  BLE_Send_Data(pData,Size);
  // user_main_debug("Answer Data:%02x",pData);
}

void BLE_Answer_Type(void)
{
  uint8_t ackdata[10]={0xFF,0xA1,0xFF,0xFF,0x00,0x01,0x00,0x00,0x00,0xEE};
  BLE_Answer_Data(ackdata,10);
}

/*************************************************************************
 * 
 * 设备内通用指令分析函数，适用于设备内通用的查询指令与设置指令
 * 
 * *************************************************************************/
void Pul_Cmd_Analyze(uint8_t *indat, uint32_t inlen, uint8_t *outdat, uint32_t *outlen)
{
  switch ((CmdType_f)*(indat+1))
  {
  /*版本号*/
  case Dversion:
    /*内部判断是设置还是查询*/
    if((Workcmd_f)*(indat+2) == Watchcmd)
    {
      outdat[0] = Dversion;
      outdat[1] = Watchcmd;
      outdat[2] = 0x20;
      outdat[3] = 0x21;
      outdat[4] = 0x04;
      outdat[5] = 0x01;
      outdat[6] = 0x01;
      outdat[7] = 0x00;
      *outlen = 8;
    }
    else 
      ;
    break;
  /*信号强度*/
  case Drssi:
    /*内部判断是设置还是查询*/
    if((Workcmd_f)*(indat+2) == Watchcmd)
      ;
    else
      ;    
    break;
  /*电池电量*/  
  case Dbattery:
    /*内部判断是设置还是查询*/
    if((Workcmd_f)*(indat+2) == Watchcmd)
      ;
    else
      ;  
    break;
  /*设备ID号*/
  case DevID:
    /*内部判断是设置还是查询*/
    if((Workcmd_f)*(indat+2) == Watchcmd)
    {
      outdat[0] = DevID;
      outdat[1] = Watchcmd;
      outdat[2] = 0x20;
      outdat[3] = 0x21;
      outdat[4] = 0x00;
      outdat[5] = 0x00;
      outdat[6] = 0x00;
      outdat[7] = 0x01;
      *outlen = 8;
    }
    else
      ;  
    break;
  default:

    break;
  }
}


/******************************************************************
 *
 *解析来自蓝牙相关协议并配置参数
 *参数：每一小节的参数信息 判断之后输入 *dat   字节长度datlen
 *
*********************************************************************/
void Analyze_Wirle_Data(uint8_t *dat, uint32_t datlen)
{
  uint8_t ackdata[20]={0xFF,0x02,0xFF,0xFF,0xE1,0x01,0xFF,0x00,0x00,0xEE,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
  uint16_t sCRC;
  uint32_t olen;
  
  // PICTRANSTAT pics;
  picDataParm picpramdata; 
  switch((DataType_f)*(dat+1)){
    case TPictureA:
      /*需要等待图片完成*/
      eWaitPicSta = WAITPIC;
    case TextA:
      /*添加应答数据*/
      ackdata[1]=TextcallA;
      /*设置需要文字图片同时传输*/
      eTableDisplayType = 1;      
      /*判断是否为直接显示图片命令*/
      if(dat[8] == 0)
      {
        /*需要等待图片完成*/
        eWaitPicSta = WAITPIC;     
      }
      //  Putstat_Queue_Event(&pics);
      /*将数据拆分为多行数据参数放入队列*/
       Text_param_write(dat,datlen);      
      /*判断数据后进行应答*/
       sCRC = usMBCRC16(ackdata, 7);
       ackdata[7]=(uint8_t)(sCRC>>8);
       ackdata[8]=(uint8_t)sCRC;
       ackdata[9]=0xEE;
       ackdata[10]=0x00;              
       BLE_Answer_Data(ackdata,10);
      break;
    // case TextcallA:
    //   break;
    // case CmdA:
    //   break;
    // case CmdcallA:
    //   break;
    case PictureA:
      ackdata[1]=PicturecallA;   
      ackdata[4]=0x01;
      ackdata[5]=0x01;
      ackdata[6]=0x00;
      ackdata[7]=0x01;
      ackdata[8]=0xFF;
      sCRC = usMBCRC16(ackdata, 9);
      ackdata[9]=(uint8_t)(sCRC>>8);
      ackdata[10]=(uint8_t)sCRC;
      ackdata[11]=0xEE;
      ackdata[12]=0x00;
      // if(!Picture_param_write(TabFaceA,dat,datlen)) ackdata[8]=0x00;
      picpramdata.tfac = TabFaceA;
      memcpy(picpramdata.tdata,dat,sizeof(picpramdata.tdata));
      picpramdata.tdatlen = datlen;
      if(PutPicData_Queue_Event(&picpramdata)) ackdata[8]=0x00;    
      BLE_Answer_Data(ackdata,12);
      break;
    // case PicturecallA:
    //   break;
    // case Heartdata:
    //   break;
    case TPictureB:
      /*需要等待图片完成*/
      eWaitPicSta = WAITPIC;      
    case TextB:
      /*添加应答数据*/
      ackdata[1]=TextcallB;
      /*设置需要文字图片同时传输*/
      eTableDisplayType = 1;      
      /*判断是否为直接显示图片命令*/
      if(dat[8] == 0)
      {
        /*需要等待图片完成*/
        eWaitPicSta = WAITPIC;      
      }    
      //  Putstat_Queue_Event(&pics);
      /*将数据拆分为多行数据参数放入队列*/
       Text_param_write(dat,datlen);      
      /*判断数据后进行应答*/
       sCRC = usMBCRC16(ackdata, 7);
       ackdata[7]=(uint8_t)(sCRC>>8);
       ackdata[8]=(uint8_t)sCRC;
       ackdata[9]=0xEE;
       ackdata[10]=0x00;      
       BLE_Answer_Data(ackdata,10);
      break;
    // case TextcallB:
    //   break;
    // case CmdB:
    //   break;
    // case CmdcallB:
    //   break;
    case PictureB:
      ackdata[1]=PicturecallB; 
      ackdata[4]=0x01;
      ackdata[5]=0x01;
      ackdata[6]=0x00;
      ackdata[7]=0x01;
      ackdata[8]=0xFF;
      sCRC = usMBCRC16(ackdata, 9);
      ackdata[9]=(uint8_t)(sCRC>>8);
      ackdata[10]=(uint8_t)sCRC;
      ackdata[11]=0xEE;
      ackdata[12]=0x00;  
      // if(!Picture_param_write(TabFaceB,dat,datlen)) ackdata[8]=0x00;
      picpramdata.tfac = TabFaceB;
      memcpy(picpramdata.tdata,dat,sizeof(picpramdata.tdata));
      picpramdata.tdatlen = datlen;
      if(PutPicData_Queue_Event(&picpramdata)) ackdata[8]=0x00;             
      BLE_Answer_Data(ackdata,12);
      break;
    // case PicturecallB:
    //   break;
    case TPictureAB:
      /*需要等待图片完成*/
      eWaitPicSta = WAITPIC;        
    case TextAB:
      /*添加应答数据*/
      ackdata[1]=TextcallAB;
      /*设置需要文字图片同时传输*/
      eTableDisplayType = 1;      
      /*判断是否为直接显示图片命令*/
      if(dat[8] == 0)
      {
        /*需要等待图片完成*/
        eWaitPicSta = WAITPIC;     
      }
      //  Putstat_Queue_Event(&pics);
      /*将数据拆分为多行数据参数放入队列*/
       Text_param_write(dat,datlen);      
      /*判断数据后进行应答*/
       sCRC = usMBCRC16(ackdata, 7);
       ackdata[7]=(uint8_t)(sCRC>>8);
       ackdata[8]=(uint8_t)sCRC;
       ackdata[9]=0xEE;
       ackdata[10]=0x00;                
       BLE_Answer_Data(ackdata,10);
      break;
    case PictureAB:
    /*应答AB面投图数据*/
      ackdata[1]=PicturecallAB; 
      ackdata[4]=0x01;
      ackdata[5]=0x01;
      ackdata[6]=0x00;
      ackdata[7]=0x01;
      ackdata[8]=0xFF;
      sCRC = usMBCRC16(ackdata, 9);
      ackdata[9]=(uint8_t)(sCRC>>8);
      ackdata[10]=(uint8_t)sCRC;
      ackdata[11]=0xEE;
      ackdata[12]=0x00;
      // if(!Picture_param_write(TabFaceAB,dat,datlen)) ackdata[8]=0x00;   
      picpramdata.tfac = TabFaceAB;
      memcpy(picpramdata.tdata,dat,sizeof(picpramdata.tdata));
      picpramdata.tdatlen = datlen;
      if(PutPicData_Queue_Event(&picpramdata)) ackdata[8]=0x00;          
      BLE_Answer_Data(ackdata,12);
      break;  
    case CmdA:
      ackdata[1]=CmdcallA;
      ackdata[2]=0xFF;
      ackdata[3]=0xFF;       
      Pul_Cmd_Analyze(dat, datlen, &ackdata[4], &olen);
      /*判断数据后进行应答*/
       sCRC = usMBCRC16(ackdata, olen+4);
       ackdata[12]=(uint8_t)(sCRC>>8);
       ackdata[13]=(uint8_t)sCRC;
       ackdata[14]=0xEE;
       ackdata[15]=0x00;                
       BLE_Answer_Data(ackdata,16);      
      break;
    case CmdB:
      ackdata[1]=CmdcallB;
      ackdata[2]=0xFF;
      ackdata[3]=0xFF;       
      Pul_Cmd_Analyze(dat, datlen, &ackdata[4], &olen);
      /*判断数据后进行应答*/
       sCRC = usMBCRC16(ackdata, olen+4);
       ackdata[12]=(uint8_t)(sCRC>>8);
       ackdata[13]=(uint8_t)sCRC;
       ackdata[14]=0xEE;
       ackdata[15]=0x00;                
       BLE_Answer_Data(ackdata,16);      
      break; 
    case CmdAB:
      ackdata[1]=CmdcallAB;
      ackdata[2]=0xFF;
      ackdata[3]=0xFF;       
      Pul_Cmd_Analyze(dat, datlen, &ackdata[4], &olen);
      /*判断数据后进行应答*/
       sCRC = usMBCRC16(ackdata, olen+4);
       ackdata[12]=(uint8_t)(sCRC>>8);
       ackdata[13]=(uint8_t)sCRC;
       ackdata[14]=0xEE;
       ackdata[15]=0x00;                
       BLE_Answer_Data(ackdata,16);      
      break;      
    default:
      user_main_error("ERROR CMD HEART:%02x",*(dat+1));
      nextaddrcount = NULL;
      break;      
  }
}


/************************************************************************外部事件处理分配***************************************************************************/

/*********************************************************************************
 *
 *初始化创建事件队列,将串口或按键等事件放进队列等待处理
 *
**********************************************************************************/
void EventQueueInit(void)
{
  // Create a queue capable of containing 300 uint8_t values.
  QueueEventHandle = osMessageQueueNew (30, sizeof(uint8_t), &QueueEvent_attributes);
  if( QueueEventHandle == NULL )
  {
    user_main_error("ble uart init fail");
    // Queue was not created and must not be used.  //可以打开中断或者初始化对应串口
  }    
}

/********************************************************************************
* 
* 蓝牙串口事件，在蓝牙接收完一包数据之后触发此任务
* 
* ******************************************************************************/
void BLE_Queue_Event(uint8_t *dat, uint32_t datlen)
{
  EXTEVENT bleevent=BLEEVENT;  
  osMessageQueuePut(QueueEventHandle, &bleevent, NULL, 0);
}

/********************************************************************************
 * 
 * 删除事件队列
 * 
 * ******************************************************************************/
osStatus_t EventQueueDelet(void)
{
  user_main_info("Event Queue Delete success!");
  return osMessageQueueDelete(QueueEventHandle);
}


/*********************************************************************************
 * 
 * 外部事件处理任务，等待事件触发用于分配处理
 * 
**********************************************************************************/
void ext_eventTask(void *argument)
{
  EXTEVENT pevent;
  // EventQueueInit();

  if(PicTransQueue_Init())
  {
    user_main_error("Picture Trans State Init Fail!");
  } 
  while(1)
  {
    user_main_debug("BLE RX Data seuccess"); 
    if(osMessageQueueGet(QueueEventHandle, &pevent, NULL, portMAX_DELAY)==osOK)
    {
      // /*打印接收数据LOG*/
      // LOG("ble RX:");
      // for(int i=0;i<BLEUart2RxCnt;i++)
      //   LOG(" %002x",BLEUart2RxData[i]);
      // LOG("\r\n");      
      
      /*判断接收到的数据是否符合正常数据长度，如果不符合可以直接PASS掉*/
      if(BLEUart2RxCnt > 5)
      {
        if(!Check_Data_CRC(BLEUart2RxData,BLEUart2RxCnt))
        {
          /*当有数据写入时需要打开电源*/
          TABLEPOWON();           
          /*验证数据是从哪写入*/          
          switch(pevent){
            case BLEEVENT:
              user_main_debug("BLE RX Data lenth:%d",BLEUart2RxCnt); 
              Analyze_Wirle_Data(BLEUart2RxData,BLEUart2RxCnt);                       
              break;
            case LORAEVENT:
              user_main_debug("LoRa RX Data lenth:%d",BLEUart2RxCnt);
              break;
            default:
            /*打印错误数据到LOG*/
              user_main_error("Receive Event Error!!!");
              LOG("ble error RX:");
              for(int i=0;i<BLEUart2RxCnt;i++)
                LOG(" %002x",BLEUart2RxData[i]);
              LOG("\r\n");
              /*清除错误数据并且打印错误*/
              Clear_BLEUart_Data();                    
              break; 
          }         
        }  
        else
        {
          user_main_error(" rx data check fail");
          LOG("ble error RX:");
          for(int i=0;i<BLEUart2RxCnt;i++)
            LOG(" %002x",BLEUart2RxData[i]);
          LOG("\r\n");        
          /*发送数据错误应答*/
          BLE_Answer_Type();
        }
      }
      else
      {
        user_main_error(" rx data lenth is too short");
        LOG("ble error RX:");
        for(int i=0;i<BLEUart2RxCnt;i++)
          LOG(" %002x",BLEUart2RxData[i]);
        LOG("\r\n");           
      } 
      /*清除错误数据并且打印错误*/
      Clear_BLEUart_Data();     
    }
  }
}


/*******************************************************************
 *
 *分别检测设备数据
 *
********************************************************************/
int selfcheck_config(CHECKDEV chtype)
{
  uint32_t batval;
  char statstr[10];
  uint8_t FLASHID[4];
  uint8_t gcheckdata[8];
  
  EXTEVENT pevent;

  switch (chtype)
  {
  case flashcheck:
    /* 读取FLASH版本号 */
    W25X_FLASH_ReadID(FLASHID,4);
    memset(statstr,0,10);
    if(FLASHID[0] == 0xEF)
    {
      /* FLASH正常 */
      eDevTypeParam.wFlash.devstatus = DEVNORMALLY;
      /* 赋值设备号 */
      memcpy(eDevTypeParam.wFlash.paramdata,FLASHID,3);
      memcpy(statstr,"OK",sizeof("OK"));
      user_main_debug("gt60m2 check status: %s, data: %s",statstr,FLASHID);
      return 0;
    }
    else
    {
      /* FLASH异常 */
      eDevTypeParam.wFlash.devstatus = DEVEEROR;
      memcpy(statstr,"FAIL",sizeof("FAIL"));
    }
    user_main_debug("gt60m2 check status: %s, data: %s",statstr,FLASHID);
    return 1;
  case gt60m2check:
    /* 读取字库芯片版本 */
    r_dat_bat(0x93,8,gcheckdata);
    memset(statstr,0,10);
    if(gcheckdata[0] == 0x18)
    {
      eDevTypeParam.gT60M2Dev.devstatus = DEVNORMALLY;
      memcpy(statstr,"OK",sizeof("OK"));
      user_main_debug("gt60m2 check status: %s, data: %s",statstr,gcheckdata);
      return 0;
    }
    else
    {
      eDevTypeParam.gT60M2Dev.devstatus = DEVEEROR;
      memcpy(statstr,"FAIL",sizeof("FAIL"));
    }
    user_main_debug("gt60m2 check status: %s, data: %s",statstr,gcheckdata);
    return 1;
  case blecheck:
    /* 读取蓝牙芯片版本号等 */
      //发送指令
    /*等待获取,区分串口指令类型*/
    if(osMessageQueueGet(QueueEventHandle, &pevent, NULL, 1000)==osOK)
    {
      if(pevent == BLEEVENT)
      {
        /*对比字符是否符合*/
        // if()
        {
          /*打印接收数据LOG*/
          LOG("ble RX:");
          for(int i=0;i<BLEUart2RxCnt;i++)
            LOG(" %002x",BLEUart2RxData[i]);
          LOG("\r\n");
          /*判断接收数据是否正确*/

          Clear_BLEUart_Data();
          return 0;  
        }
        // else
        // {

        //   return 1
        // }
      }
      return 1;       
    }  
    else
    {
      /* 如果超时并未接收到数据赋值失败，并返回错误 */

      return 1;
    }
  case loracheck:
  /* 读取Lora相关状态与版本号 */
    //发送指令
  /*等待获取,区分串口指令类型*/
  if(osMessageQueueGet(QueueEventHandle, &pevent, NULL, 1000)==osOK)
  {
    if(pevent == BLEEVENT)
    {
      /*对比字符是否符合*/
      // if()
      {
        /*打印接收数据LOG*/
        LOG("ble RX:");
        for(int i=0;i<BLEUart2RxCnt;i++)
          LOG(" %002x",BLEUart2RxData[i]);
        LOG("\r\n");
        /*判断接收数据是否正确*/

        Clear_BLEUart_Data();
        return 0;  
      }
      // else
      // {

      //   return 1
      // }
    }
    return 1;       
  }  
  else
  {
    /* 如果超时并未接收到数据赋值失败，并返回错误 */
    
    return 1;
  }
  break;
  case batterycheck:
    /* 获取电池电量 */
    Get_Battery_Val(&batval);
    if(batval == 0)
    {
      user_main_debug("%d",batval);
      return 1;
    }
    else
    {
      user_main_debug("Battery get Fail!");
      return 0; 
    }           
  default:
    return 1;
  }
}



/*******************************************************************
 *
 *桌牌上电自检功能
 *
********************************************************************/
void selfcheckTask(void *argument)
{
  CHECKDEV selfcheck_type = flashcheck;
  
  /*初始化相关硬件，用于自检*/
  EventQueueInit();
  BLE_uart_task_init();
  GetParamQueue_Init();
  FLASH_Init();  
  memset(&eDevTypeParam,0,sizeof(DevTypeParam_t));

  while(1)
  {
    if(selfcheck_type < 5)
    {
      /*如果未获得正确数值则跳出重复循环*/
      for(int i=0;i<3;i++)
      {
        if(selfcheck_config(selfcheck_type))
        {

        }
        else
        {
          /*进行下一次检测*/
          selfcheck_type++;
          break;
        }
      }
    }
    else
    {
      // /*判断蓝牙或者LORA通讯等为正常*/

      // /*判断主要模组为异常则保持出厂模式进入休眠*/
      // /*外部事件分配任务*/
      // extevenTaskHandle = osThreadNew(ext_eventTask, NULL, &extevenTask_attributes);
      // /*桌牌显示任务*/
      // tablesignTaskHandle = osThreadNew(tablesignTask, NULL, &tablesignTask_attributes);      
      
    }       
  }
}




