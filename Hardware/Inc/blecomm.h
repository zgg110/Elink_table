#ifndef __BLECOMM_H
#define __BLECOMM_H


#define	_BLE_USART huart2

typedef enum {
/*设备处于未连接状态*/
    BLENOCONNECT,
/*设备处于连接状态*/
    BLECONNECT,
/*设备处于连接唤醒状态*/
    BLEWKUPCONNECT
}BleWakeup_Flag;

void BLE_Send_Data(uint8_t *pData, uint16_t Size);
void BLE_CallBack(void);


#endif


























