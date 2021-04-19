#include "stdio.h"
#include "gpio.h"
#include "usart.h"
#include "cmsis_os.h"
#include "tablesign.h"
#include "string.h"
#include "FontOpt.h"
#include "spi.h"
#include "w25q128.h"
#include "GT60M2.h"
//#include "flashConfig.h"
//#include "blecomm.h"
//#include "dataconfig.h"
//#include "FontOpt.h"

/*是否正在投屏显示标志位 置位为1*/
uint8_t Displayflag = 0;
/*定义当前显示的为哪个面*/
TableFace eTablefac;

extern unsigned char pBits[192*192/8];
//extern Fontsparam fontdata1;

extern osMessageQueueId_t GetParamHandle;
extern uint8_t PicandTextflag;
//extern WAITPICSTAT eWaitPicSta;
extern uint8_t eTableDisplayType;

extern uint8_t Picfinish;

/**********************************************************
 *
 *屏幕显示相关引脚初始化
 *
 **********************************************************/
int Table_HandWare_Init(FunctionalState sta)
{
  if(sta)
  {
    /*相关引脚已经被初始化在main.c中*/
    /*SPI2初始化*/
    MX_SPI2_Init(); 
  }
  else
  {
    /*将SPI恢复为默认状态*/
    HAL_SPI_MspDeInit(&hspi2);
    /*将其他端口也恢复为低功耗状态*/
    
  }
  return 0;
}


/***************************************************************
 *
 *桌牌显示初始化信号
 *
*****************************************************************/
void TableSignInit(TableFace fac)
{
  TABLE_BS_LOW();
  switch(fac){
    case TabFaceA:
      TABLEA_RES_LOW();
      break;
    case TabFaceB:
      TABLEB_RES_LOW();
      break;
    default:
      TABLEA_RES_LOW();
      TABLEB_RES_LOW();
      break;}  
  HAL_Delay(1000);
  switch(fac){
    case TabFaceA:
      TABLEA_RES_HIGH();
      break;
    case TabFaceB:
      TABLEB_RES_HIGH();
      break;
    default:
      TABLEA_RES_HIGH();
      TABLEB_RES_HIGH();
      break;}
  osDelay(1000);
}


/*******************************************************************
 *
 *写1BYTE数据到桌牌
 *
 *********************************************************************/
void TableSignWriteByte(TableFace fac,DataType typ,uint8_t data)
{
  switch(fac){
    case TabFaceA:
      TABLEA_NSS_LOW();
      break;
    case TabFaceB:
      TABLEB_NSS_LOW();
      break;
    default:
      TABLEA_NSS_LOW();
      TABLEB_NSS_LOW();
      break;}
  /*选择发送的是命令还是数据*/
  if(typ == SendDATA)
    TABLE_DC_HIGH();
  else
    TABLE_DC_LOW();
  HAL_SPI_Transmit(&hspi2,&data,1,0xff);
    
  switch(fac){
    case TabFaceA:
      TABLEA_NSS_HIGH();
      break;
    case TabFaceB:
      TABLEB_NSS_HIGH();
      break;
    default:
      TABLEA_NSS_HIGH();
      TABLEB_NSS_HIGH();
      break;}    
}

/*******************************************************************
 *
 *写n BYTE数据到桌牌
 *
 *********************************************************************/
void TableSignWriteData(TableFace fac,uint8_t *data,uint32_t datalen)
{
  switch(fac){
    case TabFaceA:
      TABLEA_NSS_LOW();
      break;
    case TabFaceB:
      TABLEB_NSS_LOW();
      break;
    default:
      TABLEA_NSS_LOW();
      TABLEB_NSS_LOW();
      break;}

  TABLE_DC_HIGH();
  HAL_SPI_Transmit(&hspi2,data,datalen,0xff);
    
  switch(fac){
    case TabFaceA:
      TABLEA_NSS_HIGH();
      break;
    case TabFaceB:
      TABLEB_NSS_HIGH();
      break;
    default:
      TABLEA_NSS_HIGH();
      TABLEB_NSS_HIGH();
      break;}    
}

