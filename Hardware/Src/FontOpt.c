#include "cmsis_os.h"
#include "FontOpt.h"
#include "spi.h"
#include "GT60M2.h"
#include "w25q128.h"
#include "tablesign.h"
#include "dataconfig.h"


extern unsigned char pBits[192*192/8];

extern uint8_t comPicFinishFlag;
// /*定义文字起始位置*/
// uint32_t  Textlocat=820;
// uint32_t  Textsize = 192;

///*图片存储线程*/
//osThreadId_t picinputTaskHandle;
//const osThreadAttr_t picinputTask_attributes = {
//  .name = "picinputTask",
//  .priority = (osPriority_t) osPriorityNormal,
//  .stack_size = 1024 * 4
//};


/*起始位置地址计算公式： BLACKTEXTBASE + (Textlocat/TABLEROW)*100 + Textlocat%TABLEROW*/
/*根据变量i的变化*/
//BLACKTEXTBASE + ((Textlocat/TABLEROW)+i)*100 + Textlocat%TABLEROW
//for(int i=0;i<Textsize,i++)
//{
//  /*写每一个的每一行到flash中，每一行的起始地址为 （BLACKTEXTBASE + ((Textlocat/TABLEROW)+i)*100 + Textlocat%TABLEROW）*/
//  /*需要判断文字大小是否超出100-Textlocat%TABLEROW 是否大于 Textsize*/
//}

/*擦除对应的文字扇区或者图片扇区,saddr为地址，num为删除几个扇区*/
void TableSectorErase(uint32_t saddr,uint8_t num)
{
  for(int i=0;i<num;i++)
    W25X_SectorErase((saddr/4096)+i);
}


/*根据起始位置将文字写入外部FLASH*/
void InputTexttoFlash(Fontsparam fontdata)
{
  int i;
  /*清空取字体值前的整体数据*/
  memset(pBits,0,192*192/8);
  // TableSectorErase();
  for(int q=0;q<fontdata.Textcount;q++)
  {
    if(fontdata.Textdata[2*q] == 0x00)
      get_font(pBits,VEC_HZ_ASCII_STY,0,fontdata.Textdata[2*q+1],fontdata.Textsize, fontdata.Textsize, fontdata.Textsize);
    else
      get_font(pBits,fontdata.TextFont,fontdata.Textdata[2*q],fontdata.Textdata[2*q+1],fontdata.Textsize, fontdata.Textsize, fontdata.Textsize);
    for(int m=0;m<(fontdata.Textsize*fontdata.Textsize/8);m++)pBits[m]=~pBits[m];   
    for(i=0;i<fontdata.Textsize;i++)
    {
      /* 文字与字母的字体间隔不同需要区分对待 */
      uint32_t addr = (((fontdata.Textlocat/TABLEROW)+i)*TABLEROW) + fontdata.Textlocat%TABLEROW;  //BLACKTEXTBASE  + (q*fontdata.Textsize/8) +  
      if(fontdata.Textdata[2*q] != 0x00)
        addr += (q*fontdata.Textsize/8);
      else
        addr += (q*fontdata.Textsize/16);

      /*判断A面与B面 设置字体文字颜色*/
      if(fontdata.Texttableface == 0x03)
      {
        if(fontdata.Textcolour == 0x01)addr += B_BLACKTEXTBASE;
        else if(fontdata.Textcolour == 0x02) addr += B_WHITETEXTBASE;
        else if(fontdata.Textcolour == 0x03) addr += B_REDTEXTBASE;
        else addr += B_BLACKTEXTBASE;      
      }
      else
      {
        if(fontdata.Textcolour == 0x01)addr += A_BLACKTEXTBASE;
        else if(fontdata.Textcolour == 0x02) addr += A_WHITETEXTBASE;
        else if(fontdata.Textcolour == 0x03) addr += A_REDTEXTBASE;
        else addr += A_BLACKTEXTBASE;         
      }
      
      /*方法一*/
      /*因为只能以256个字节做一个写入单位，剩余字节大于要写的字节时可直接写入，小于时逐个写入*/
      if((256-(addr%256)) > fontdata.Textsize/8)
        W25X_FLASH_PageWrite(&pBits[(i*fontdata.Textsize/8)],addr,fontdata.Textsize/8);
      else
      {
        for(int n=0;n<fontdata.Textsize/8;n++)
        {
          W25X_FLASH_PageWrite(&pBits[(i*fontdata.Textsize/8)+n],addr+n,1);
          osDelay(1);
        }
      }
      /*方法二*/
      // for(int n=0;n<fontdata.Textsize/8;n++)
      // {
      //   W25X_FLASH_PageWrite(&pBits[(i*fontdata.Textsize/8)+n],addr+n,1);
      //   osDelay(1);
      // }
    } 
  }
}


/*************************************************************************************
 * 
 * 接收图片写入外部Flash任务
 * 
*************************************************************************************/
// void pic_input_Task(void *argument)
// {
//   picDataParm rdat;
//   while(1)
//   {
//     if(!GetPicData_Queue_Event(&rdat))
//       Picture_param_write(rdat.tfac,rdat.tdata,rdat.tdatlen);
//   }
// }

///*************************************************************************************
// * 
// * 接收图片写入外部Flash任务
// * 
//*************************************************************************************/
//void pic_input_Task(void *argument)
//{
//  picDataParm rdat;
//  picDataParm *rldat;
//  while(1)
//  {
//    if(!GetPicData_Queue_Event(&rdat))
//    {
//      rldat=(picDataParm*)pvPortMalloc(sizeof(picDataParm));
//      Picture_param_write(rdat.tfac,rdat.tdata,rdat.tdatlen);
//      // uncompress_data_toflash(rdat.tfac,rdat.tdata,rdat.tdatlen);
//    }
//    while(1)
//    {
//      if(!GetPicDataING_Queue_Event(&rdat))
//      {
//        rldat->tfac = rdat.tfac;
//        memcpy(rldat->tdata,rdat.tdata,sizeof(rdat.tdata));
//        // uncompress_data_toflash(rdat.tfac,rdat.tdata,rdat.tdatlen);
//        Picture_param_write(rdat.tfac,rdat.tdata,rdat.tdatlen);
//      }
//      else
//      {
//        comPicFinishFlag = 1;
//        user_main_debug("rx finish!!!!!!!!!!!!!!!!!!!!!!!!\n");
//        // uncompress_data_toflash(rldat->tfac,rldat->tdata,0);
//        Picture_param_write(rldat->tfac,rldat->tdata,0);
//        vPortFree(rldat);
//        break;
//      }
//    }
//  }
//}

///*初始化创建图片写入线程*/
//int pic_input_init(void)
//{
//  if(!picinputTaskHandle)
//    picinputTaskHandle = osThreadNew(pic_input_Task, NULL, &picinputTask_attributes);
//  else
//  {
//    user_main_info("Picture Input Task Creat Fail!");
//    return 1;
//  }
//  user_main_info("Picture Input Task Creat Success!");
//  return 0;  
//}




