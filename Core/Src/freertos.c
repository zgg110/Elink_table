/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "string.h"
#include "w25q128.h"
#include "dataconfig.h" 
#include "blecomm.h"
#include "gpio.h"
#include "usart.h"
#include "spi.h"
#include "adc.h"    
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
void Lpower_sleep_config(void);
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/*定义无连接计�?*/
uint8_t connttimeout=0;

extern uint8_t Displayflag;

extern UART_HandleTypeDef huart2;

extern Display_Data eDisplay_Data;
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
unsigned char pBits[192*192/8];
/* USER CODE END Variables */
/* Definitions for getBLETask */
osThreadId_t getBLETaskHandle;
const osThreadAttr_t getBLETask_attributes = {
  .name = "getBLETask",
  .priority = (osPriority_t) osPriorityHigh,
  .stack_size = 256 * 24
};
/* Definitions for ConfigdisTask */
osThreadId_t ConfigdisTaskHandle;
const osThreadAttr_t ConfigdisTask_attributes = {
  .name = "ConfigdisTask",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 128 * 8
};
/* Definitions for QueueBLEusart */
osMessageQueueId_t QueueBLEusartHandle;
const osMessageQueueAttr_t QueueBLEusart_attributes = {
  .name = "QueueBLEusart"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void GetBLETask(void *argument);
void configdisTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of QueueBLEusart */
  QueueBLEusartHandle = osMessageQueueNew (1100, sizeof(uint8_t), &QueueBLEusart_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of getBLETask */
  getBLETaskHandle = osThreadNew(GetBLETask, NULL, &getBLETask_attributes);

  /* creation of ConfigdisTask */
  ConfigdisTaskHandle = osThreadNew(configdisTask, NULL, &ConfigdisTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_GetBLETask */
/**
  * @brief  Function implementing the getBLETask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_GetBLETask */
void GetBLETask(void *argument)
{
  /* USER CODE BEGIN GetBLETask */
  uint8_t indata;
  uint8_t ebuff;
  uint32_t BLEUart2RxCnt=0;
  uint8_t  BLEUart2RxData[1124] = {0}; 
  HAL_UART_Receive_IT(&_BLE_USART, &indata, 1);
  
  memset(&eDisplay_Data,0,sizeof(eDisplay_Data));
  /* Infinite loop */
  while(1)
  {
    if(osMessageQueueGet(QueueBLEusartHandle, &ebuff, NULL, portMAX_DELAY) == osOK)
    {
      BLEUart2RxData[BLEUart2RxCnt++] = ebuff;
      while(1)
      {
        /*将获取的数据放入接收字符串中*/        
        if(osMessageQueueGet(QueueBLEusartHandle, &ebuff, NULL, 300) == osOK)
        {
          BLEUart2RxData[BLEUart2RxCnt++] = ebuff;  
        }  
        else
        {         
          /*接收数据完毕处理相应信息*/
          Rev_DataAnalye(BLEEVENT,BLEUart2RxData,BLEUart2RxCnt);
          /*清理相关BUFF*/
          memset(BLEUart2RxData,0,sizeof(BLEUart2RxData));
          BLEUart2RxCnt = 0;
          break;
        }                
      }      
    }
  }
  /* USER CODE END GetBLETask */
}

/* USER CODE BEGIN Header_configdisTask */
/**
* @brief Function implementing the ConfigdisTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_configdisTask */
void configdisTask(void *argument)
{
  /* USER CODE BEGIN configdisTask */
  /* Infinite loop */
  while(1)
  {
    /*判断电平是否处于高电平，高电平处于连接状�?*/
    if( BLEWakeUp == 0 )  
    {
      if(connttimeout > 0) 
      {
        connttimeout=0;
      }
    }
    /*判断是否在显示屏�?*/
    else if(Displayflag == 1)
    {
      if((TABLEA_BUSY() != 0) && (TABLEB_BUSY() != 0))
      {
        Displayflag = 0;
        TABLEPOWOFF();
      }
      if(connttimeout > 0) 
      {
        connttimeout=0;
      }      
    }
    else
    {
      connttimeout++;
      user_main_info("BLE connect timeout %d",connttimeout);
      /*如果断开设备之后6秒，则进入显示设备消息显�?*/
      if(connttimeout > 4)
      {
        /*�?测设备是否有输入数据并进行显�?*/
        if(eDisplay_Data.DATAMODA)
        {
          TableSignSeting(TabFaceA,eDisplay_Data.PICADDRA,eDisplay_Data.DATAMODA);
          eDisplay_Data.DATAMODA = Picnone;
          eDisplay_Data.PICADDRA = 0;
        }
        /*�?测设备是否有输入数据并进行显�?*/
        if(eDisplay_Data.DATAMODB)
        {
          TableSignSeting(TabFaceB,eDisplay_Data.PICADDRB,eDisplay_Data.DATAMODB);
          eDisplay_Data.DATAMODB = Picnone;
          eDisplay_Data.PICADDRB = 0;
        }
        /*�?测设备是否有输入数据并进行显�?*/
        if(eDisplay_Data.DATAMODAB)
        {
          TableSignSeting(TabFaceAB,eDisplay_Data.PICADDRAB,eDisplay_Data.DATAMODAB);
          eDisplay_Data.DATAMODAB = Picnone;
          eDisplay_Data.PICADDRAB = 0;          
        }        
      }
      /*如果超时将设备进入休眠模�?*/
      if(connttimeout > 10)
      {
        connttimeout = 0;
        TABLEPOWOFF();
        user_main_info("BLE connect timeout entry");
        /*断开超时之后进入休眠*/
        Lpower_sleep_config();     
      }
    }
    osDelay(500);
   }
  /* USER CODE END configdisTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void BLE_handle_uartirq(char ch)
{
  osMessageQueuePut(QueueBLEusartHandle, &ch, NULL, 0);
}

/*低功耗休眠函数设�?*/
void Lpower_sleep_config(void)
{
  GPIO_InitTypeDef GPIO_InitStruct; 
  
  user_main_info("BLE no connent device will sleep");  
  SPI_DeInit();
  ADC_DeInit();
//  UART_Init();

  user_main_info("BLE entry sleep"); 
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Pin = GPIO_PIN_All;    
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct); 	
  HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);
  GPIO_InitStruct.Pin = GPIO_PIN_All & ~GPIO_PIN_3;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  GPIO_InitStruct.Pin = GPIO_PIN_All & ~GPIO_PIN_3;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  
  /* Disable GPIOs clock */
  __HAL_RCC_GPIOC_CLK_DISABLE();
  __HAL_RCC_GPIOB_CLK_DISABLE();
  // __HAL_RCC_GPIOA_CLK_DISABLE();  
  __HAL_RCC_GPIOH_CLK_DISABLE();
  
  sleep_BLE_Uartconfig();
      
  HAL_SuspendTick();    
  HAL_PWR_DeInit();
  HAL_PWR_DisablePVD();
//  
  HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON,PWR_STOPENTRY_WFI);
//  osDelay(5000);
//  
  HAL_PWR_EnablePVD();
  HAL_ResumeTick();    
//  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();  
  UART_Init();
  SPI_Init();
  ADC_Init();
//  GPIO_Init();
//  MX_USART2_UART_Init(); 
//  HAL_UART_Receive_IT(&_BLE_USART, &indata, 1);  
//  HAL_NVIC_DisableIRQ(EXTI3_IRQn);      
//  MX_RTC_Init();  
  user_main_info("wake up!!!"); 
  
}

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
