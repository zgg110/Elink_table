#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include "cmsis_os.h"
#include "gpio.h"
#include "usart.h"
#include "string.h"
#include "blecomm.h"
#include "tablesign.h"
//#include "dataconfig.h"




/*默认为唤醒状态*/
static BleWakeup_Flag eBleWakeupsta = BLECONNECT; 

extern UART_HandleTypeDef _BLE_USART;

extern osMessageQueueId_t QueueBLEusartHandleHandle;

void BLE_CallBack(void)
{
  uint8_t ch = 0;
  HAL_UART_Receive_IT(&_BLE_USART, &ch, 1);
  /*将接收到的数据塞进接收数据队列*/
  osMessageQueuePut(QueueBLEusartHandleHandle, &ch, NULL, 0);
}


/********************************************************************************************/


