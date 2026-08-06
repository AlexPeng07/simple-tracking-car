#include "stm32f10x.h"                  // Device header

void PWM_Init()
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;   
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9; 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    TIM_InternalClockConfig(TIM1);
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;    // 时钟分频
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; // 向上计数
    TIM_TimeBaseInitStructure.TIM_Period = 100 - 1;                // ARR自动重装值
    TIM_TimeBaseInitStructure.TIM_Prescaler = 36 - 1;              // PSC预分频器
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;           // TIM1特有重复计数器（无需求则设0）
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseInitStructure);

    TIM_OCInitTypeDef TIM_OC1InitStructure;
    TIM_OCStructInit(&TIM_OC1InitStructure);
    TIM_OC1InitStructure.TIM_OCMode = TIM_OCMode_PWM1;            // PWM1模式
    TIM_OC1InitStructure.TIM_OCPolarity = TIM_OCPolarity_High;     // 高电平有效
    TIM_OC1InitStructure.TIM_OutputState = TIM_OutputState_Enable;// 输出使能
    TIM_OC1InitStructure.TIM_Pulse = 0;                           // CCR捕获/compare值
    TIM_OC1Init(TIM1, &TIM_OC1InitStructure);
	
    TIM_OCInitTypeDef TIM_OC2InitStructure;
    TIM_OCStructInit(&TIM_OC2InitStructure);
    TIM_OC2InitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OC2InitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OC2InitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OC2InitStructure.TIM_Pulse = 0;
    TIM_OC2Init(TIM1, &TIM_OC2InitStructure);
	
	TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
    TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);
	
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQn;  // TIM1 更新中断
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; // 抢占优先级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;    // 子优先级
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
	
    TIM_CtrlPWMOutputs(TIM1, ENABLE);
    TIM_Cmd(TIM1, ENABLE);
}

void PWM_SetCompareL(uint16_t Compare)
{
    TIM_SetCompare1(TIM1, Compare);
}

void PWM_SetCompareR(uint16_t Compare)
{
    TIM_SetCompare2(TIM1, Compare);
}