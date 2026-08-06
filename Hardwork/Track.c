#include "stm32f10x.h"

void Track_Init(void)
{
	// 开启 GPIOA, GPIOB 和 AFIO
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
	
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;
	
	// 初始化 PA寻迹
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15 | GPIO_Pin_12 | GPIO_Pin_11 | GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	// 初始化 PB寻迹
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_4 | GPIO_Pin_3;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void Track_Value(uint8_t* R3, uint8_t* R2, uint8_t* R1, uint8_t* M, uint8_t* L3, uint8_t* L2, uint8_t* L1)
{

	*R3 = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_5);
	*R2 = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_4);
	*R1 = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_3); 
	*M  = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_15);
	*L3 = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_12);
	*L2 = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_11);
	*L1 = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_10);
}