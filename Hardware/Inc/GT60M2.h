#ifndef __GT60M2_H
#define __GT60M2_H

//#include "Font.h"
#include "GT60L16M2K4.h"

#define        GT60M2_CS_HIGH()            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET)  
#define        GT60M2_CS_LOW()             HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET)



unsigned char r_dat(unsigned long int address);
unsigned char r_dat_bat(unsigned long int address,unsigned char byte_long,unsigned char *p_arr);

void GT60M2_Read_data(uint8_t *dat, uint32_t len);
void GT60M2_Reset_Cmd(void);

#endif




















