#ifndef __PWM_H__
#define __PWM_H__

/* ����ͷ�ļ� ----------------------------------------------------------------*/
#include <stm32f10x.h>

/* ���Ͷ��� ------------------------------------------------------------------*/
/* �궨�� --------------------------------------------------------------------*/
/********************ͨ�ö�ʱ��TIM�������壬ֻ��TIM2 & TIM3 & TIM4 & TIM5************/
#define GENERAL_TIMx                        TIM3
#define GENERAL_TIM_APBxClock_FUN           RCC_APB1PeriphClockCmd
#define GENERAL_TIM_CLK                     RCC_APB1Periph_TIM3
#define GENERAL_TIM_GPIO_APBxClock_FUN      RCC_APB2PeriphClockCmd
#define GENERAL_TIM_GPIO_CLK                (RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB )
#define GENERAL_TIM_CH1_PORT                GPIOA
#define GENERAL_TIM_CH1_PIN                 GPIO_Pin_6
#define GENERAL_TIM_CH2_PORT                GPIOA
#define GENERAL_TIM_CH2_PIN                 GPIO_Pin_7
#define GENERAL_TIM_CH3_PORT                GPIOB
#define GENERAL_TIM_CH3_PIN                 GPIO_Pin_0
#define GENERAL_TIM_CH4_PORT                GPIOB
#define GENERAL_TIM_CH4_PIN                 GPIO_Pin_1

/* ��չ���� ------------------------------------------------------------------*/
/* �������� ------------------------------------------------------------------*/
void GENERAL_TIMx_PWM_Init(void);

#endif	/* __PWM_H__ */
/******************* (C) COPYRIGHT 2015-2020 ӲʯǶ��ʽ�����Ŷ� *****END OF FILE****/