/*******************************************************************
 *
 *写数据是否忙检测
 *
********************************************************************/
int CheckTableBusy(TableFace fac,uint32_t timeout)
{
  uint32_t tim=0;
  switch(fac){
    case TabFaceA:
      while(tim++ < timeout)
      {
        if(TABLEA_BUSY() != 0)return 0;
        // osDelay(1); 
      }
      break;
    case TabFaceB:
      while(tim++ < timeout)
      {
        if(TABLEB_BUSY() != 0)return 0;
        // osDelay(1);
      }
      break;
    default:
      while(tim++ < timeout)
      {
        if((TABLEA_BUSY() != 0)||(TABLEB_BUSY() != 0))return 0;
        // osDelay(1);
      }
      break;
   }  
  return 1;
}


/*******************************************************************
 *
 *显示文字与图片像素到800x480像素的屏幕上    //需要加入图片编号
 *
********************************************************************/
void Display_Pix_Write(TableFace fac, uint8_t pict, uint8_t *data)
{
  int picfd=0;
  uint8_t rxdat1[PIC_ROW_SIZE];
  uint8_t rxdat2[PIC_ROW_SIZE];
  uint8_t rxdat3[PIC_ROW_SIZE];
  uint8_t rxdat4[PIC_ROW_SIZE];
  uint8_t rxdat5[2];
  uint32_t wbaddr,redaddr;
  uint32_t beforaddr=0,afterdaddr=0;
  uint8_t deffval;   //单次差值变量
  uint8_t valdown;   //总量递减变化值  
  uint16_t num=0;
  /*根据图片编号选择FLASH芯片内图片的地址*/
  wbaddr = Get_pic_Addr(fac, pict,WBColorPic);
  /*宽度断行*/
  {
    /*写黑白到屏幕*/
    TableSignWriteByte(fac,SendCmd,0x10);
    for(int i=0;i<TABLECOLUMN;i++)
    {
        // W25X_BufferRead(rxdat4,wbaddr+i*PIC_ROW_SIZE,100); 
        // for (size_t a = 0; a < PIC_ROW_SIZE; a++)
        // {
        //   LOG("%02x ",rxdat4[a]);
        // }
        

        if(fac == TabFaceB)
        {
          W25X_BufferRead(rxdat1, B_BLACKTEXTBASE + i*PIC_ROW_SIZE, PIC_ROW_SIZE);
          W25X_BufferRead(rxdat2, B_WHITETEXTBASE + i*PIC_ROW_SIZE, PIC_ROW_SIZE);
        }
        else
        {
          W25X_BufferRead(rxdat1, A_BLACKTEXTBASE + i*PIC_ROW_SIZE, PIC_ROW_SIZE);
          W25X_BufferRead(rxdat2, A_WHITETEXTBASE + i*PIC_ROW_SIZE, PIC_ROW_SIZE);       
        }
        // W25X_BufferRead(rxdat3, wbaddr + i*PIC_ROW_SIZE, PIC_ROW_SIZE);
        memset(rxdat3, 0, PIC_ROW_SIZE);  
        valdown = PIC_ROW_SIZE;
        /*当递减值为0时跳出*/
        while(valdown)
        {
          // user_main_debug("befor valdown:%d\n",valdown);
          /*相减等于0则跳出循环*/
          if(afterdaddr == beforaddr)
          {
            /*读取两个字节*/
            W25X_BufferRead(rxdat5,wbaddr+picfd*2,2);   
            //  user_main_debug("flash data %02x %02x",rxdat5[0],rxdat5[1]);         
            beforaddr = rxdat5[1] + afterdaddr;
            picfd++;
            // user_main_debug("data get size:%d\n",picfd);            
          }          
          if((beforaddr-afterdaddr) < valdown)     //beforaddr - deffval
          {
            deffval = beforaddr-afterdaddr;
            /*每次数组为100有相应的字节个数写入后递减*/  
            valdown -= deffval;             
            afterdaddr += deffval;    //deffval += deffval;
          }         
          else if((beforaddr-afterdaddr) >= valdown)
          {
            /*确定要写入的字节个数*/
            deffval = valdown;
            valdown -= valdown;
            afterdaddr += deffval;          
          } 
          // user_main_debug("after valdown: %d,beforaddr: %08x,afterdaddr: %08x\n",valdown,beforaddr,afterdaddr);
          memset(&rxdat3[100-valdown-deffval], rxdat5[0], deffval);
        } 
        // for(int n=0;n<100;n++)
        // {
        //   LOG(" %02x",rxdat3[n]);
        // }
        // LOG("\r\n");
        /*需要读取图片位置的像素同时将黑白显示到屏幕*/
        for(int j=0;j<PIC_ROW_SIZE;j++)rxdat1[j]=(rxdat3[j] & rxdat1[j] | ~rxdat2[j]);  //| rxdat2[j]
        TableSignWriteData(fac,rxdat1,PIC_ROW_SIZE);
        num++;
      //  user_main_debug("Display one PIC_ROW_SIZE Finish: %d\n",num);
    }
  }
  redaddr = Get_pic_Addr(fac, pict,RedColorPic);
  afterdaddr = 0;
  beforaddr = 0;
  picfd = 0;
  num = 0;
  {
    /*写红色到屏幕*/
    TableSignWriteByte(fac,SendCmd,0x13);
    for(int i=0;i<TABLECOLUMN;i++)
    {
        // W25X_BufferRead(rxdat4,redaddr+i*PIC_ROW_SIZE,100); 
        // for (size_t a = 0; a < PIC_ROW_SIZE; a++)
        // {
        //   LOG("%02x ",rxdat4[a]);
        // }        
        
        /*需要判断文字还是图片*/
        if(fac == TabFaceB)
        {
          W25X_BufferRead(rxdat1, B_BLACKTEXTBASE + i*PIC_ROW_SIZE, PIC_ROW_SIZE);
          W25X_BufferRead(rxdat2, B_WHITETEXTBASE + i*PIC_ROW_SIZE, PIC_ROW_SIZE);        
          W25X_BufferRead(rxdat3, B_REDTEXTBASE + i*PIC_ROW_SIZE, PIC_ROW_SIZE);
        }
        else
        {
          /* A面或者AB面同时显示 */
          W25X_BufferRead(rxdat1, A_BLACKTEXTBASE + i*PIC_ROW_SIZE, PIC_ROW_SIZE);
          W25X_BufferRead(rxdat2, A_WHITETEXTBASE + i*PIC_ROW_SIZE, PIC_ROW_SIZE);        
          W25X_BufferRead(rxdat3, A_REDTEXTBASE + i*PIC_ROW_SIZE, PIC_ROW_SIZE);         
        }
        /*需要读取图片位置的像素同时将红色显示到屏幕*/
        // W25X_BufferRead(rxdat4, redaddr + i*PIC_ROW_SIZE, PIC_ROW_SIZE);
        memset(rxdat4, 0, PIC_ROW_SIZE);  
        valdown = PIC_ROW_SIZE;
        /*当递减值为0时跳出*/
        while(valdown)
        {
          // user_main_debug("befor valdown:%d\n",valdown);
          /*相减等于0则跳出循环*/
          if(afterdaddr == beforaddr)
          {
            /*读取两个字节*/
            W25X_BufferRead(rxdat5,redaddr+picfd*2,2);   
            // user_main_debug("flash data %02x %02x",rxdat5[0],rxdat5[1]);         
            beforaddr = rxdat5[1] + afterdaddr;
            picfd++;
            // user_main_debug("data get size:%d\n",picfd);            
          }          
          if((beforaddr-afterdaddr) < valdown)     //beforaddr - deffval
          {
            deffval = beforaddr-afterdaddr;
            /*每次数组为100有相应的字节个数写入后递减*/  
            valdown -= deffval;             
            afterdaddr += deffval;    //deffval += deffval;
          }         
          else if((beforaddr-afterdaddr) >= valdown)
          {
            /*确定要写入的字节个数*/
            deffval = valdown;
            valdown -= valdown;
            afterdaddr += deffval;          
          } 
          // user_main_debug("after valdown: %d,beforaddr: %08x,afterdaddr: %08x\n",valdown,beforaddr,afterdaddr);
          memset(&rxdat4[100-valdown-deffval], rxdat5[0], deffval);
        } 
        // for(int n=0;n<100;n++)
        // {
        //   LOG(" %02x",rxdat4[n]);
        // }
        // LOG("\r\n");     

        for(int j=0;j<PIC_ROW_SIZE;j++)rxdat1[j]=(rxdat4[j] & rxdat1[j] & rxdat2[j] | ~rxdat3[j]);  //& ~rxdat2[j]
        TableSignWriteData(fac,rxdat1,PIC_ROW_SIZE);
        num++;
        // user_main_debug("Display one PIC_ROW_SIZE Finish: %d\n",num);        
    }    
  }
}

