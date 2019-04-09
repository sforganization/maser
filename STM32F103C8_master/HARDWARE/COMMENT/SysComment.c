
#include "usart3.h"
#include "usart1.h" 	 
#include "string.h"	 

#include "SysComment.h"
#include "stm32f10x_it.h"
#include "stm32f10x_iwdg.h"
#include "san_flash.h"

#include "as608.h"
#include "bmp.h"



#define     SLAVE_WAIT_TIME         3000    //3s 从机应答时间


#define     SLAVE_SEND_ALL        0X10   //BIT[7]:发送全部      BIT[6~0]:代表从机个数
#define     GLASS_SEND_ALL        0XFF  //发送给该从机个所有的手表


//内部变量
static vu16 	mDelay;
u8    g_SlaveSendAddr = 0;   //要发送的从机地址
u8    g_GlassSendAddr = 0;   //要发送的手表地址

//内部函数
void SysTickConfig(void);


/*******************************************************************************
* 名称: SysTick_Handler
* 功能: 系统时钟节拍1MS
* 形参:		
* 返回: 无
* 说明: 
*******************************************************************************/
void SysTick_Handler(void)
{
	static u16		Tick_1S = 0;

	mSysTick++;
	mSysSec++;
	mTimeRFRX++;

	if (mDelay)
		mDelay--;

	if (++Tick_1S >= 1000)
	{
		Tick_1S 			= 0;

		if (mSysIWDGDog)
		{
			IWDG_ReloadCounter();					/*喂STM32内置硬件狗*/

			if ((++mSysSoftDog) > 5) /*软狗system  DOG 2S over*/
			{
				mSysSoftDog 		= 0;
				NVIC_SystemReset();
			}
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
void DelayUs(uint16_t nCount)
{
	u32 			del = nCount * 5;

	//48M 0.32uS
	//24M 0.68uS
	//16M 1.02us
	while (del--)
		;
}


/*******************************************************************************
* 名称: 
* 功能: 
* 形参:		
* 返回: 无
* 说明: 
*******************************************************************************/
void DelayMs(uint16_t nCount)
{
	unsigned int	ti;

	for (; nCount > 0; nCount--)
	{
		for (ti = 0; ti < 4260; ti++)
			; //16M/980-24M/1420 -48M/2840
	}
}


/*******************************************************************************
* 名称: Strcpy()
* 功能: 
* 形参:		
* 返回: 无
* 说明: 
******************************************************************************/
void Strcpy(u8 * str1, u8 * str2, u8 len)
{
	for (; len > 0; len--)
	{
		*str1++ 			= *str2++;
	}
}


/*******************************************************************************
* 名称: Strcmp()
* 功能: 
* 形参:		
* 返回: 无
* 说明: 
******************************************************************************/
bool Strcmp(u8 * str1, u8 * str2, u8 len)
{
	for (; len > 0; len--)
	{
		if (*str1++ != *str2++)
			return FALSE;
	}

	return TRUE;
}


/*******************************************************************************
* 名称: Sys_DelayMS()
* 功能: 系统延迟函数
* 形参:		
* 返回: 无
* 说明: 
*******************************************************************************/
void Sys_DelayMS(uint16_t nms)
{
	mDelay				= nms + 1;

	while (mDelay != 0x0)
		;
}


/*******************************************************************************
* 名称: Sys_LayerInit
* 功能: 系统初始化
* 形参:		
* 返回: 无
* 说明: 
*******************************************************************************/
void Sys_LayerInit(void)
{
	SysTickConfig();

	mSysSec 			= 0;
	mSysTick			= 0;
	SysTask.mUpdate 	= TRUE;
}


/*******************************************************************************
* 名称: 
* 功能: 
* 形参:		
* 返回: 无
* 说明: 
*******************************************************************************/
void Sys_IWDGConfig(u16 time)
{
	/* 写入0x5555,用于允许狗狗寄存器写入功能 */
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

	/* 狗狗时钟分频,40K/64=0.625K()*/
	IWDG_SetPrescaler(IWDG_Prescaler_64);

	/* 喂狗时间 TIME*1.6MS .注意不能大于0xfff*/
	IWDG_SetReload(time);

	/* 喂狗*/
	IWDG_ReloadCounter();

	/* 使能狗狗*/
	IWDG_Enable();
}


/*******************************************************************************
* 名称: Sys_IWDGReloadCounter
* 功能: 
* 形参:		
* 返回: 无
* 说明: 
*******************************************************************************/
void Sys_IWDGReloadCounter(void)
{
	mSysSoftDog 		= 0;						//喂软狗
}


/*******************************************************************************
* 名称: 
* 功能: 
* 形参:		
* 返回: 无
* 说明: 
*******************************************************************************/
void SysTickConfig(void)
{
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);

	/* Setup SysTick Timer for 1ms interrupts  */
	if (SysTick_Config(SystemCoreClock / 1000))
	{
		/* Capture error */
		while (1)
			;
	}

	/* Configure the SysTick handler priority */
	NVIC_SetPriority(SysTick_IRQn, 0x0);

#if (								SYSINFOR_PRINTF == 1)
	printf("SysTickConfig:Tick=%d/Second\r\n", 1000);
#endif
}




/*******************************************************************************
* 名称: 
* 功能: 
* 形参:		
* 返回: 无
* 说明: 
*******************************************************************************/
void RemoteAddUser(void)
{
	/*
		u8 key;
		u8 ensure;

		//	u8 u8Result 		= 0;
		u16 u8Shownum;
		u16 u16FingerCnt	= 0;						//指纹模版个数
		static u16 u16FingerID = 0; 					//指纹模版ID
		static u8 u8Position = 0;						//显示位置
		static u8 u8InputCnt = 0;						//输入个数

		switch (SysTask.RemoteSub)
		{
			case INIT: // *
				if (SysTask.nWaitTime == 0)
				{
					PS_ValidTempleteNum(&u16FingerCnt); //读库指纹个数

					if (u16FingerCnt >= FINGER_MAX_CNT)
					{
						SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
						SysTask.RemoteSub	= WAIT;
						SysTask.nWaitTime	= 2000; 	//延时一定时间退出
						OLED_Clear();
						OLED_ShowString(0, 3, "Fingerprint template full！", 12);
					}
					else 
					{
						SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
						SysTask.RemoteSub	= GETKEY;
						u16FingerID 		= 0;
						u8Position			= 0;
						u8InputCnt			= 0;
						OLED_Clear();
						OLED_ShowString(0, 0, "Input ID to store", 12);
						OLED_ShowString(40, 3, "(0 ~ 9)", 12);
					}
				}

				break;

			case ENTER: //选择
				if ((PS_Sta) && (SysTask.nWaitTime == 0)) //如果有指纹按下
				{

					SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
					ensure				= PS_GetImage();

					if (ensure == 0x00)
					{
						ensure				= PS_GenChar(CharBuffer1); //生成特征

						if (ensure == 0x00)
						{
							OLED_Clear();
							OLED_ShowString(0, 3, "Please press  agin", 12);
							SysTask.nWaitTime	= 2000; //延时一定时间再去采集
							SysTask.RemoteSub	= AGAIN; //跳到第二步						
						}
					}
				}

				break;

			case AGAIN:
				if ((PS_Sta) && (SysTask.nWaitTime == 0)) //如果有指纹按下
				{
					SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
					ensure				= PS_GetImage();

					if (ensure == 0x00)
					{
						ensure				= PS_GenChar(CharBuffer2); //生成特征

						if (ensure == 0x00)
						{
							ensure				= PS_Match(); //对比两次指纹

							if (ensure == 0x00) //成功
							{

								ensure				= PS_RegModel(); //生成指纹模板

								if (ensure == 0x00)
								{

									ensure				= PS_StoreChar(CharBuffer2, u16FingerID); //储存模板

									if (ensure == 0x00)
									{
										OLED_Clear();
										SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
										SysTask.RemoteSub	= WAIT;
										SysTask.nWaitTime	= 3000; //延时一定时间退出
										OLED_ShowString(0, 3, "Add User Success", 16);
									}
									else 
									{ //失败
										OLED_Clear();
										OLED_ShowString(0, 3, "Store Failed", 16);
									}
								}
								else 
								{ //失败
									OLED_Clear();
									OLED_ShowString(0, 3, "Generate Template  Failed", 12);
								}
							}
							else 
							{ //失败
								OLED_Clear();
								OLED_ShowString(0, 3, "Match  Failed", 12);
							}
						}
					}
				}

				break;

			case GETKEY: // *
				if (Remote_Rdy)
				{
					SysTask.nShowTime	= REMOTE_SHOW_TIME; //刷新显示时间

					key 				= Remote_Process();

					if (key != REMOTE_KEY_ERR)
					{
						if (key == REMOTE_KEY_OK) //确定键
						{
							if (u8InputCnt != 0)
							{
								SysTask.RemoteSub	= ENTER;

								SysTask.nWaitTime	= 2000;
								OLED_Clear();
								OLED_ShowString(0, 3, "Please press finger", 12);
							}
							else 
							{
								SysTask.RemoteSub	= INIT;
								SysTask.nWaitTime	= 2000;
								OLED_Clear();
								OLED_ShowString(0, 0, "NOT input ID", 12);
							}
						}
						else if (key == REMOTE_KEY_RETURN) //返回键
						{
							SysTask.RemoteState = MANAGE_FINGER;
							SysTask.RemoteSub	= INIT;
						}
						else if ((key == REMOTE_KEY_0) || (key == REMOTE_KEY_1) || (key == REMOTE_KEY_2) ||
							 (key == REMOTE_KEY_3) || (key == REMOTE_KEY_4) || (key == REMOTE_KEY_5) ||
							 (key == REMOTE_KEY_6) || (key == REMOTE_KEY_7) || (key == REMOTE_KEY_8) ||
							 (key == REMOTE_KEY_9))
						{
							u8Shownum			= RemoteKey2Val(key);
							u16FingerID 		= u16FingerID * 10 + u8Shownum;
							u8Position			= 32 + u8InputCnt * 16;

							OLED_ShowChar(u8Position, 6, '0' + u8Shownum, 16);

							if ((u16FingerID >= FINGER_MAX_CNT) || (u8InputCnt++ >= 3))
							{
								SysTask.RemoteSub	= INIT;
								SysTask.nWaitTime	= 2500;

								OLED_Clear();
								OLED_ShowString(0, 0, "number More than MAX", 12);
								OLED_ShowString(0, 6, "Try agin", 16);
							}

						}
					}
				}

				break;

			case WAIT: // *
				if ((SysTask.nWaitTime == 0) || (Remote_Rdy))
				{
					SysTask.RemoteState = MANAGE_FINGER;
					SysTask.RemoteSub	= INIT;
				}

				break;

			default:
				break;
		}
		*/
}


/*******************************************************************************
* 名称: 
* 功能: 
* 形参:		
* 返回: 无
* 说明: 
*******************************************************************************/
void RemoteDelUser(void)
{
	/*
		u8 key;
		u8 ensure;

		//	u8 u8Result 		= 0;
		u16 u8Shownum;
		u16 u16FingerCnt	= 0;						//指纹模版个数
		static u16 u16FingerID = 0; 					//指纹模版ID
		static u8 u8Position = 0;						//显示位置
		static u8 u8InputCnt = 0;						//输入个数

		switch (SysTask.RemoteSub)
		{
			case INIT: // *
				if (SysTask.nWaitTime == 0)
				{
					PS_ValidTempleteNum(&u16FingerCnt); //读库指纹个数

					if (u16FingerCnt == 0)
					{
						SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
						SysTask.RemoteSub	= WAIT;
						SysTask.nWaitTime	= 2000; 	//延时一定时间退出
						OLED_Clear();
						OLED_ShowString(0, 3, "Fingerprint template empty！", 12);
					}
					else 
					{
						SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
						SysTask.RemoteSub	= GETKEY;
						u16FingerID 		= 0;
						u8Position			= 0;
						u8InputCnt			= 0;
						OLED_Clear();
						OLED_ShowString(0, 0, "Input ID to del", 12);
						OLED_ShowString(30, 3, "(0 ~ 9)", 12);
					}
				}

				break;

			case ENTER: //选择
				if (u16FingerID == 300) //全部删除
					ensure = PS_Empty();
				else 
					ensure = PS_DeletChar(u16FingerID, 1);

				if (ensure == 0)
				{

					OLED_Clear();
					SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
					SysTask.RemoteSub	= WAIT;
					SysTask.nWaitTime	= 3000; 		//延时一定时间退出
					OLED_ShowString(0, 3, "Del User Success", 16);

				}
				else 
				{
					OLED_Clear();
					SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
					SysTask.RemoteSub	= INIT;
					SysTask.nWaitTime	= 3000; 		//延时一定时间退出
					OLED_ShowString(0, 3, "Del User Failed", 16);

				}

				//PS_ValidTempleteNum(&u16FingerCnt); //读库指纹个数
				break;

			case GETKEY: // *
				if (Remote_Rdy)
				{
					SysTask.nShowTime	= REMOTE_SHOW_TIME; //刷新显示时间

					key 				= Remote_Process();

					if (key != REMOTE_KEY_ERR)
					{
						if (key == REMOTE_KEY_OK) //确定键
						{
							if (u8InputCnt != 0)
							{
								SysTask.RemoteSub	= ENTER;

								SysTask.nWaitTime	= 2000;
								OLED_Clear();
							}
							else 
							{
								SysTask.RemoteSub	= INIT;
								SysTask.nWaitTime	= 2000;
								OLED_Clear();
								OLED_ShowString(0, 0, "NOT input ID", 12);
							}
						}
						else if (key == REMOTE_KEY_RETURN) //返回键
						{
							SysTask.RemoteState = MANAGE_FINGER;
							SysTask.RemoteSub	= INIT;
						}
						else if ((key == REMOTE_KEY_0) || (key == REMOTE_KEY_1) || (key == REMOTE_KEY_2) ||
							 (key == REMOTE_KEY_3) || (key == REMOTE_KEY_4) || (key == REMOTE_KEY_5) ||
							 (key == REMOTE_KEY_6) || (key == REMOTE_KEY_7) || (key == REMOTE_KEY_8) ||
							 (key == REMOTE_KEY_9))
						{
							u8Shownum			= RemoteKey2Val(key);
							u16FingerID 		= u16FingerID * 10 + u8Shownum;
							u8Position			= 32 + u8InputCnt * 16;

							OLED_ShowChar(u8Position, 6, '0' + u8Shownum, 16);


							if ((u16FingerID >= FINGER_MAX_CNT) || (u8InputCnt++ >= 3))
							{
								SysTask.RemoteSub	= INIT;
								SysTask.nWaitTime	= 2500;

								OLED_Clear();
								OLED_ShowString(0, 0, "number More than MAX", 12);
								OLED_ShowString(0, 6, "Try agin", 16);
							}

						}
					}
				}

				break;

			case WAIT: // *
				if ((SysTask.nWaitTime == 0) || (Remote_Rdy))
				{
					SysTask.RemoteState = MANAGE_FINGER;
					SysTask.RemoteSub	= INIT;
				}

				break;

			default:
				break;
		}
		*/
}


#define DEBOUNCE_TIME			40 //40ms 防抖
void FingerTouchTask(void)
{
	/*SearchResult seach;
	u8 ensure;
	static u8 u8MatchCnt = 0;						//匹配失败次数，默认匹配MATCH_FINGER_CNT次 
	static u8 u8MotoSelect = 0;
	static u8 u8SelectPosi = 0;
	static u16 u16FingerID = 0; 					//指纹模版ID
	u16 u16FingerCnt	= 0;						//指纹模版个数


	if ((SysTask.RemoteState == SAN_DEF) || (SysTask.RemoteState == WRITE_PASSWD))
	{
		switch (SysTask.TouchState)
		{
			case TOUCH_INIT:
				if (PS_Sta) //有指纹按下
				{
					SysTask.nWaitTime	= 100;		//一段时间再检测
					SysTask.TouchState	= TOUCH_CHECK;
					u8MatchCnt			= 0;
				}

				break;

			case TOUCH_CHECK:
				if (SysTask.nWaitTime == 0) //有指纹按下
				{
					ensure				= PS_GetImage();

					if (ensure == 0x00) //生成特征成功
					{
						//PS_ValidTempleteNum(&u16FingerID); //读库指纹个数
						ensure				= PS_GenChar(CharBuffer1);

						if (ensure == 0x00) //生成特征成功
						{
							ensure				= PS_Search(CharBuffer1, 0, FINGER_MAX_CNT, &seach);

							if (ensure == 0) //匹配成功
							{
								u16FingerID 		= seach.pageID;
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
								SysTask.nWaitTime	= SysTask.nTick + 1000; //延时一定时间 实测此时只会延?
		?300ms
								SysTask.TouchSub	= TOUCH_SUB_INIT;

								if (u16FingerID == 0) //超级用户管理
								{
									SysTask.TouchState	= TOUCH_MANAGE_DISPLAY;
								}
								else 
								{
									SysTask.TouchState	= TOUCH_DISPLAY_ID;
								}

								OLED_Clear();
								OLED_ShowString(32, 0, "User ID ", 16);

								OLED_ShowChar(40, 3, '0' + u16FingerID / 100, 16);
								OLED_ShowChar(56, 3, '0' + (u16FingerID / 10) % 10, 16);
								OLED_ShowChar(72, 3, '0' + (u16FingerID) % 10, 16);


								//								OLED_DrawBMP(0, 4, 48, 8, BMP_clock);
								//								OLED_DrawBMP(80, 4, 128, 8, BMP_FwdRev);
							}
							else if (u8MatchCnt >= MATCH_FINGER_CNT)
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
								SysTask.nWaitTime	= SysTask.nTick + 2000; //延时一定时间退出
								SysTask.TouchState	= TOUCH_WAIT;


								OLED_Clear();
								OLED_ShowString(0, 3, "Can't fined fingerprin", 16);
							}
						}
					}

					if (ensure) //匹配失败
					{
						u8MatchCnt++;
					}
				}

				break;

			case TOUCH_MANAGE_DISPLAY:
				if ((SysTask.nWaitTime == 0) && (GPIO_ReadInputDataBit(KEY5_GPIO, KEY5_GPIO_PIN) != 0) //按时间选?
		??键 松手检测，不然一直闪
				&& (GPIO_ReadInputDataBit(KEY6_GPIO, KEY6_GPIO_PIN) != 0) //按HOME键
				&& (GPIO_ReadInputDataBit(KEY7_GPIO, KEY7_GPIO_PIN) != 0)) //确定键
				{
					SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
					SysTask.nWaitTime	= 20;		//2s显示ID时间
					SysTask.TouchState	= TOUCH_MANAGE;
					SysTask.TouchSub	= TOUCH_SUB_INIT;
					u8SelectPosi		= 0;

					OLED_Clear();
					OLED_ShowString(48, 0, "Manage", 12);
					OLED_ShowString(32, 3, "-> Add User", 12);
					OLED_ShowString(32, 5, "Del User", 12);
				}

				break;

			case TOUCH_MANAGE:
				if (GPIO_ReadInputDataBit(KEY5_GPIO, KEY5_GPIO_PIN) == 0) //按时间选择键
				{
					SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
					SysTask.TouchState	= TOUCH_MANAGE_CHOOSE; //防抖,松手检测
				}

				if (GPIO_ReadInputDataBit(KEY6_GPIO, KEY6_GPIO_PIN) == 0) //按HOME键
				{
						   SysTask.nShowTime = 0;
				
							   OledInitTask();
				}

				if (GPIO_ReadInputDataBit(KEY7_GPIO, KEY7_GPIO_PIN) == 0) //确定键
				{
					SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
					SysTask.TouchSub	= TOUCH_SUB_INIT;

					if (u8SelectPosi == 0)
					{
						SysTask.TouchState	= TOUCH_ADD_USER;
					}
					else 
					{
						SysTask.TouchState	= TOUCH_DEL_USER;
					}

				}

				break;

			case TOUCH_MANAGE_CHOOSE: //防抖,松手检测
				if (SysTask.nFingerSubWaitT != 0)
				{
					break;
				}

				if ((GPIO_ReadInputDataBit(KEY5_GPIO, KEY5_GPIO_PIN) == 0) //按时间选择键
				|| (GPIO_ReadInputDataBit(KEY6_GPIO, KEY6_GPIO_PIN) == 0) //按HOME键
				|| (GPIO_ReadInputDataBit(KEY7_GPIO, KEY7_GPIO_PIN) == 0)) //确定键
				{
					SysTask.nFingerSubWaitT = 20;	//20ms检测一次
				}
				else 
				{
					SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示

					if (++u8SelectPosi >= 2)
						u8SelectPosi = 0;

					switch (u8SelectPosi)
					{
						case 0:
							OLED_Clear();
							OLED_ShowString(48, 0, "Manage", 12);
							OLED_ShowString(32, 3, "-> Add User", 12);
							OLED_ShowString(32, 5, "Del User", 12);
							break;

						case 1:
							OLED_Clear();
							OLED_ShowString(48, 0, "Manage", 12);
							OLED_ShowString(32, 3, "Add User", 12);
							OLED_ShowString(32, 5, "-> Del User", 12);
							break;

						default:
							u8SelectPosi = 0;
							break;
					}

					SysTask.TouchState	= TOUCH_MANAGE;
				}

				break;

			case TOUCH_ADD_USER:
				if (GPIO_ReadInputDataBit(KEY7_GPIO, KEY7_GPIO_PIN) == 0) //确定键松手检测
				{
					break;
				}

				{
					switch (SysTask.TouchSub)
					{
						case TOUCH_SUB_INIT: // *
							if (SysTask.nWaitTime == 0)
							{
								PS_ValidTempleteNum(&u16FingerCnt); //读库指纹个数

								if (u16FingerCnt >= FINGER_MAX_CNT)
								{
									SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
									SysTask.nWaitTime	= 2000; //延时一定时间退出
									SysTask.TouchState	= TOUCH_MANAGE_DISPLAY;
									OLED_Clear();
									OLED_ShowString(0, 3, "Fingerprint template full！", 12);
								}
								else 
								{
									SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
									SysTask.TouchSub	= TOUCH_GETKEY;
									u16FingerID 		= 0;
									u8SelectPosi		= 0;
									OLED_Clear();
									OLED_ShowString(0, 0, "Choose ID (1~9)", 12);
									OLED_ShowString(32, 2, "-> 1", 12);
									OLED_ShowString(32, 4, "2", 12);
									OLED_ShowString(32, 6, "3", 12);
								}
							}

							break;

						case TOUCH_GETKEY: // *
							if (GPIO_ReadInputDataBit(KEY5_GPIO, KEY5_GPIO_PIN) == 0) //按时间选择键
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示

								SysTask.TouchSub	= TOUCH_GETKEY_DEBOUNCE; //防抖,松手检测
							}

							if (GPIO_ReadInputDataBit(KEY6_GPIO, KEY6_GPIO_PIN) == 0) //按HOME键
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
								SysTask.TouchState	= TOUCH_MANAGE_DISPLAY;
							}

							if (GPIO_ReadInputDataBit(KEY7_GPIO, KEY7_GPIO_PIN) == 0) //确定键
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
								SysTask.TouchSub	= TOUCH_SUB_ENTER;
								u16FingerID 		= u8SelectPosi;
								SysTask.nWaitTime	= 1000;
								OLED_Clear();
								OLED_ShowString(0, 3, "Please press", 12);
							}

							break;

						case TOUCH_GETKEY_DEBOUNCE: //防抖,松手检测
							if (SysTask.nFingerSubWaitT != 0)
							{
								break;
							}

							if ((GPIO_ReadInputDataBit(KEY5_GPIO, KEY5_GPIO_PIN) == 0)) //按时间选择键
							{
								SysTask.nFingerSubWaitT = 20; //20ms检测一次
							}
							else 
							{
								if (++u8SelectPosi >= 10)
									u8SelectPosi = 1;

								if (u8SelectPosi == 8)
								{
									OLED_ShowString(32, 2, "-> 8", 12);
									OLED_ShowString(32, 4, "9", 12);
									OLED_ShowString(32, 6, "1", 12);

								}
								else if (u8SelectPosi == 9)
								{
									OLED_ShowString(32, 2, "-> 9", 12);
									OLED_ShowString(32, 4, "1", 12);
									OLED_ShowString(32, 6, "2", 12);
								}
								else 
								{
									OLED_ShowString(32, 2, "-> ", 12);
									OLED_ShowChar(56, 2, '0' + u8SelectPosi, 12);
									OLED_ShowChar(32, 4, '1' + u8SelectPosi, 12);
									OLED_ShowChar(32, 6, '2' + u8SelectPosi, 12);
								}

								SysTask.TouchSub	= TOUCH_GETKEY;

							}

							break;

						case TOUCH_SUB_ENTER: //选择
							if ((PS_Sta) && (SysTask.nWaitTime == 0)) //如果有指纹按下
							{

								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
								ensure				= PS_GetImage();

								if (ensure == 0x00)
								{
									ensure				= PS_GenChar(CharBuffer1); //生成特征

									if (ensure == 0x00)
									{
										OLED_Clear();
										OLED_ShowString(0, 3, "Please press  agin", 12);
										SysTask.nWaitTime	= 1000; //延时一定时间再去采集
										SysTask.TouchSub	= TOUCH_SUB_AGAIN; //跳到第二步						
									}
								}
							}

							if (GPIO_ReadInputDataBit(KEY6_GPIO, KEY6_GPIO_PIN) == 0) //按HOME键
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
								SysTask.TouchState	= TOUCH_MANAGE_DISPLAY;
							}

							break;

						case TOUCH_SUB_AGAIN:
							if ((PS_Sta) && (SysTask.nWaitTime == 0)) //如果有指纹按下
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
								ensure				= PS_GetImage();

								if (ensure == 0x00)
								{
									ensure				= PS_GenChar(CharBuffer2); //生成特征

									if (ensure == 0x00)
									{
										ensure				= PS_Match(); //对比两次指纹

										if (ensure == 0x00) //成功
										{

											ensure				= PS_RegModel(); //生成指纹模板

											if (ensure == 0x00)
											{

												ensure				= PS_StoreChar(CharBuffer2, u16FingerID); //储存模板

												if (ensure == 0x00)
												{
													OLED_Clear();
													SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
													SysTask.TouchSub	= TOUCH_SUB_WAIT;
													SysTask.nWaitTime	= 3000; //延时一定时间退出
													OLED_ShowString(0, 3, "Add User Success", 16);
												}
												else 
												{ //失败
													OLED_Clear();
													OLED_ShowString(0, 3, "Store Failed", 16);
													SysTask.TouchSub	= TOUCH_SUB_ENTER;
													SysTask.nWaitTime	= 3000; //延时一定时间退出
												}
											}
											else 
											{ //失败
												OLED_Clear();
												OLED_ShowString(0, 3, "Generate Template  Failed", 12);
												SysTask.TouchSub	= TOUCH_SUB_ENTER;
												SysTask.nWaitTime	= 3000; //延时一定时间退出
											}
										}
										else 
										{ //失败
											OLED_Clear();
											OLED_ShowString(0, 3, "Match  Failed", 12);
											SysTask.TouchSub	= TOUCH_SUB_ENTER;
											SysTask.nWaitTime	= 3000; //延时一定时间退出
										}
									}
								}
							}

							if (GPIO_ReadInputDataBit(KEY6_GPIO, KEY6_GPIO_PIN) == 0) //按HOME键
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
								SysTask.TouchState	= TOUCH_MANAGE_DISPLAY;
							}

							break;

						case TOUCH_SUB_WAIT: // *
							if ((SysTask.nWaitTime == 0))
							{
								SysTask.TouchState	= TOUCH_MANAGE;
							}

							if (GPIO_ReadInputDataBit(KEY6_GPIO, KEY6_GPIO_PIN) == 0) //按HOME键
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
								SysTask.TouchState	= TOUCH_MANAGE_DISPLAY;
							}

							break;

						default:
							break;
					}
				}
				break;

			case TOUCH_DEL_USER:
				if (GPIO_ReadInputDataBit(KEY7_GPIO, KEY7_GPIO_PIN) == 0) //确定键松手检测
				{
					break;
				}

				{
					switch (SysTask.TouchSub)
					{
						case TOUCH_SUB_INIT: // *
							if (SysTask.nWaitTime == 0)
							{
								PS_ValidTempleteNum(&u16FingerCnt); //读库指纹个数

								if (u16FingerCnt == 0)
								{
									SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
									SysTask.nWaitTime	= 2000; //延时一定时间退出
									SysTask.TouchState	= TOUCH_MANAGE_DISPLAY;
									OLED_Clear();
									OLED_ShowString(0, 3, "Fingerprint template Null！", 12);
								}
								else 
								{
									SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
									SysTask.TouchSub	= TOUCH_GETKEY;
									u16FingerID 		= 0;
									u8SelectPosi		= 0;
									OLED_Clear();
									OLED_ShowString(0, 0, "Del ID (1~9)", 12);
									OLED_ShowString(32, 2, "-> 1", 12);
									OLED_ShowString(32, 4, "2", 12);
									OLED_ShowString(32, 6, "3", 12);
								}
							}

							break;

						case TOUCH_GETKEY: // *
							if (SysTask.nWaitTime)
							{
								break;
							}

							if (GPIO_ReadInputDataBit(KEY5_GPIO, KEY5_GPIO_PIN) == 0) //按时间选择键
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示

								SysTask.TouchSub	= TOUCH_GETKEY_DEBOUNCE; //防抖,松手检测
							}

							if (GPIO_ReadInputDataBit(KEY6_GPIO, KEY6_GPIO_PIN) == 0) //按HOME键
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
								SysTask.TouchState	= TOUCH_MANAGE_DISPLAY;
							}

							if (GPIO_ReadInputDataBit(KEY7_GPIO, KEY7_GPIO_PIN) == 0) //确定键
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
								SysTask.TouchSub	= TOUCH_SUB_WAIT;
								u16FingerID 		= u8SelectPosi;
								SysTask.nWaitTime	= 1000;
								OLED_Clear();
								OLED_ShowString(0, 3, "Sure Del ?", 12);
								OLED_ShowString(0, 5, "ID : ", 12);
								OLED_ShowChar(48, 5, '0' + u16FingerID, 12);
							}

							break;

						case TOUCH_GETKEY_DEBOUNCE: //防抖,松手检测
							if (SysTask.nFingerSubWaitT != 0)
							{
								break;
							}

							if ((GPIO_ReadInputDataBit(KEY5_GPIO, KEY5_GPIO_PIN) == 0)) //按时间选择键
							{
								SysTask.nFingerSubWaitT = 20; //20ms检测一次
							}
							else 
							{
								if (++u8SelectPosi >= 10)
									u8SelectPosi = 1;

								if (u8SelectPosi == 8)
								{
									OLED_ShowString(32, 2, "-> 8", 12);
									OLED_ShowString(32, 4, "9", 12);
									OLED_ShowString(32, 6, "1", 12);

								}
								else if (u8SelectPosi == 9)
								{
									OLED_ShowString(32, 2, "-> 9", 12);
									OLED_ShowString(32, 4, "1", 12);
									OLED_ShowString(32, 6, "2", 12);
								}
								else 
								{
									OLED_ShowString(32, 2, "-> ", 12);
									OLED_ShowChar(56, 2, '0' + u8SelectPosi, 12);
									OLED_ShowChar(32, 4, '1' + u8SelectPosi, 12);
									OLED_ShowChar(32, 6, '2' + u8SelectPosi, 12);
								}

								SysTask.TouchSub	= TOUCH_GETKEY;

							}

							break;

						case TOUCH_SUB_ENTER: //选择
							if (GPIO_ReadInputDataBit(KEY6_GPIO, KEY6_GPIO_PIN) == 0) //按HOME键
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
								SysTask.TouchSub	= TOUCH_GETKEY;
							}

							if (GPIO_ReadInputDataBit(KEY7_GPIO, KEY7_GPIO_PIN) == 0) //确定键
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
								SysTask.TouchSub	= TOUCH_SUB_AGAIN;
							}

							break;

						case TOUCH_SUB_AGAIN:
							if (GPIO_ReadInputDataBit(KEY7_GPIO, KEY7_GPIO_PIN) != 0) //确定键松手检测
							{
								ensure				= PS_DeletChar(u16FingerID, 1);

								if (ensure == 0)
								{

									OLED_Clear();
									SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
									SysTask.TouchSub	= TOUCH_GETKEY;
									SysTask.nWaitTime	= 1000; //延时一定时间退出
									OLED_ShowString(0, 3, "Del User Success", 16);

								}
								else 
								{
									OLED_Clear();
									SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
									SysTask.TouchSub	= TOUCH_GETKEY;
									SysTask.nWaitTime	= 1000; //延时一定时间退出
									OLED_ShowString(0, 3, "Del User Failed", 16);

								}
							}

							break;

						case TOUCH_SUB_WAIT: // *
							if ((GPIO_ReadInputDataBit(KEY7_GPIO, KEY7_GPIO_PIN) != 0)) //确定键松手检测
							{
								SysTask.TouchSub	= TOUCH_SUB_ENTER;
							}

							if (GPIO_ReadInputDataBit(KEY6_GPIO, KEY6_GPIO_PIN) == 0) //按HOME键
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
								SysTask.TouchState	= TOUCH_MANAGE_DISPLAY;
							}

							break;

						default:
							break;
					}
				}
				break;

			case TOUCH_DISPLAY_ID:
				if ((SysTask.nWaitTime == 0) && (GPIO_ReadInputDataBit(KEY5_GPIO, KEY5_GPIO_PIN) != 0) //按时间选?
		??键 松手检测，不然一直闪
				&& (GPIO_ReadInputDataBit(KEY6_GPIO, KEY6_GPIO_PIN) != 0) //按HOME键
				&& (GPIO_ReadInputDataBit(KEY7_GPIO, KEY7_GPIO_PIN) != 0)) //确定键
				{
					SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
					SysTask.nWaitTime	= 20;		//2s显示ID时间
					SysTask.TouchState	= TOUCH_KEY_CHECK;
					SysTask.TouchSub	= TOUCH_SUB_INIT;

					ShowLockMode();

				}

				break;

			case TOUCH_KEY_CHECK:
				if (SysTask.nWaitTime == 0)
				{
					switch (SysTask.TouchSub)
					{
						case TOUCH_SUB_INIT:
							if (GPIO_ReadInputDataBit(KEY6_GPIO, KEY6_GPIO_PIN) == 0) //按HOME键
							{
			
			SysTask.nShowTime = 0;

				OledInitTask();
							}
							if (GPIO_ReadInputDataBit(KEY1_GPIO, KEY1_GPIO_PIN) == 0) //按键按下
							{
								SysTask.nShowTime	= MOTO_ZERO_DETECT; //5秒时间进入显示
								SysTask.TouchSub	= TOUCH_SUB_WAIT; //防抖动
								SysTask.nWaitTime	= DEBOUNCE_TIME;

								//SysTask.AClockState = ~(SysTask.AClockState) & 0x01;//取反
								if ((SysTask.MotoAState == MOTO_STATE_STOP) ||
									 (SysTask.MotoAState == MOTO_STATE_IDLE))
								{
									SysTask.MotoAState	= MOTO_STATE_INIT;
									ShowLockMode();
									SysTask.AClockState = CLOCK_OFF; //关锁状态

								}
								else 
								{
									SysTask.AClockState = CLOCK_ON; //开锁状态
								}

							}

							if (GPIO_ReadInputDataBit(KEY2_GPIO, KEY2_GPIO_PIN) == 0) //按键按下
							{
								SysTask.nShowTime	= MOTO_ZERO_DETECT; //5秒时间进入显示
								SysTask.TouchSub	= TOUCH_SUB_WAIT; //防抖动
								SysTask.nWaitTime	= DEBOUNCE_TIME;

								if ((SysTask.MotoBState == MOTO_STATE_STOP) ||
									 (SysTask.MotoBState == MOTO_STATE_IDLE))
								{
									SysTask.MotoBState	= MOTO_STATE_INIT;
									ShowLockMode();
									SysTask.BClockState = CLOCK_OFF; //关锁状态

								}
								else 
								{
									SysTask.BClockState = CLOCK_ON; //开锁状态
								}
							}

							if (GPIO_ReadInputDataBit(KEY3_GPIO, KEY3_GPIO_PIN) == 0) //按键按下
							{
								SysTask.nShowTime	= MOTO_ZERO_DETECT; //5秒时间进入显示
								SysTask.TouchSub	= TOUCH_SUB_WAIT; //防抖动
								SysTask.nWaitTime	= DEBOUNCE_TIME;

								if ((SysTask.MotoCState == MOTO_STATE_STOP) ||
									 (SysTask.MotoCState == MOTO_STATE_IDLE))
								{
									SysTask.MotoCState	= MOTO_STATE_INIT;
									ShowLockMode();
									SysTask.CClockState = CLOCK_OFF; //关锁状态

								}
								else 
								{
									SysTask.CClockState = CLOCK_ON; //开锁状态
								}
							}

							if (GPIO_ReadInputDataBit(KEY4_GPIO, KEY4_GPIO_PIN) == 0) //按键按下 
							{
								SysTask.nShowTime	= MOTO_ZERO_DETECT; //5秒时间进入显示
								SysTask.TouchSub	= TOUCH_SUB_WAIT; //防抖动
								SysTask.nWaitTime	= DEBOUNCE_TIME;

								if ((SysTask.MotoDState == MOTO_STATE_STOP) ||
									 (SysTask.MotoDState == MOTO_STATE_IDLE))
								{
									SysTask.MotoDState	= MOTO_STATE_INIT;
									ShowLockMode();
									SysTask.DClockState = CLOCK_OFF; //关锁状态

								}
								else 
								{
									SysTask.DClockState = CLOCK_ON; //开锁状态
								}
							}

							// *******************************************************************
							// *	 PAUSE		 home		  RATION
							// *	 时间选择		 返回 			正反转选择  
							// *
							// *
							
							//功能键按下
							if (GPIO_ReadInputDataBit(KEY5_GPIO, KEY5_GPIO_PIN) == 0) //按时间选择键
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
								SysTask.TouchSub	= TOUCH_SUB_TIMER_S;
								u8MotoSelect		= 0;

								//								u8FunSelect 		= 0;
								OLED_Clear();
								OLED_ShowString(0, 0, "Choose which one", 12);
								OLED_DrawBMP(0, 3, 128, 7, BMP_FourClock); //显示主界面

							}

							if (GPIO_ReadInputDataBit(KEY6_GPIO, KEY6_GPIO_PIN) == 0) //按HOME键
							{
							}

							if (GPIO_ReadInputDataBit(KEY7_GPIO, KEY7_GPIO_PIN) == 0) //正反转选择
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
								SysTask.TouchSub	= TOUCH_SUB_FR_S;
								u8MotoSelect		= 0;

								//								u8FunSelect 		= 0;
								OLED_Clear();
								OLED_ShowString(0, 0, "Choose which one", 12);
								OLED_DrawBMP(0, 3, 128, 7, BMP_FourClock); //显示主界面
							}

							break;

						case TOUCH_SUB_TIMER_S: //时间功能选择
							if (GPIO_ReadInputDataBit(KEY1_GPIO, KEY1_GPIO_PIN) == 0) //按键按下
							{
								u8MotoSelect		= 1;
							}

							if (GPIO_ReadInputDataBit(KEY2_GPIO, KEY2_GPIO_PIN) == 0) //按键按下
							{
								u8MotoSelect		= 2;
							}

							if (GPIO_ReadInputDataBit(KEY3_GPIO, KEY3_GPIO_PIN) == 0) //按键按下
							{
								u8MotoSelect		= 3;
							}

							if (GPIO_ReadInputDataBit(KEY4_GPIO, KEY4_GPIO_PIN) == 0) //按键按下 
							{
								u8MotoSelect		= 4;
							}

							if (u8MotoSelect != 0) //有按键按下
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
								SysTask.TouchSub	= TOUCH_SUB_CH_TI;
								SysTask.nFingerSubWaitT = 0;
								u8SelectPosi		= 0;
								OLED_Clear();
								OLED_ShowChar(0, 0, 'A' + u8MotoSelect - 1, 16);
								OLED_DrawHorizontal(0, 16, 2, 4, 1);

								OLED_ShowString(48, 2, "->TPD", 12);
								OLED_ShowString(48, 3, "650", 12);
								OLED_ShowString(48, 4, "750", 12);
								OLED_ShowString(48, 5, "850", 12);
								OLED_ShowString(48, 6, "1000", 12);
								OLED_ShowString(48, 7, "1950", 12);
							}

							if (GPIO_ReadInputDataBit(KEY6_GPIO, KEY6_GPIO_PIN) == 0) //按HOME键
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
								SysTask.TouchState	= TOUCH_DISPLAY_ID;
							}

							break;

						case TOUCH_SUB_FR_S: //正反转选择
							if (GPIO_ReadInputDataBit(KEY1_GPIO, KEY1_GPIO_PIN) == 0) //按键按下
							{
								SysTask.nShowTime	= MOTO_ZERO_DETECT; //5秒时间进入显示
								u8MotoSelect		= 1;
								SysTask.TouchSub	= TOUCH_SUB_CH_DI;
							}

							if (GPIO_ReadInputDataBit(KEY2_GPIO, KEY2_GPIO_PIN) == 0) //按键按下
							{
								SysTask.nShowTime	= MOTO_ZERO_DETECT; //5秒时间进入显示
								u8MotoSelect		= 2;
								SysTask.TouchSub	= TOUCH_SUB_CH_DI;
							}

							if (GPIO_ReadInputDataBit(KEY3_GPIO, KEY3_GPIO_PIN) == 0) //按键按下
							{
								SysTask.nShowTime	= MOTO_ZERO_DETECT; //5秒时间进入显示
								u8MotoSelect		= 3;
								SysTask.TouchSub	= TOUCH_SUB_CH_DI;
							}

							if (GPIO_ReadInputDataBit(KEY4_GPIO, KEY4_GPIO_PIN) == 0) //按键按下 
							{
								SysTask.nShowTime	= MOTO_ZERO_DETECT; //5秒时间进入显示
								u8MotoSelect		= 4;
								SysTask.TouchSub	= TOUCH_SUB_CH_DI;
							}

							if (u8MotoSelect != 0) //有按键按下
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
								SysTask.nFingerSubWaitT = 0;
								u8SelectPosi		= 0;
								OLED_Clear();
								OLED_ShowChar(0, 0, 'A' + u8MotoSelect - 1, 16);
								OLED_DrawHorizontal(0, 16, 2, 4, 1);

								OLED_ShowString(48, 1, "->FWD", 12);
								OLED_ShowString(48, 3, "REV", 12);
								OLED_ShowString(48, 5, "F-R", 12);
							}

							if (GPIO_ReadInputDataBit(KEY6_GPIO, KEY6_GPIO_PIN) == 0) //按HOME键
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
								SysTask.TouchState	= TOUCH_DISPLAY_ID;
							}

							break;

						case TOUCH_SUB_CH_TI: //改变时间
							if (GPIO_ReadInputDataBit(KEY5_GPIO, KEY5_GPIO_PIN) == 0) //按时间选择键
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
								SysTask.TouchSub	= TOUCH_SUB_CH_TI_DE;

								OLED_Clear();
								OLED_ShowChar(0, 0, 'A' + u8MotoSelect - 1, 16);
								OLED_DrawHorizontal(0, 16, 2, 4, 1);

								if (++u8SelectPosi >= 6)
									u8SelectPosi = 0;

								switch (u8SelectPosi)
								{
									case 0:
										OLED_ShowString(48, 2, "->TPD", 12);
										OLED_ShowString(48, 3, "650", 12);
										OLED_ShowString(48, 4, "750", 12);
										OLED_ShowString(48, 5, "850", 12);
										OLED_ShowString(48, 6, "1000", 12);
										OLED_ShowString(48, 7, "1950", 12);
										break;

									case 1:
										OLED_ShowString(48, 2, "TPD", 12);
										OLED_ShowString(48, 3, "->650", 12);
										OLED_ShowString(48, 4, "750", 12);
										OLED_ShowString(48, 5, "850", 12);
										OLED_ShowString(48, 6, "1000", 12);
										OLED_ShowString(48, 7, "1950", 12);
										break;

									case 2:
										OLED_ShowString(48, 1, "TPD", 12);
										OLED_ShowString(48, 3, "650", 12);
										OLED_ShowString(48, 4, "->750", 12);
										OLED_ShowString(48, 5, "850", 12);
										OLED_ShowString(48, 6, "1000", 12);
										OLED_ShowString(48, 7, "1950", 12);
										break;

									case 3:
										OLED_ShowString(48, 2, "TPD", 12);
										OLED_ShowString(48, 3, "650", 12);
										OLED_ShowString(48, 4, "750", 12);
										OLED_ShowString(48, 5, "->850", 12);
										OLED_ShowString(48, 6, "1000", 12);
										OLED_ShowString(48, 7, "1950", 12);
										break;

									case 4:
										OLED_ShowString(48, 2, "TPD", 12);
										OLED_ShowString(48, 3, "650", 12);
										OLED_ShowString(48, 4, "750", 12);
										OLED_ShowString(48, 5, "850", 12);
										OLED_ShowString(48, 6, "->1000", 12);
										OLED_ShowString(48, 7, "1950", 12);
										break;

									case 5:
										OLED_ShowString(48, 2, "TPD", 12);
										OLED_ShowString(48, 3, "650", 12);
										OLED_ShowString(48, 4, "750", 12);
										OLED_ShowString(48, 5, "850", 12);
										OLED_ShowString(48, 6, "1000", 12);
										OLED_ShowString(48, 7, "->1950", 12);
										break;

									default:
										u8SelectPosi = 0;
										break;
								}


							}

							if (GPIO_ReadInputDataBit(KEY6_GPIO, KEY6_GPIO_PIN) == 0) //按HOME键
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
								SysTask.TouchState	= TOUCH_DISPLAY_ID;
							}

							if (GPIO_ReadInputDataBit(KEY7_GPIO, KEY7_GPIO_PIN) == 0) //确定键
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
								SysTask.TouchSub	= TOUCH_SUB_CH_TI_DE;
								switch (u8SelectPosi)
								{
									case 0:
										OLED_ShowString(48, 2, "->TPD *", 12);
										OLED_ShowString(48, 3, "650", 12);
										OLED_ShowString(48, 4, "750", 12);
										OLED_ShowString(48, 5, "850", 12);
										OLED_ShowString(48, 6, "1000", 12);
										OLED_ShowString(48, 7, "1950", 12);
										break;

									case 1:
										OLED_ShowString(48, 2, "TPD", 12);
										OLED_ShowString(48, 3, "->650 *", 12);
										OLED_ShowString(48, 4, "750", 12);
										OLED_ShowString(48, 5, "850", 12);
										OLED_ShowString(48, 6, "1000", 12);
										OLED_ShowString(48, 7, "1950", 12);
										break;

									case 2:
										OLED_ShowString(48, 1, "TPD", 12);
										OLED_ShowString(48, 3, "650", 12);
										OLED_ShowString(48, 4, "->750 *", 12);
										OLED_ShowString(48, 5, "850", 12);
										OLED_ShowString(48, 6, "1000", 12);
										OLED_ShowString(48, 7, "1950", 12);
										break;

									case 3:
										OLED_ShowString(48, 2, "TPD", 12);
										OLED_ShowString(48, 3, "650", 12);
										OLED_ShowString(48, 4, "750", 12);
										OLED_ShowString(48, 5, "->850 *", 12);
										OLED_ShowString(48, 6, "1000", 12);
										OLED_ShowString(48, 7, "1950", 12);
										break;

									case 4:
										OLED_ShowString(48, 2, "TPD", 12);
										OLED_ShowString(48, 3, "650", 12);
										OLED_ShowString(48, 4, "750", 12);
										OLED_ShowString(48, 5, "850", 12);
										OLED_ShowString(48, 6, "->1000 *", 12);
										OLED_ShowString(48, 7, "1950", 12);
										break;

									case 5:
										OLED_ShowString(48, 2, "TPD", 12);
										OLED_ShowString(48, 3, "650", 12);
										OLED_ShowString(48, 4, "750", 12);
										OLED_ShowString(48, 5, "850", 12);
										OLED_ShowString(48, 6, "1000", 12);
										OLED_ShowString(48, 7, "->1950 *", 12);
										break;

									default:
										u8SelectPosi = 0;
										break;
								}

								switch (u8MotoSelect) //ABCD
								{
									case 1:
										SysTask.MotoATime = (MotoTime)
										u8SelectPosi;
										SysTask.MotoARunTime = u16MoteTime_a[u8SelectPosi + 1][1];
										SysTask.MotoAWaitTime = u16MoteTime_a[u8SelectPosi + 1][2];
										break;

									case 2:
										SysTask.MotoBTime = (MotoTime)
										u8SelectPosi;
										SysTask.MotoBRunTime = u16MoteTime_a[u8SelectPosi + 1][1];
										SysTask.MotoBWaitTime = u16MoteTime_a[u8SelectPosi + 1][2];
										break;

									case 3:
										SysTask.MotoCTime = (MotoTime)
										u8SelectPosi;
										SysTask.MotoCRunTime = u16MoteTime_a[u8SelectPosi + 1][1];
										SysTask.MotoCWaitTime = u16MoteTime_a[u8SelectPosi + 1][2];
										break;

									case 4:
										SysTask.MotoDTime = (MotoTime)
										u8SelectPosi;
										SysTask.MotoDRunTime = u16MoteTime_a[u8SelectPosi + 1][1];
										SysTask.MotoDWaitTime = u16MoteTime_a[u8SelectPosi + 1][2];
										break;

									default:
										break;
								}
							}

							break;

						case TOUCH_SUB_CH_TI_DE: //改变时间，防抖,松手检测
							if (SysTask.nFingerSubWaitT != 0)
							{
								break;
							}

							if ((GPIO_ReadInputDataBit(KEY5_GPIO, KEY5_GPIO_PIN) == 0) //按时间选择键
							|| (GPIO_ReadInputDataBit(KEY6_GPIO, KEY6_GPIO_PIN) == 0) //按HOME键
							|| (GPIO_ReadInputDataBit(KEY7_GPIO, KEY7_GPIO_PIN) == 0)) //确定键
							{
								SysTask.nFingerSubWaitT = 20; //20ms检测一次
							}
							else 
							{
								SysTask.TouchSub	= TOUCH_SUB_CH_TI;
							}

							break;

						case TOUCH_SUB_CH_DI: //改变方向
							if (SysTask.nFingerSubWaitT != 0)
							{
								break;
							}

							if (GPIO_ReadInputDataBit(KEY5_GPIO, KEY5_GPIO_PIN) == 0) //按时间选择键
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
								SysTask.TouchSub	= TOUCH_SUB_CH_DI_DE;

								if (++u8SelectPosi >= 3)
									u8SelectPosi = 0;

								OLED_Clear();
								OLED_ShowChar(0, 0, 'A' + u8MotoSelect - 1, 16);
								OLED_DrawHorizontal(0, 16, 2, 4, 1);

								switch (u8SelectPosi)
								{
									case 0:
										OLED_ShowString(48, 1, "->FWD", 12);
										OLED_ShowString(48, 3, "REV", 12);
										OLED_ShowString(48, 5, "F-R", 12);
										break;

									case 1:
										OLED_ShowString(48, 1, "FWD", 12);
										OLED_ShowString(48, 3, "->REV", 12);
										OLED_ShowString(48, 5, "F-R", 12);
										break;

									case 2:
										OLED_ShowString(48, 1, "FWD", 12);
										OLED_ShowString(48, 3, "REV", 12);
										OLED_ShowString(48, 5, "->F-R", 12);
										break;

									default:
										u8SelectPosi = 0;
										break;
								}


							}

							if (GPIO_ReadInputDataBit(KEY6_GPIO, KEY6_GPIO_PIN) == 0) //按HOME键
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
								SysTask.TouchState	= TOUCH_DISPLAY_ID;
							}

							if (GPIO_ReadInputDataBit(KEY7_GPIO, KEY7_GPIO_PIN) == 0) //确定键
							{
								switch (u8SelectPosi)
								{
									case 0:
										OLED_ShowString(48, 1, "->FWD *", 12);
										OLED_ShowString(48, 3, "REV", 12);
										OLED_ShowString(48, 5, "F-R", 12);
										break;

									case 1:
										OLED_ShowString(48, 1, "FWD", 12);
										OLED_ShowString(48, 3, "->REV *", 12);
										OLED_ShowString(48, 5, "F-R", 12);
										break;

									case 2:
										OLED_ShowString(48, 1, "FWD", 12);
										OLED_ShowString(48, 3, "REV", 12);
										OLED_ShowString(48, 5, "->F-R *", 12);
										break;

									default:
										u8SelectPosi = 0;
										break;
								}
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
								SysTask.TouchSub	= TOUCH_SUB_CH_DI_DE;

								switch (u8MotoSelect)
								{
									case 1:
										SysTask.MotoAMode = (MotoFR)
										u8SelectPosi;
										break;

									case 2:
										SysTask.MotoBMode = (MotoFR)
										u8SelectPosi;
										;
										break;

									case 3:
										SysTask.MotoCMode = (MotoFR)
										u8SelectPosi;
										;
										break;

									case 4:
										SysTask.MotoDMode = (MotoFR)
										u8SelectPosi;
										;
										break;

									default:
										break;
								}
							}

							break;

						case TOUCH_SUB_CH_DI_DE: //改变方向 debounce
							if (SysTask.nFingerSubWaitT != 0)
							{
								break;
							}

							if ((GPIO_ReadInputDataBit(KEY5_GPIO, KEY5_GPIO_PIN) == 0) //按时间选择键
							|| (GPIO_ReadInputDataBit(KEY6_GPIO, KEY6_GPIO_PIN) == 0) //按HOME键
							|| (GPIO_ReadInputDataBit(KEY7_GPIO, KEY7_GPIO_PIN) == 0)) //确定键
							{
								SysTask.nFingerSubWaitT = 20; //20ms检测一次
							}
							else 
							{
								SysTask.TouchSub	= TOUCH_SUB_CH_DI;
							}

							break;

						case TOUCH_SUB_WAIT: //防抖
							if (SysTask.nWaitTime != 0)
								break;

							if ((GPIO_ReadInputDataBit(KEY1_GPIO, KEY1_GPIO_PIN) != 0) &&
								 (GPIO_ReadInputDataBit(KEY2_GPIO, KEY2_GPIO_PIN) != 0) &&
								 (GPIO_ReadInputDataBit(KEY3_GPIO, KEY3_GPIO_PIN) != 0) &&
								 (GPIO_ReadInputDataBit(KEY4_GPIO, KEY4_GPIO_PIN) != 0))
							{
								SysTask.TouchSub	= TOUCH_SUB_INIT;
								SysTask.nWaitTime	= DEBOUNCE_TIME; //防止一开始就检测到0点
							}

							break;

						default:
							break;
					}
				}

				break;

			case TOUCH_WAIT:
				if (SysTask.nWaitTime == 0)
				{
					OledInitTask();
				}
				else if (Remote_Rdy)
				{
					SysTask.nWaitTime	= 0;
				}

				break;

			default:
				break;
		}
	}
	*/
}


/*******************************************************************************
* 名称: 
* 功能: 
* 形参:		
* 返回: 无
* 说明: 
*******************************************************************************/
void LedTask(void)
{
	//LedTask();
}

/*******************************************************************************
* 名称: 
* 功能: 校验和
* 形参:		
* 返回: 无
* 说明: 
*******************************************************************************/
u8 CheckSum(u8 *pu8Addr, u8 u8Count)
{
    u8 i = 0;
    u8 u8Result = 0;

    for(i = 0; i < u8Count; i++)
    {
        u8Result += pu8Addr[i];
    }

    return ~u8Result;
}

/*******************************************************************************
* 名称: 
* 功能: 从机包发送
*		包头 + 从机地址 +      手表地址 + 命令 + 数据包[3] + 校验和 + 包尾
* 形参:		
* 返回: 无
* 说明: 
*******************************************************************************/
void SlavePackageSend(u8 u8SlaveAddr, u8 u8GlassAddr, u8 u8Cmd, u8 *u8Par)
{
	u8 i				= 0;
	u8 u8SendArr[9] 	=
	{
		0
	};

	u8SendArr[0]		= 0xAA; 					//包头
	u8SendArr[1]		= u8SlaveAddr;					//从机地址
	u8SendArr[2]		= u8GlassAddr;					//手表地址
	u8SendArr[3]		= u8Cmd;					//命令
	u8SendArr[4]		= u8Par[0]; 				//参数1
	u8SendArr[5]		= u8Par[1]; 				//参数2
	u8SendArr[6]		= u8Par[2]; 				//参数3

    u8SendArr[7] = CheckSum(u8SendArr, 7);         //校验和
	u8SendArr[8]		= 0x55; 					//包尾


	for (i = 0; i < 8; i++)
	{
		USART_SendData(USART3, u8SendArr[i]);
	}
}

/*******************************************************************************
* 名称: 平板发给主机的命令包处理
* 功能: 判断中断接收的数组有没有更新数据包 或者就绪包
* 形参:		
* 返回: -1, 失败 0:成功
* 说明: 
*******************************************************************************/
static int Usart1JudgeStr(void)
{
    //    包头 + 从机地址 + 手表地址　+ 命令 + 数据包[3] * 8 + 校验和 + 包尾
    //    包头：0XAA
    //    从机地址：0XFF：全部         0x01：第一个从机
    //    手表地址：0XFF：全部         0x01：第一个手表
    //    命令： CMD_UPDATE   、 CMD_READY
    //    数据：（锁开关 + 方向 + 时间）
    //    校验和：
    //    包尾： 0X55

	const char *data;
	u8 i, j;
    u8 str[8] = {0}; 
	str[0]=0xAA;

    u8 u8CheckSum = 0;

	data=strstr((const char*)USART1_RX_BUF,(const char*)str);
	if(!data)
        return -1;

    if(*(data + 3) == CMD_READY)   //如果是就绪包，则通知主机可以上传数据了
    {

        if(     (*(data + 1) == 0xFF)   //从机地址
            &&  (*(data + 2) >= 0xFF) //手表地址
            &&  (*(data + 29) == 0X55)) //包尾
                {
                    for(i = 0; i < 27; i++)
                        u8CheckSum += *(data + i);

                    if(u8CheckSum == 0)
                    {   
                        SysTask.bTabReady   = TRUE;
                        return 0;  
                    }
                }
    }
    else if((*(data + 1) >= 0x01)  && (*(data + 1) <= 0x04))   //从机地址
        if(((*(data + 2) >= 0x01)  && (*(data + 2) <= 0x08)) ||(*(data + 2) ==  0xFF)) //手表地址
            if((*(data + 3) == CMD_UPDATE)) //更新数据包
                if((*(data + 29) == 0X55)) //包尾
                {
                    for(i = 0; i < 27; i++)
                        u8CheckSum += *(data + i);

                    if(u8CheckSum == 0)
                    {
                        if(*(data + 2) ==  0xFF)   //该从机下面的全部手表
                        {
                            for(j = 0 ; j < 8; j++)
                            {
                                SysTask.SlaveStateSave[*(data + 1) - 1].GlassState[j].u8LockState  = *(data + 4 + 3 * j);
                                SysTask.SlaveStateSave[*(data + 1) - 1].GlassState[j].u8Dir        = *(data + 5 + 3 * j);
                                SysTask.SlaveStateSave[*(data + 1) - 1].GlassState[j].MotoTime     = (unsigned char)(*(data + 6 + 3 * j));
                            }
                        }
                        else  //单个手表的状态上传
                        {
//                            SysTask.SlaveStateSave[*(data + 1) - 1].GlassState[*(data + 2)].u8LockState  = *(data + 4 + 3 * (*(data + 2)));
//                            SysTask.SlaveStateSave[*(data + 1) - 1].GlassState[*(data + 2)].u8Dir        = *(data + 5 + 3 * (*(data + 2)));
//                            SysTask.SlaveStateSave[*(data + 1) - 1].GlassState[*(data + 2)].MotoTime     = *(data + 6 + 3 * (*(data + 2)));
                            SysTask.SlaveStateSave[*(data + 1) - 1].GlassState[*(data + 2)].u8LockState  = *(data + 4 );
                            SysTask.SlaveStateSave[*(data + 1) - 1].GlassState[*(data + 2)].u8Dir        = *(data + 5 );
                            SysTask.SlaveStateSave[*(data + 1) - 1].GlassState[*(data + 2)].MotoTime     = *(data + 6 );
                        }
                        return 0;    
                    }
                }
   return -1;   //出错返回
                
}


/*******************************************************************************
* 名称: 
* 功能: 平板给主机发送的命令包处理
* 形参:		
* 返回: 无
* 说明: 
*******************************************************************************/
void TabletToHostTask(void)
{
    if(USART3_RX_STA & 0X8000)//接收到从机数据
    {
        Usart1JudgeStr();
    }
}


/*******************************************************************************
* 名称: 
* 功能: 判断中断接收的数组有没有应答包, 从机发给主机的数据
* 形参:		
* 返回: -1, 失败 0:成功
* 说明: 
*******************************************************************************/
static int Usart3JudgeStr(void)
{
    //    包头 + 从机地址 + 手表地址　+ 命令 + 数据包[3] * 8 + 校验和 + 包尾
    //    包头：0XAA
    //    从机地址：0XFF：全部         0x01：第一个从机
    //    手表地址：0XFF：全部         0x01：第一个手表
    //    命令：CMD_ACK
    //    数据：（锁开关 + 方向 + 时间）
    //    校验和：
    //    包尾： 0X55

	const char *data;
	u8  i, j;
    u8 str[8] = {0};
	str[0]=0xAA;

    u8 u8CheckSum = 0;

	data=strstr((const char*)USART3_RX_BUF,(const char*)str);
	if(!data)
        return -1;

    if((*(data + 1) >= 0x01)  && (*(data + 1) <= 0x04))   //从机地址
        if(((*(data + 2) >= 0x01)  && (*(data + 2) <= 0x08)) ||(*(data + 2) ==  0xFF)) //手表地址
            if((*(data + 3) == CMD_ACK)) //应答包
                if((*(data + 29) == 0X55)) //包尾
                {
                    for(i = 0; i < 27; i++)
                        u8CheckSum += *(data + i);

                    if(u8CheckSum == 0)
                    {
                        if(*(data + 2) ==  0xFF)   //该从机下面的全部手表
                        {
                            for(j = 0 ; j < 8; j++)
                            {
                                SysTask.SlaveState[*(data + 1) - 1].GlassState[j].u8LockState  = *(data + 4 + 3 * j);
                                SysTask.SlaveState[*(data + 1) - 1].GlassState[j].u8Dir        = *(data + 5 + 3 * j);
                                SysTask.SlaveState[*(data + 1) - 1].GlassState[j].MotoTime     = (int)*(data + 6 + 3 * j);
                            }
                        }
                        else  //单个手表的状态上传
                        {
//                            SysTask.SlaveState[*(data + 1) - 1].GlassState[*(data + 2)].u8LockState  = *(data + 4 + 3 * (*(data + 2)));
//                            SysTask.SlaveState[*(data + 1) - 1].GlassState[*(data + 2)].u8Dir        = *(data + 5 + 3 * (*(data + 2)));
//                            SysTask.SlaveState[*(data + 1) - 1].GlassState[*(data + 2)].MotoTime     = *(data + 6 + 3 * (*(data + 2)));
                            SysTask.SlaveState[*(data + 1) - 1].GlassState[*(data + 2)].u8LockState  = *(data + 4 );
                            SysTask.SlaveState[*(data + 1) - 1].GlassState[*(data + 2)].u8Dir        = *(data + 5 );
                            SysTask.SlaveState[*(data + 1) - 1].GlassState[*(data + 2)].MotoTime     = *(data + 6 );
                        }
                        return 0;    
                    }
                }
   return -1;   //出错返回
                
}


/*******************************************************************************
* 名称: 
* 功能: 检测有几个从机，并获取从机状态
* 形参:		
* 返回: 无
* 说明: 
*******************************************************************************/
void SlaveDetected(void)
{
	static u8 u8Addr			= 1;						//0xFF : 广播地址， 0x00 ：主机地址
    u8 u8aSendData[3]   = {0};
    
	switch (SysTask.SendSubState)
	{
		case SEND_SUB_INIT:
            SlavePackageSend(u8Addr, 0xFF, CMD_GET_ATTR, u8aSendData);
            SysTask.SendSubState = SEND_SUB_WAIT;
            SysTask.u16SubWaitTime = SLAVE_WAIT_TIME;
			break;
            
        case SEND_SUB_WAIT:
            if(SysTask.u16SubWaitTime == 0)  //超时无应答
            {
                SysTask.u8SlaveCount = u8Addr - 1;
                g_SlaveSendAddr      = SLAVE_SEND_ALL | SysTask.u8SlaveCount;  
                g_GlassSendAddr      = GLASS_SEND_ALL; 
                SysTask.SendState    = SEND_TABLET_ALL; //发送全部从机状态给平板
                SysTask.SendSubState = SEND_SUB_INIT;
            }
            else if(USART3_RX_STA & 0X8000)//接收到从机数据
            {
                if(!Usart3JudgeStr())
                {
                    if(u8Addr != 4)
                    {
                        u8Addr += 1;
                        SysTask.u16SubWaitTime = SEND_SUB_INIT;
                    }
                    else
                    {
                        SysTask.SendState    = SEND_TABLET_ALL; //发送全部从机状态给平板
                        SysTask.SendSubState = SEND_SUB_INIT;
                    }
                }
            }
            break;
		default:
			break;
	}
}


/*******************************************************************************
* 名称: 
* 功能: 主机发送全部状态给平板
* 形参:		
* 返回: 无
* 说明: 
*******************************************************************************/
void SendToTablet(u8 u8SlaveAddr, u8 u8GlassAddr)
{
    //    包头 + 地址 + 命令 + 数据包[96】 + 校验和 + 包尾
    //    包头：0XAA
    //    从机地址：0XFF：全部         0x01：第一个从机
    //    手表地址：0XFF：全部         0x01：第一个手表
    //    命令：
    //    数据：（锁开关 + 方向 + 时间）* 4 * 8 = 96
    //    校验和：
    //    包尾： 0X55

    u8 i;
    u8 u8aSendArr[102] = {0};
    SendArrayUnion_u SendArrayUnion;

    memcpy(SendArrayUnion.SlaveState, SysTask.SlaveState, sizeof(SysTask.SlaveState));
    memcpy(SysTask.SlaveStateSave, SysTask.SlaveState, sizeof(SysTask.SlaveState));

    u8aSendArr[0] = 0xAA;
    u8aSendArr[1] = u8SlaveAddr;
    u8aSendArr[2] = u8GlassAddr;
    u8aSendArr[3] = 0x11;       //命令保留用


    for(i = 0; i < 96; i++)
    {
        u8aSendArr[4 + i] = SendArrayUnion.u8SendArray[i];
    }

    u8aSendArr[100] = CheckSum(u8aSendArr, 100); //校验和
    u8aSendArr[101] = 0x55;  //包尾s 
}

/*******************************************************************************
* 名称: 
* 功能: 发送初始化   获取全部从机状态
* 形参:		
* 返回: 无
* 说明: 
*******************************************************************************/
void SendInitTask(void)
{
	switch (SysTask.SendState)
	{
		case SEND_INIT:
            if(SysTask.u16BootTime == 0)   //等待从机启动完全
            {
                SlaveDetected();
            }
			break;
        
		case SEND_TABLET_ALL: //发送全部从机给平板
		    if(SysTask.bTabReady == TRUE)
            
{
                g_SlaveSendAddr      = SLAVE_SEND_ALL | SysTask.u8SlaveCount; 
    			SendToTablet(g_SlaveSendAddr, g_GlassSendAddr);  //0x1? 所有从机  ？代表从机个数
    			SysTask.SendState = SEND_IDLE;
            }
			break;

        /* 现在阶段没有使用这部分的内容，因为不做应答检验，所以下面这部分暂时用不到
		case SEND_TABLET_SINGLE://发送单个从机状态给平板
			SendToTablet(g_SlaveSendAddr, g_GlassSendAddr);
			break;
        
        
		case SEND_SLAVE_ALL: //主机发送群控命令给从机
            g_SlaveSendAddr      = SLAVE_SEND_ALL | SysTask.u8SlaveCount; 
			SendToTablet(g_SlaveSendAddr, g_GlassSendAddr);  //0x1? 所有从机  ？代表从机个数
			break;
        
		case SEND_SLAVE_SINGLE://主机发送单个从机控制命令
			SendToTablet(g_SlaveSendAddr, g_GlassSendAddr);
			break;
        */
		case SEND_IDLE:
		default:
			break;
	}
}


/*******************************************************************************
* 名称: 
* 功能: 主机接发送给从机发送的命令包处理
* 形参:		
* 返回: 无
* 说明: 
*******************************************************************************/
void HostToTabletTask(void)
{
    static u8 i = 0;
    static u8 j = 0;

    static u8 state = 0;


//SysTask.SlaveStateSave, SysTask.SlaveState
    switch(state)
        {
        case HtoS_CHECK:
            if(     (SysTask.SlaveStateSave[j].GlassState[i].MotoTime != SysTask.SlaveState[j].GlassState[i].MotoTime)
               ||   (SysTask.SlaveStateSave[j].GlassState[i].MotoTime != SysTask.SlaveState[j].GlassState[i].MotoTime)
               ||   (SysTask.SlaveStateSave[j].GlassState[i].MotoTime != SysTask.SlaveState[j].GlassState[i].MotoTime))

            {
                state = HtoS_SEND;
            }
            else
            {
                if(i == 7){
                    j++;
                    if(j == 4) j = 0;
                    i = 0;
                 }
                else
                    i++;
            }
            break;
        case HtoS_SEND:
            SysTask.SlaveStateSave[j].GlassState[i].MotoTime = SysTask.SlaveState[j].GlassState[i].MotoTime;
            SysTask.SlaveStateSave[j].GlassState[i].MotoTime = SysTask.SlaveState[j].GlassState[i].MotoTime;
            SysTask.SlaveStateSave[j].GlassState[i].MotoTime = SysTask.SlaveState[j].GlassState[i].MotoTime;
            SysTask.u16HtoSWaitTime = 50; //延时50ms
            state = HtoS_WAIT;
            break;
        case HtoS_WAIT:
            if(SysTask.u16HtoSWaitTime == 0)
                state = HtoS_CHECK;
            break;
        default:
            break;
    }
}

/*******************************************************************************
* 名称: 
* 功能: 
* 形参:		
* 返回: 无
* 说明: 
*******************************************************************************/
void MainTask(void)
{
	LedTask();
	SendInitTask();								//发送初始化
	TabletToHostTask(); 							//接收平板命令任务处理
	HostToTabletTask(); 					//主机发送给从机的命令任务处理

	//SlaveToHostTask();   //从机发送到主机端的接收任务处理,  因为不做应答校验，所以这部分现在用不到，
	                        //只有在detect 或者命令下发时会发送数据给主机
	FingerTouchTask();								//指纹模块
}


/*******************************************************************************
* 名称: 
* 功能: 
* 形参:		
* 返回: 无
* 说明: 
*******************************************************************************/
void SysInit(void)
{
	SysTask.TouchState	= TOUCH_INIT;
	SysTask.TouchSub	= TOUCH_SUB_INIT;
	SysTask.nWaitTime	= 0;
	SysTask.nTick		= 0;


	SysTask.SendState	= SEND_INIT;
	SysTask.SendSubState = SEND_SUB_INIT;
    SysTask.bTabReady   = FALSE;
    SysTask.u16BootTime = 3000;   //3秒等待从机启动完全

}


