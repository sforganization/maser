
#include "delay.h"
#include "sys.h"
#include "usart1.h"
#include "usart2.h"
#include "usart3.h"
#include "as608.h"

#include "timer.h"

#include "as608.h"
#include "pwm.h"


#define _MAININC_
#include "SysComment.h"
#undef _MAININC_
#define usart2_baund			57600//����2�����ʣ�����ָ��ģ�鲨���ʸ��ģ�ע�⣺ָ��ģ��Ĭ��57600��



void SysTickTask(void);


void ledinit(void) //DEBUG ��

{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB, GPIO_Pin_13);
}


int main(void)
{
	delay_init();									//��ʱ������ʼ��	  
	NVIC_Configuration();							//����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ� 	LED_Init();				 

	usart1_init(9600);
	usart3_init(9600);
	usart2_init(usart2_baund);						//��ʼ������2,������ָ��ģ��ͨѶ
	PS_StaGPIO_Init();								//��ʼ��FR��״̬����  ʶ��ָ�ư�ѹ����ߵ�ƽ����Ϊ���ù�ϵ��ʼ�������ŵ�KEY_GPIO_Init����
	GENERAL_TIMx_PWM_Init();
	SysInit();

	GENERAL_TIMx_Configuration();
	while (1)
	{
		MainTask();

		if (SysTask.nTick)
		{
			SysTask.nTick--;
			SysTickTask();

		}
	}
}


/*******************************************************************************
* ����: 
* ����: 
* �β�:		
* ����: ��
* ˵��: 
*******************************************************************************/
void SysTickTask(void)
{
	vu16 static 	u16SecTick = 0; 				//�����

	if (u16SecTick++ >= 1000) //������
	{
		u16SecTick			= 0;
	}

	if (SysTask.nWaitTime)
	{
		SysTask.nWaitTime--;
	}

	if (SysTask.nFingerSubWaitT)
		SysTask.nFingerSubWaitT--;
    
	if (SysTask.u16SubWaitTime)
		SysTask.u16SubWaitTime--;

    if(SysTask.u16BootTime)
        SysTask.u16BootTime--;
    
    if(SysTask.u16HtoSWaitTime)
        SysTask.u16HtoSWaitTime--;


    

	//			if (state)
	//			{
	//				state				= 0;
	//				GPIO_ResetBits(GPIOC, GPIO_Pin_13);
	//			}
	//			else 
	//			{
	//				state				= 1;
	//				GPIO_SetBits(GPIOC, GPIO_Pin_13);
	//			}
}