/*******************************************************************
 *
 *显示图片像素到800x480像素的屏幕上    //需要加入图片编号
 *
********************************************************************/
void Display_Pic_Write(TableFace fac, uint8_t pict, uint8_t *data)
{
  uint8_t rxdat1[PIC_ROW_SIZE];

  uint32_t wbaddr,redaddr;
  /*根据图片编号选择FLASH芯片内图片的地址*/
  wbaddr = Get_pic_Addr(fac, pict,WBColorPic);
  /*宽度断行*/
  {
    /*写黑白到屏幕*/
    TableSignWriteByte(fac,SendCmd,0x10);
    for(int i=0;i<TABLECOLUMN;i++)
    {
        /*需要读取图片位置的像素同时将黑白显示到屏幕*/
        W25X_BufferRead(rxdat1, wbaddr + i*PIC_ROW_SIZE, PIC_ROW_SIZE);
        TableSignWriteData(fac,rxdat1,PIC_ROW_SIZE);
    }
  }
  redaddr = Get_pic_Addr(fac, pict,RedColorPic);
  {
    /*写红色到屏幕*/
    TableSignWriteByte(fac,SendCmd,0x13);
    for(int i=0;i<TABLECOLUMN;i++)
    {
        /*需要读取图片位置的像素同时将红色显示到屏幕*/
        W25X_BufferRead(rxdat1, redaddr + i*PIC_ROW_SIZE, PIC_ROW_SIZE);
        TableSignWriteData(fac,rxdat1,PIC_ROW_SIZE);
    }    
  }
}

