#include "stm32f10x.h"                  // Device header
#include "Delay.h"

void MyI2C_W_SCL(uint8_t BitValue)
{
	GPIO_WriteBit(GPIOA,GPIO_Pin_2,(BitAction)BitValue);
	Delay_us(10);
}

void MyI2C_W_SDA(uint8_t BitValue)
{
	GPIO_WriteBit(GPIOA,GPIO_Pin_3,(BitAction)BitValue);
	Delay_us(10);
}

uint8_t MyI2C_R_SDA()
{
	uint8_t BitValue;
	BitValue=GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_3);
	Delay_us(10);
	return BitValue;
}

void MyI2C_Init()
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_OD;
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_2 | GPIO_Pin_3 ;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	GPIO_SetBits(GPIOA,GPIO_Pin_2 | GPIO_Pin_3);
}

void MyI2C_Start()
{
	MyI2C_W_SDA(1);
	MyI2C_W_SCL(1);
	MyI2C_W_SDA(0);
	MyI2C_W_SCL(0);
}

void MyI2C_Stop()
{
	MyI2C_W_SDA(0);
	MyI2C_W_SCL(1);
	MyI2C_W_SDA(1);
}

void MyI2C_SendByte(uint8_t Byte)
{
	uint8_t i;
	for(i=0;i<8;i++)
	{
		MyI2C_W_SDA(Byte & (0x80>>i));
		MyI2C_W_SCL(1);
		MyI2C_W_SCL(0);
	}
}

uint8_t MyI2C_ReceiveByte()
{
	uint8_t i,Byte=0x00;
	MyI2C_W_SDA(1);
	for(i=0;i<8;i++)
	{
	  MyI2C_W_SCL(1);
		if(MyI2C_R_SDA()==1){Byte|=(0x80>>i);}
		MyI2C_W_SCL(0);
	}
	return Byte;
}

void MyI2C_SendACK(uint8_t ACKBit)
{
		MyI2C_W_SDA(ACKBit);
		MyI2C_W_SCL(1);
		MyI2C_W_SCL(0);
}

uint8_t MyI2C_ReceiveACK()
{
	uint8_t ACKBit;
	MyI2C_W_SDA(1);
	MyI2C_W_SCL(1);
	ACKBit=MyI2C_R_SDA();
	MyI2C_W_SCL(0);
	return ACKBit;
}