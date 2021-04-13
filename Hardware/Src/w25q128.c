#include "spi.h"
#include "w25q128.h"
#include "cmsis_os.h"


 /************************************
  * @brief  向Flash芯片发送写1byte数据
  * @param  dat 一个字节
  * @retval none
  ***********************************/
void W25X_Write_byte(uint8_t dat)
{
  /* 发送1byte数据*/
  HAL_SPI_Transmit(&hspi1,&dat,1,0xff);
}

 /************************************
  * @brief  向Flash芯片发送写nbyte数据
  * @param  dat 数据  len 字节个数
  * @retval none
  ***********************************/
void W25X_Write_data(uint8_t *dat, uint32_t len)
{
  /* 发送len byte数据*/
  HAL_SPI_Transmit(&hspi1,dat,len,0xff);
}

 /************************************
  * @brief  读Flash芯片nbyte数据
  * @param  dat 数据  len 字节个数
  * @retval none
  ***********************************/
void W25X_Read_data(uint8_t *dat, uint32_t len)
{
  /* 发送len byte数据*/
  HAL_SPI_Receive(&hspi1,&dat[0],len,0xff);
}



 /************************************************************************
  * @brief  等待WIP(BUSY)标志被置0，即等待到FLASH内部数据写入完毕
  * @param  none
  * @retval none
  *************************************************************************/
void W25X_WaitForWriteEnd(void)
{
  uint8_t FLASH_Status = 0;

  /* 选择 FLASH: CS 低 */
  W25X_CS_LOW();
  /* 发送 读状态寄存器 命令 */
  W25X_Write_byte(W25X_ReadStatusReg);
  /* 若FLASH忙碌，则等待 */
  do
  {
  /* 读取FLASH芯片的状态寄存器 */
    W25X_Read_data(&FLASH_Status,1);	 
  }
  while ((FLASH_Status & WIP_Flag) == 1);  /* 正在写入标志 */
  /* 停止信号  FLASH: CS 高 */
  W25X_CS_HIGH();
}

/************************************************************
  * @brief  擦除FLASH扇区
  * @param  SectorAddr：要擦除的扇区 第几个扇区
  * @retval 无
  **********************************************************/
void W25X_SectorErase(uint32_t SectorAddr)
{  
  /* 选择 FLASH: CS 低 */
  W25X_CS_LOW();  
  /* 发送FLASH写使能命令 */
  W25X_Write_byte(W25X_WriteEnable);
  /* 停止信号  FLASH: CS 高 */
  W25X_CS_HIGH();  
  /* 等待执行写完毕*/
  W25X_WaitForWriteEnd();
  /*选择 FLASH: CS 低 */
  W25X_CS_LOW();  
  /* 擦除扇区 */
  /* 发送扇区擦除指令*/
  W25X_Write_byte(W25X_SectorErase4KB); //4KB删除
  /*发送擦除扇区地址的高位*/
  W25X_Write_byte((SectorAddr*4096 & 0xFF0000) >> 16);
  /* 发送擦除扇区地址的中位 */
  W25X_Write_byte((SectorAddr*4096 & 0xFF00) >> 8);
  /* 发送擦除扇区地址的低位 */
  W25X_Write_byte(SectorAddr*4096 & 0xFF);
  /* 停止信号  FLASH: CS 高 */
  W25X_CS_HIGH();
  /* 等待擦除完毕*/
  W25X_WaitForWriteEnd();

}



/*********************************************************************************************
  * @brief  对FLASH按页写入数据，调用本函数写入数据前需要先擦除扇区
  * @param	pBuffer，要写入数据的指针
  * @param WriteAddr，写入地址
  * @param  NumByteToWrite，写入数据长度，必须小于等于SPI_FLASH_PerWritePageSize
  * @retval 无
  ********************************************************************************************/
void W25X_FLASH_PageWrite(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
  /* 选择 FLASH: CS 低 */
  W25X_CS_LOW();  
  /* 发送FLASH写使能命令 */
  W25X_Write_byte(W25X_WriteEnable);
  /* 停止信号  FLASH: CS 高 */
  W25X_CS_HIGH(); 
  /* 等待执行写完毕*/
  W25X_WaitForWriteEnd();  
  
  /* 选择 FLASH: CS 低 */
  W25X_CS_LOW();  
  /* 写页写指令*/
  W25X_Write_byte(W25X_PageProgram);
  /*发送写地址的高位*/
  W25X_Write_byte((WriteAddr & 0xFF0000) >> 16);
  /*发送写地址的中位*/
  W25X_Write_byte((WriteAddr & 0xFF00) >> 8);
  /*发送写地址的低位*/
  W25X_Write_byte(WriteAddr & 0xFF);

  /* 写入数据*/
  W25X_Write_data(pBuffer,NumByteToWrite);
  /* 停止信号  FLASH: CS 高 */
  W25X_CS_HIGH();

  /* 等待写入完毕*/
  W25X_WaitForWriteEnd();
}


 /******************************************************************************
  * @brief  读取FLASH数据
  * @param 	pBuffer，存储读出数据的指针
  * @param   ReadAddr，读取地址
  * @param   NumByteToRead，读取数据长度
  * @retval 无
  ******************************************************************************/
void W25X_BufferRead(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead)
{ 
    // osMutexAcquire(exFlashMutexHandle,0);
  /* 选择 FLASH: CS 低 */
  W25X_CS_LOW(); 
  /* 发送 读 指令 */
  W25X_Write_byte(W25X_ReadData);
  /*发送写地址的高位*/
  W25X_Write_byte((ReadAddr & 0xFF0000) >> 16);
  /*发送写地址的中位*/
  W25X_Write_byte((ReadAddr & 0xFF00) >> 8);
  /*发送写地址的低位*/
  W25X_Write_byte(ReadAddr & 0xFF);
  /* 读取数据 */
  W25X_Read_data(pBuffer,NumByteToRead);
  /* 停止信号  FLASH: CS 高 */
  W25X_CS_HIGH(); 
  // osMutexRelease(exFlashMutexHandle);    
}


 /****************************************
  * @brief  读取FLASH ID
  * @param 	无
  * @retval FLASH ID
  ****************************************/
void W25X_FLASH_ReadID(uint8_t* pBuffer, uint16_t NumByteToRead)
{
  W25X_CS_LOW();
  /* 发送JEDEC指令，读取ID */
  W25X_Write_byte(W25X_JedecDeviceID);  //W25X_ManufactDeviceID
  
  /* 读取数据 */
  W25X_Read_data(&pBuffer[0],NumByteToRead);
  W25X_CS_HIGH();
}