/*******************************************************************
 *
 *写文字到桌牌设置
 *
********************************************************************/
void TableSignSeting(TableFace fac, uint8_t pict,TableDisplayType dis)
{
  TableSignInit(fac);
  osDelay(300);
  TableSignWriteByte(fac,SendCmd,POWR);
  TableSignWriteByte(fac,SendDATA,0x07);
  TableSignWriteByte(fac,SendDATA,0x17);
  TableSignWriteByte(fac,SendDATA,0x3f);
  TableSignWriteByte(fac,SendDATA,0x3f);
  
  TableSignWriteByte(fac,SendCmd,PON);
  osDelay(300);
  /*加入busy引脚检测*/
  if(CheckTableBusy(fac,200))return;
  
  TableSignWriteByte(fac,SendCmd,PSR);
  TableSignWriteByte(fac,SendDATA,0x0f);
  
  TableSignWriteByte(fac,SendCmd,0x61);
  TableSignWriteByte(fac,SendDATA,0x03);
  TableSignWriteByte(fac,SendDATA,0x20);
  TableSignWriteByte(fac,SendDATA,0x01);
  TableSignWriteByte(fac,SendDATA,0xe0);
  
  TableSignWriteByte(fac,SendCmd,0x15);
  TableSignWriteByte(fac,SendDATA,0x00);
  
  TableSignWriteByte(fac,SendCmd,0x60);
  TableSignWriteByte(fac,SendDATA,0x22);
  
  TableSignWriteByte(fac,SendCmd,0x50);
  TableSignWriteByte(fac,SendDATA,0x11);
  TableSignWriteByte(fac,SendDATA,0x07);
  
  user_main_info("TextOrPic Display Start");
  user_main_info("Picture:%d,Display type:%d,Display face:%d",pict,dis,fac);
  /*逐像素写数据*/
  if(dis == TextAndPic)
    Display_Pix_Write(fac,pict,(uint8_t *)"TextAndPic");
  else if(dis == OnlyPic)
    Display_Pic_Write(fac,pict,(uint8_t *)"TextAndPic");
  osDelay(2);
  
  TableSignWriteByte(fac,SendCmd,0x12);
  user_main_info("Picture Display running");
  
  /*设置定时检查是否投屏完毕*/
  while(CheckTableBusy(fac,5)) 
  {
    Displayflag = 1;
    osDelay(500);
    return;
  }
//  if(CheckTableBusy(fac,5))
//  {
//    eTablefac = fac;
//    Displayflag = 1;
//  }

  user_main_info("TextOrPic Display Finish");
  // osDelay(2000);
}

