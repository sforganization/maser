
#include "timer.h"

#include "SysComment.h"

//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK Mini STM32������
//ͨ�ö�ʱ�� ��������			   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2014/3/07
//�汾��V1.2
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved	
//********************************************************************************	  
extern vu16 	USART1_RX_STA;
extern vu16 	USART2_RX_STA;
extern vu16 	USART3_RX_STA;
extern int		tick;

//��ʱ��4�жϷ������			
void TIM4_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET) //�Ǹ����ж�
	{
		USART1_RX_STA		|= 1 << 15; 			//��ǽ������
		USART2_RX_STA		|= 1 << 15; 			//��ǽ������
		USART3_RX_STA		|= 1 << 15; 			//��ǽ������
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update); //���TIM4�����жϱ�־	  
		TIM_Cmd(TIM4, DISABLE); 					//�ر�TIM4 
	}
}


//ͨ�ö�ʱ��4�жϳ�ʼ��
//����ʱ��ѡ��ΪAPB1��2������APB1Ϊ42M
//arr���Զ���װֵ��
//psc��ʱ��Ԥ��Ƶ��
//��ʱ�����ʱ����㷽��:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=��ʱ������Ƶ��,��λ:Mhz 
//ͨ�ö�ʱ���жϳ�ʼ��
//����ʼ��ѡ��ΪAPB1��2������APB1Ϊ36M
//arr���Զ���װֵ��
//psc��ʱ��Ԥ��Ƶ��		 
void TIM4_Int_Init(u16 arr, u16 psc)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE); //TIM4ʱ��ʹ��    

	//��ʱ��TIM4��ʼ��
	TIM_TimeBaseStructure.TIM_Period = arr; 		//��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	
	TIM_TimeBaseStructure.TIM_Prescaler = psc;		//����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure); //����ָ���Ĳ�����ʼ��TIMx��ʱ�������λ

	TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);		//ʹ��ָ����TIM4�ж�,��������ж�

	TIM_Cmd(TIM4, ENABLE);							//������ʱ��4

	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; //��ռ���ȼ�0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2; //�����ȼ�2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure); 				//����ָ���Ĳ�����ʼ��VIC�Ĵ���

}


/* �궨�� --------------------------------------------------------------------*/
/********************ͨ�ö�ʱ��TIM�������壬ֻ��TIM2 & TIM3 & TIM4 & TIM5************/
#define GENERAL_TIMx			TIM2
#define GENERAL_TIM_APBxClock_FUN RCC_APB1PeriphClockCmd
#define GENERAL_TIM_CLK 		RCC_APB1Periph_TIM2
#define GENERAL_TIM_IRQ 		TIM2_IRQn
#define GENERAL_TIM_INT_FUN 	TIM2_IRQHandler



//��ʱ��2�жϷ������			 ����tickʱ��
void TIM2_IRQHandler(void)
{

	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) //�Ǹ����ж�
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update); //���TIM4�����жϱ�־	 
		SysTask.nTick++;
	}
}


/**
  * ��������: ͨ�ö�ʱ�� TIMx,x[2,3,4,5]�ж����ȼ�����
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ������
  */
void GENERAL_TIMx_NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	/* �����ж���Ϊ0 */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);

	/* �����ж���Դ */
	NVIC_InitStructure.NVIC_IRQChannel = GENERAL_TIM_IRQ;

	/* ���������ȼ�Ϊ 0 */
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;

	/* ������ռ���ȼ�Ϊ3 */
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;

	/*��ʱ��ʹ�� */
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}


/*
 * TIM_Period / Auto Reload Register(ARR) = 1000   TIM_Prescaler--71 
 * �ж�����Ϊ = 1/(72MHZ /72) * 1000 = 1ms
 *
 * TIMxCLK/CK_PSC --> TIMxCNT --> TIM_Period(ARR) --> �ж� ��TIMxCNT����Ϊ0���¼��� 
 */
/*
 * ע�⣺TIM_TimeBaseInitTypeDef�ṹ��������5����Ա��TIM6��TIM7�ļĴ�������ֻ��
 * TIM_Prescaler��TIM_Period������ʹ��TIM6��TIM7��ʱ��ֻ���ʼ����������Ա���ɣ�
 * ����������Ա��ͨ�ö�ʱ���͸߼���ʱ������.
 *-----------------------------------------------------------------------------
 *typedef struct
 *{  TIM_Prescaler		   ����
 *	TIM_CounterMode			   TIMx,x[6,7]û�У���������
 *	TIM_Period			   ����
 *	TIM_ClockDivision	   TIMx,x[6,7]û�У���������
 *	TIM_RepetitionCounter  TIMx,x[1,8,15,16,17]����
 *}TIM_TimeBaseInitTypeDef; 
 *-----------------------------------------------------------------------------
 */
void GENERAL_TIMx_Configuration(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

	/* ����TIMx_CLK,x[2,3,4,5],���ڲ�ʱ��CK_INT=72M */
	GENERAL_TIM_APBxClock_FUN(GENERAL_TIM_CLK, ENABLE);

	/* ͨ�ö�ʱ�� TIMx,x[2,3,4,5]�ж����ȼ����� */
	GENERAL_TIMx_NVIC_Configuration();

	/* �Զ���װ�ؼĴ����ܵ�ֵ(����ֵ) */
	TIM_TimeBaseStructure.TIM_Period = 1000;

	/* �ۼ� TIM_Period��Ƶ�ʺ����һ�����»����ж�
	   ʱ��Ԥ��Ƶ��Ϊ71����������������ʱ��CK_CNT = CK_INT / (71+1)=1M */
	TIM_TimeBaseStructure.TIM_Prescaler = 71;

	/* ʱ�ӷ�Ƶ���� ��û���õ������ù� */
	//TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;

	/* ����������ģʽ��������ʱ��TIM6��TIM7ֻ�����ϼ�����û�м���ģʽ�����ã����ù� 
		*/
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

	/* �ظ���������ֵ��ͨ�ö�ʱ��û�У����ù� */
	//TIM_TimeBaseStructure.TIM_RepetitionCounter=0;

	/* ��ʼ����ʱ��TIMx, x[2,3,4,5] */
	TIM_TimeBaseInit(GENERAL_TIMx, &TIM_TimeBaseStructure);

	/* ����������жϱ�־λ */
	TIM_ClearITPendingBit(GENERAL_TIMx, TIM_IT_Update);

	/* �����������ж� */
	TIM_ITConfig(GENERAL_TIMx, TIM_IT_Update, ENABLE);

	/* ʹ�ܼ�����: */
	TIM_Cmd(GENERAL_TIMx, ENABLE);
}

















