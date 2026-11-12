#include "stm32f10x.h"                  // Device header
#include "Motor.h"

/**
  * 函    数：电机初始化
  * 参    数：无
  * 返 回 值：无
  * 说    明：PB0 = TIM3_CH3 PWM，PB14/PB15 = 方向控制
  *            TB6612：PWMA -> PB0，AIN1 -> PB14，AIN2 -> PB15
  */
void Motor_Init(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    /* PB0 复用推挽，输出 TIM3_CH3 PWM */
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* PB14 / PB15 推挽输出，控制方向 */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* TIM3：72MHz / 72 = 1MHz，ARR = 100 -> 10kHz */
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_TimeBaseStructure.TIM_Period = 100 - 1;
    TIM_TimeBaseStructure.TIM_Prescaler = 72 - 1;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

    /* PWM1 模式，通道 3 */
    TIM_OCInitTypeDef TIM_OCInitStructure;
    TIM_OCStructInit(&TIM_OCInitStructure);
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 0;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OC3Init(TIM3, &TIM_OCInitStructure);

    TIM_Cmd(TIM3, ENABLE);

    Motor_SetSpeed(0);
}

/**
  * 函    数：设置电机速度
  * 参    数：Speed -100 ~ 100，正数正转，负数反转，0 停止
  * 返 回 值：无
  */
void Motor_SetSpeed(int16_t Speed)
{
    if (Speed > 100)
    {
        Speed = 100;
    }
    if (Speed < -100)
    {
        Speed = -100;
    }

    if (Speed > 0)
    {
        GPIO_SetBits(GPIOB, GPIO_Pin_14);
        GPIO_ResetBits(GPIOB, GPIO_Pin_15);
        TIM_SetCompare3(TIM3, (uint16_t)Speed);
    }
    else if (Speed < 0)
    {
        GPIO_ResetBits(GPIOB, GPIO_Pin_14);
        GPIO_SetBits(GPIOB, GPIO_Pin_15);
        TIM_SetCompare3(TIM3, (uint16_t)(-Speed));
    }
    else
    {
        GPIO_ResetBits(GPIOB, GPIO_Pin_14 | GPIO_Pin_15);
        TIM_SetCompare3(TIM3, 0);
    }
}