Fontsparam RXdata={1,820,VEC_SONG_STY,192,1,2,{0xcd,0xcf,0xd0,0xac}};

///*******************************************************************
// *
// *写桌牌任务函数
// *
//********************************************************************/
//void tablesignTask(void *argument)
//{
//  PICTRANSTAT picsta;
//  Fontsparam efontdata;
//
////  uint8_t tface = 0;
//  
//  /*初始化相关硬件，用于自检*/
//  EventQueueInit();
//  BLE_uart_task_init();
//  GetParamQueue_Init();
//  FinishParamQueue_Init();
//  PicDataQueue_Init();
//  pic_input_init();
//  FLASH_Init(); 
//
//  osDelay(1000);
//
//
//  while(1)
//  {
//    if(Finishget_Queue_Event() == osOK)
//    {
//      if(osMessageQueueGet(GetParamHandle, &efontdata, NULL, portMAX_DELAY) == osOK)
//      {
//        /*如果efontdata.Textcount为0则直接显示图片*/
//        if(efontdata.Textcount != 0)
//        {
//          InputTexttoFlash(efontdata);
//          user_main_info("Text count:%s",efontdata.Textdata);
//        }
//        else
//        {
//          /*如果为直接显示命令 则进入队列进行等待*/
//          if(eWaitPicSta == WAITPIC)
//          {
//            user_main_info("Just display picture, wait transmission finish");
//            /*获取超时跳出*/
//            while(!Getstat_Queue_Event(&picsta))
//            {
//              /*图片传输完成跳出*/
//              if(picsta == pictfinish)
//              {
//                user_main_info("picture transmission finish");
//                break;
//              }
//            }
//            eTableDisplayType = 0;         
//            /*恢复进入不需要等待传输完成*/
//            eWaitPicSta = NOWAITPIC;           
//            /*传完图片恢复传文字标志位*/
//            PicandTextflag = 0;
//          }
//
//          /*执行投图相关函数*/          
//          switch(efontdata.Texttableface){
//            case TextA:
//              TableSignSeting(TabFaceA,efontdata.Textbackpic,efontdata.eTextandPic);
//              break;
//            case TextB:
//              TableSignSeting(TabFaceB,efontdata.Textbackpic,efontdata.eTextandPic);
//              break;
//            case TextAB:
//              TableSignSeting(TabFaceAB,efontdata.Textbackpic,efontdata.eTextandPic);
//              break;
//            case PictureA:
//              TableSignSeting(TabFaceA,efontdata.Textbackpic,efontdata.eTextandPic);            
//              break;
//            case PictureB:
//              TableSignSeting(TabFaceB,efontdata.Textbackpic,efontdata.eTextandPic);             
//              break;          
//            case PictureAB:
//              TableSignSeting(TabFaceAB,efontdata.Textbackpic,efontdata.eTextandPic);              
//              break;
//            default:break;
//          }               
//        }        
//      } 
//    }     
//  }
//}











