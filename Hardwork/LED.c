#include "stm32f10x.h"   

void LED_Init()
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_13 | GPIO_Pin_14 ;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	
	GPIO_SetBits(GPIOB,GPIO_Pin_13 | GPIO_Pin_14);
}


void LED1_ON()
{
	GPIO_ResetBits(GPIOB,GPIO_Pin_13 );
}

void LED1_OFF()
{
	GPIO_SetBits(GPIOB,GPIO_Pin_13 );
}

void LED2_ON()
{
	GPIO_ResetBits(GPIOB,GPIO_Pin_14 );
}

void LED2_OFF()
{
	GPIO_SetBits(GPIOB,GPIO_Pin_14 );
}

