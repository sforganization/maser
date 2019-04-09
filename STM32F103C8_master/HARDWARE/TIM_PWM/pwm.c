/**
  ******************************************************************************
  * �ļ�����: bsp_GeneralTIM.c 
  * ��    ��: ӲʯǶ��ʽ�����Ŷ�
  * ��    ��: V1.0
  * ��д����: 2015-10-04
  * ��    ��: ͨ�ö�ʱ��TIM3�����·PWM�ײ���������
  ******************************************************************************
  * ˵����
  * ����������Ӳʯstm32������YS-F1Proʹ�á�
  * 
  * �Ա���
  * ��̳��http://www.ing10bbs.com
  * ��Ȩ��ӲʯǶ��ʽ�����Ŷ����У��������á�
  ******************************************************************************
  */
/* ����ͷ�ļ� ----------------------------------------------------------------*/
#include "pwm.h" 

/* ˽�����Ͷ��� --------------------------------------------------------------*/
/* ˽�к궨�� ----------------------------------------------------------------*/
/* ˽�б��� ------------------------------------------------------------------*/
/* ��չ���� ------------------------------------------------------------------*/
/* ˽�к���ԭ�� --------------------------------------------------------------*/
/* ������ --------------------------------------------------------------------*/
/**
  * ��������: ����TIMx�������PWMʱ�õ���I/O
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ������
  */
static void GENERAL_TIMx_GPIO_Config(void) 
{
  GPIO_InitTypeDef GPIO_InitStructure;

	/* ʹ�ܶ�ʱ��ʼ�գ�����TIM3CLK Ϊ 72MHZ */
	GENERAL_TIM_APBxClock_FUN (GENERAL_TIM_CLK, ENABLE);

  /* ʹ�ܶ�ʱ��ͨ������GPIOʱ�� */
  GENERAL_TIM_GPIO_APBxClock_FUN(GENERAL_TIM_GPIO_CLK, ENABLE); 

  /* ���ö�ʱ��ͨ��1�������ģʽ�������������ģʽ */
  GPIO_InitStructure.GPIO_Pin =  GENERAL_TIM_CH1_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GENERAL_TIM_CH1_PORT, &GPIO_InitStructure);

  /* ���ö�ʱ��ͨ��2�������ģʽ */
  GPIO_InitStructure.GPIO_Pin =  GENERAL_TIM_CH2_PIN;
  GPIO_Init(GENERAL_TIM_CH2_PORT, &GPIO_InitStructure);

  /* ���ö�ʱ��ͨ��3�������ģʽ */
  GPIO_InitStructure.GPIO_Pin =  GENERAL_TIM_CH3_PIN;
  GPIO_Init(GENERAL_TIM_CH3_PORT, &GPIO_InitStructure);
	
  /* ���ö�ʱ��ͨ��4�������ģʽ */
  GPIO_InitStructure.GPIO_Pin =  GENERAL_TIM_CH4_PIN;
  GPIO_Init(GENERAL_TIM_CH4_PORT, &GPIO_InitStructure);
}

/**
  * ��������: ����TIM3�����PWM�źŵ�ģʽ�������ڡ����ԡ�ռ�ձ�
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ������
  */
/*
 * TIMxCLK/CK_PSC --> TIMxCNT --> TIMx_ARR --> TIMxCNT ���¼���
 *                    TIMx_CCR(��ƽ�����仯)
 * �ź�����=(TIMx_ARR +1 ) * ʱ������
 * ռ�ձ�=TIMx_CCR/(TIMx_ARR +1)
 */
static void GENERAL_TIMx_Configuration(void)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;

	/* PWM�źŵ�ƽ����ֵ */
 	uint16_t CCR1_Val = 500;        
 	uint16_t CCR2_Val = 375;
 	uint16_t CCR3_Val = 250;
 	uint16_t CCR4_Val = 125;

