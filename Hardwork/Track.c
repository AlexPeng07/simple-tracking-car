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

	/* read each port once so all 7 channels come from the same instant */
	uint16_t PortA = GPIO_ReadInputData(GPIOA);
	uint16_t PortB = GPIO_ReadInputData(GPIOB);

	*R3 = (PortB >> 5) & 0x0001;
	*R2 = (PortB >> 4) & 0x0001;
	*R1 = (PortB >> 3) & 0x0001;
	*M  = (PortA >> 15) & 0x0001;
	*L3 = (PortA >> 12) & 0x0001;
	*L2 = (PortA >> 11) & 0x0001;
	*L1 = (PortA >> 10) & 0x0001;
}