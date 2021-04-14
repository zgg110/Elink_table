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
#include "w25q128.h"
#include "dataconfig.h"    
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

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
  .stack_size = 256 * 4
};
/* Definitions for QueueBLEusartHandle */
osMessageQueueId_t QueueBLEusartHandleHandle;
const osMessageQueueAttr_t QueueBLEusartHandle_attributes = {
  .name = "QueueBLEusartHandle"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void GetBLETask(void *argument);

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
  /* creation of QueueBLEusartHandle */
  QueueBLEusartHandleHandle = osMessageQueueNew (300, sizeof(uint8_t), &QueueBLEusartHandle_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of getBLETask */
  getBLETaskHandle = osThreadNew(GetBLETask, NULL, &getBLETask_attributes);

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
  uint8_t ebuff;
  uint32_t BLEUart2RxCnt=0;
  uint8_t  BLEUart2RxData[1124] = {0};  
  /* Infinite loop */
  while(1)
  {
    if(osMessageQueueGet(QueueBLEusartHandleHandle, &ebuff, NULL, portMAX_DELAY) == osOK)
    {
      BLEUart2RxData[BLEUart2RxCnt++] = ebuff;
      while(1)
      {
        /*将获取的数据放入接收字符串中*/        
        if(osMessageQueueGet(QueueBLEusartHandleHandle, &ebuff, NULL, 300) == osOK)
        {
          BLEUart2RxData[BLEUart2RxCnt++] = ebuff;  
        }  
        else
        {         
          /*接收数据完毕处理相应信息*/
          Rev_DataAnalye(BLEEVENT,BLEUart2RxData,BLEUart2RxCnt);
         
          break;
        }                
      }      
    }
  }
  /* USER CODE END GetBLETask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
