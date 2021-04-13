#ifndef __W25Q128_H
#define __W25Q128_H


#define  sFLASH_ID              0XEF4017    //W25Q64


/*命令定义-开头*******************************/
#define W25X_WriteEnable		      0x06 
#define W25X_WriteDisable		      0x04 
#define W25X_ReadStatusReg		      0x05 
#define W25X_WriteStatusReg		      0x01 
#define W25X_ReadData			      0x03 
#define W25X_FastReadData		      0x0B 
#define W25X_FastReadDual		      0x3B 
#define W25X_PageProgram		      0x02 
#define W25X_BlockErase			      0xD8 
#define W25X_SectorErase4KB		      0x20 
#define W25X_SectorErase32KB		      0x52
#define W25X_SectorErase64KB		      0xD8
#define W25X_ChipErase			      0xC7 
#define W25X_PowerDown			      0xB9 
#define W25X_ReleasePowerDown	              0xAB 
#define W25X_DeviceID			      0xAB
#define W25X_ManufactDeviceID   	      0x90 
#define W25X_JedecDeviceID		      0x9F

/* WIP(busy)标志，FLASH内部正在写入 */
#define WIP_Flag                              0x01
#define Dummy_Byte                            0xFF
/*命令定义-结尾*******************************/

#define 	W25X_FLASH_SIZE	            (1*1024*1024)	// 1M字节
#define		W25X_PAGE_SIZE			8192	        // 256 bytes
#define 	W25X_SECTOR_SIZE		512	        // 4-Kbyte
#define		W25X_BLOCK_SIZE		        32	        // 64-Kbyte	

#define        W25X_CS_HIGH()            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET)  
#define        W25X_CS_LOW()             HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET)



void W25X_SectorErase(uint32_t SectorAddr);
void W25X_FLASH_PageWrite(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);
void W25X_BufferRead(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead);
void W25X_FLASH_ReadID(uint8_t* pBuffer, uint16_t NumByteToRead);

#endif



