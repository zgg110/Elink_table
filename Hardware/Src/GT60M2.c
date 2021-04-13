#include "gpio.h"
#include "spi.h"
#include "GT60M2.h"





 /************************************
  * @brief  ��GT60M2оƬ����д1byte����
  * @param  dat һ���ֽ�
  * @retval none
  ***********************************/
void GT60M2_Write_byte(uint8_t dat)
{
  /* ����1byte����*/
  HAL_SPI_Transmit(&hspi1,&dat,1,0xff);
}

 /************************************
  * @brief  ��GT60M2оƬ����дnbyte����
  * @param  dat ����  len �ֽڸ���
  * @retval none
  ***********************************/
void GT60M2_Write_data(uint8_t *dat, uint32_t len)
{
  /* ����len byte����*/
  HAL_SPI_Transmit(&hspi1,dat,len,0xff);
}

 /************************************
  * @brief  ��GT60M2оƬnbyte����
  * @param  dat ����  len �ֽڸ���
  * @retval none
  ***********************************/
void GT60M2_Read_data(uint8_t *dat, uint32_t len)
{
  /* ����len byte����*/
  HAL_SPI_Receive(&hspi1,&dat[0],len,0xff);
}

 /************************************
  * @brief  ��GT60M2оƬ1byte����
  * @param  dat ���� 
  * @retval return �����ֽ�
  ***********************************/
uint8_t GT60M2_Read_byte(void)
{
  uint8_t Data;
  uint8_t TXData=0xFF;
//  HAL_SPI_Receive(&hspi1,&Data,1,0xff);
  HAL_SPI_TransmitReceive(&hspi1,&TXData,&Data,1,0xff);
  return Data;
  
}

 /************************************
  * @brief  ��λGT60M2ָ��
  * @param  none
  * @retval none
  ***********************************/
 void GT60M2_Reset_Cmd(void)
 {
   GT60M2_CS_LOW();
   GT60M2_Write_byte(0x66);
   GT60M2_CS_HIGH();
   GT60M2_CS_LOW();
   GT60M2_Write_byte(0x99);
   GT60M2_CS_HIGH();
 }


/*
*??:???????? address ?? byte_long ???? p_arr ??????
*/
unsigned char r_dat_bat(unsigned long int address,unsigned char byte_long,unsigned char *p_arr)
{
  uint32_t j=0;
  
  GT60M2_CS_LOW();
  
  GT60M2_Write_byte(0x03);
  GT60M2_Write_byte((unsigned char)(address>>16));
  GT60M2_Write_byte((unsigned char)(address>>8));
  GT60M2_Write_byte((unsigned char)(address>>0));
//  for(j=0;j<byte_long;j++)
//  {
//    GT60M2_Read_data(&p_arr[j],1);
////    p_arr[j] = SpiReadByte(0);
//  }  ��
  for(j=0;j<byte_long;j++)
  {
//    GT60M2_Read_data(&p_arr[j],1);
//    p_arr[j] = SpiReadByte(0);
    p_arr[j] = GT60M2_Read_byte();
  }   
//  GT60M2_Read_data(p_arr,byte_long);
  GT60M2_CS_HIGH();
  return p_arr[0];	
}



unsigned char r_dat(unsigned long int address)
{
  unsigned char dat=0;
  r_dat_bat(address,1,&dat);
  
  return dat;
}