/* ----------------------------------------------------------------------- 
    GENERAL_TIMx Channel1 duty cycle = (GENERAL_TIMx_CCR1/ GENERAL_TIMx_ARR+1)* 100% = 50%
    GENERAL_TIMx Channel2 duty cycle = (GENERAL_TIMx_CCR2/ GENERAL_TIMx_ARR+1)* 100% = 37.5%
    GENERAL_TIMx Channel3 duty cycle = (GENERAL_TIMx_CCR3/ GENERAL_TIMx_ARR+1)* 100% = 25%
    GENERAL_TIMx Channel4 duty cycle = (GENERAL_TIMx_CCR4/ GENERAL_TIMx_ARR+1)* 100% = 12.5%
  ----------------------------------------------------------------------- */

  /* ��ʱ����������ʼ�� */		 
  /* ����ʱ����0������999����Ϊ1000�Σ�Ϊһ����ʱ���� */
  TIM_TimeBaseStructure.TIM_Period = 999; 
  /* ����Ԥ��Ƶ����Ԥ��Ƶ����Ϊ72MHz,�������Ƶ�ʣ�72MHz/(999+1) */
  /* ͬһ����ʱ���Ĳ�ͬͨ��PWM��Ƶ��ֻ������ͬ�ģ���ͬͨ����������Ϊ��ͬ��ռ�ձ� */
  TIM_TimeBaseStructure.TIM_Prescaler = 0;
  /* ����ʱ�ӷ�Ƶϵ��������Ƶ(�����ò���) */
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1 ;
  /* ���ϼ���ģʽ */
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(GENERAL_TIMx, &TIM_TimeBaseStructure);

  /* ��ʱ�����ͨ��1ģʽ���� */
  /* ģʽ���ã�PWMģʽ1 */
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  /* ���״̬���ã�ʹ����� */
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;	
  /* ��������ֵ�������������������ֵʱ����ƽ�������� */
  TIM_OCInitStructure.TIM_Pulse = CCR1_Val;
  /* ����ʱ������ֵС��CCR1_ValʱΪ�ߵ�ƽ */
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
  /* ��ʼ����ʱ��ͨ��1���PWM */
  TIM_OC1Init(GENERAL_TIMx, &TIM_OCInitStructure);
  /* ��ʱ���Ƚ����ͨ��1Ԥװ�����ã�ʹ��Ԥװ�� */
  TIM_OC1PreloadConfig(GENERAL_TIMx, TIM_OCPreload_Enable);
  
  /* ��ʱ�����ͨ��2ģʽ���� */
  /* ����ͨ��2�ĵ�ƽ����ֵ���������һ��ռ�ձȵ�PWM */
  TIM_OCInitStructure.TIM_Pulse = CCR2_Val;
  /* ��ʼ����ʱ��ͨ��2���PWM */
  TIM_OC2Init(GENERAL_TIMx, &TIM_OCInitStructure);
  /* ��ʱ���Ƚ����ͨ��2Ԥװ�����ã�ʹ��Ԥװ�� */
  TIM_OC2PreloadConfig(GENERAL_TIMx, TIM_OCPreload_Enable);

  /* ��ʱ�����ͨ��3ģʽ���� */
  /* ����ͨ��3�ĵ�ƽ����ֵ���������һ��ռ�ձȵ�PWM */
  TIM_OCInitStructure.TIM_Pulse = CCR3_Val;
  /* ��ʼ����ʱ��ͨ��3���PWM */
  TIM_OC3Init(GENERAL_TIMx, &TIM_OCInitStructure);
  /* ��ʱ���Ƚ����ͨ��3Ԥװ�����ã�ʹ��Ԥװ�� */
  TIM_OC3PreloadConfig(GENERAL_TIMx, TIM_OCPreload_Enable);

  /* ��ʱ�����ͨ��4ģʽ���� */
  /* ����ͨ��4�ĵ�ƽ����ֵ���������һ��ռ�ձȵ�PWM */
  TIM_OCInitStructure.TIM_Pulse = CCR4_Val;
  /* ��ʼ����ʱ��ͨ��4���PWM */
  TIM_OC4Init(GENERAL_TIMx, &TIM_OCInitStructure);
  /* ��ʱ���Ƚ����ͨ��4Ԥװ�����ã�ʹ��Ԥװ�� */
  TIM_OC4PreloadConfig(GENERAL_TIMx, TIM_OCPreload_Enable);
  
  /* ʹ�ܶ�ʱ�����ؼĴ���ARR */
  TIM_ARRPreloadConfig(GENERAL_TIMx, ENABLE);

  /* ʹ�ܶ�ʱ�� */
  TIM_Cmd(GENERAL_TIMx, ENABLE);
}

/**
  * ��������: TIMx ���PWM�źų�ʼ��
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ����ֻҪ�����������TIMx���ĸ�ͨ���ͻ���PWM�ź����
  */
void GENERAL_TIMx_PWM_Init(void)
{
	GENERAL_TIMx_GPIO_Config();
	GENERAL_TIMx_Configuration();	
}
/******************* (C) COPYRIGHT 2015-2020 ӲʯǶ��ʽ�����Ŷ� *****END OF FILE****/
