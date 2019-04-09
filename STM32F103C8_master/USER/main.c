
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
#define usart2_baund			57600//串口2波特率，根据指纹模块波特率更改（注意：指纹模块默认57600）



void SysTickTask(void);


void ledinit(void) //DEBUG 用

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
	delay_init();									//延时函数初始化	  
	NVIC_Configuration();							//设置NVIC中断分组2:2位抢占优先级，2位响应优先级 	LED_Init();				 

	usart1_init(9600);
	usart3_init(9600);
	usart2_init(usart2_baund);						//初始化串口2,用于与指纹模块通讯
	PS_StaGPIO_Init();								//初始化FR读状态引脚  识别到指纹按压输出高电平，因为复用关系初始化动作放到KEY_GPIO_Init里面
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
* 名称: 
* 功能: 
* 形参:		
* 返回: 无
* 说明: 
*******************************************************************************/
void SysTickTask(void)
{
	vu16 static 	u16SecTick = 0; 				//秒计数

	if (u16SecTick++ >= 1000) //秒任务
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


