#ifndef _MOTOR_H
#define _MOTOR_H
#include "stm32f10x.h" 
void Motor_Init(void);
void Motor_SetSpeedL(int16_t Speed); 
void Motor_SetSpeedR(int16_t Speed); 
#endif