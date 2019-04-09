
#include "usart3.h"
#include "usart1.h" 	 
#include "string.h"	 

#include "SysComment.h"
#include "stm32f10x_it.h"
#include "stm32f10x_iwdg.h"
#include "san_flash.h"

#include "as608.h"
#include "bmp.h"



#define     SLAVE_WAIT_TIME         3000    //3s �ӻ�Ӧ��ʱ��


#define     SLAVE_SEND_ALL        0X10   //BIT[7]:����ȫ��      BIT[6~0]:����ӻ�����
#define     GLASS_SEND_ALL        0XFF  //���͸��ôӻ������е��ֱ�


//�ڲ�����
static vu16 	mDelay;
u8    g_SlaveSendAddr = 0;   //Ҫ���͵Ĵӻ���ַ
u8    g_GlassSendAddr = 0;   //Ҫ���͵��ֱ��ַ

//�ڲ�����
void SysTickConfig(void);


/*******************************************************************************
* ����: SysTick_Handler
* ����: ϵͳʱ�ӽ���1MS
* �β�:		
* ����: ��
* ˵��: 
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
			IWDG_ReloadCounter();					/*ιSTM32����Ӳ����*/

			if ((++mSysSoftDog) > 5) /*��system  DOG 2S over*/
			{
				mSysSoftDog 		= 0;
				NVIC_SystemReset();
			}
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
* ����: 
* ����: 
* �β�:		
* ����: ��
* ˵��: 
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
* ����: Strcpy()
* ����: 
* �β�:		
* ����: ��
* ˵��: 
******************************************************************************/
void Strcpy(u8 * str1, u8 * str2, u8 len)
{
	for (; len > 0; len--)
	{
		*str1++ 			= *str2++;
	}
}


/*******************************************************************************
* ����: Strcmp()
* ����: 
* �β�:		
* ����: ��
* ˵��: 
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
* ����: Sys_DelayMS()
* ����: ϵͳ�ӳٺ���
* �β�:		
* ����: ��
* ˵��: 
*******************************************************************************/
void Sys_DelayMS(uint16_t nms)
{
	mDelay				= nms + 1;

	while (mDelay != 0x0)
		;
}


/*******************************************************************************
* ����: Sys_LayerInit
* ����: ϵͳ��ʼ��
* �β�:		
* ����: ��
* ˵��: 
*******************************************************************************/
void Sys_LayerInit(void)
{
	SysTickConfig();

	mSysSec 			= 0;
	mSysTick			= 0;
	SysTask.mUpdate 	= TRUE;
}


/*******************************************************************************
* ����: 
* ����: 
* �β�:		
* ����: ��
* ˵��: 
*******************************************************************************/
void Sys_IWDGConfig(u16 time)
{
	/* д��0x5555,�����������Ĵ���д�빦�� */
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

	/* ����ʱ�ӷ�Ƶ,40K/64=0.625K()*/
	IWDG_SetPrescaler(IWDG_Prescaler_64);

	/* ι��ʱ�� TIME*1.6MS .ע�ⲻ�ܴ���0xfff*/
	IWDG_SetReload(time);

	/* ι��*/
	IWDG_ReloadCounter();

	/* ʹ�ܹ���*/
	IWDG_Enable();
}


/*******************************************************************************
* ����: Sys_IWDGReloadCounter
* ����: 
* �β�:		
* ����: ��
* ˵��: 
*******************************************************************************/
void Sys_IWDGReloadCounter(void)
{
	mSysSoftDog 		= 0;						//ι��
}


/*******************************************************************************
* ����: 
* ����: 
* �β�:		
* ����: ��
* ˵��: 
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
* ����: 
* ����: 
* �β�:		
* ����: ��
* ˵��: 
*******************************************************************************/
void RemoteAddUser(void)
{
	/*
		u8 key;
		u8 ensure;

		//	u8 u8Result 		= 0;
		u16 u8Shownum;
		u16 u16FingerCnt	= 0;						//ָ��ģ�����
		static u16 u16FingerID = 0; 					//ָ��ģ��ID
		static u8 u8Position = 0;						//��ʾλ��
		static u8 u8InputCnt = 0;						//�������

		switch (SysTask.RemoteSub)
		{
			case INIT: // *
				if (SysTask.nWaitTime == 0)
				{
					PS_ValidTempleteNum(&u16FingerCnt); //����ָ�Ƹ���

					if (u16FingerCnt >= FINGER_MAX_CNT)
					{
						SysTask.nShowTime	= REMOTE_SHOW_TIME; //5��ʱ�������ʾ
						SysTask.RemoteSub	= WAIT;
						SysTask.nWaitTime	= 2000; 	//��ʱһ��ʱ���˳�
						OLED_Clear();
						OLED_ShowString(0, 3, "Fingerprint template full��", 12);
					}
					else 
					{
						SysTask.nShowTime	= REMOTE_SHOW_TIME; //5��ʱ�������ʾ
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

			case ENTER: //ѡ��
				if ((PS_Sta) && (SysTask.nWaitTime == 0)) //�����ָ�ư���
				{

					SysTask.nShowTime	= REMOTE_SHOW_TIME; //5��ʱ�������ʾ
					ensure				= PS_GetImage();

					if (ensure == 0x00)
					{
						ensure				= PS_GenChar(CharBuffer1); //��������

						if (ensure == 0x00)
						{
							OLED_Clear();
							OLED_ShowString(0, 3, "Please press  agin", 12);
							SysTask.nWaitTime	= 2000; //��ʱһ��ʱ����ȥ�ɼ�
							SysTask.RemoteSub	= AGAIN; //�����ڶ���						
						}
					}
				}

				break;

			case AGAIN:
				if ((PS_Sta) && (SysTask.nWaitTime == 0)) //�����ָ�ư���
				{
					SysTask.nShowTime	= REMOTE_SHOW_TIME; //5��ʱ�������ʾ
					ensure				= PS_GetImage();

					if (ensure == 0x00)
					{
						ensure				= PS_GenChar(CharBuffer2); //��������

						if (ensure == 0x00)
						{
							ensure				= PS_Match(); //�Ա�����ָ��

							if (ensure == 0x00) //�ɹ�
							{

								ensure				= PS_RegModel(); //����ָ��ģ��

								if (ensure == 0x00)
								{

									ensure				= PS_StoreChar(CharBuffer2, u16FingerID); //����ģ��

									if (ensure == 0x00)
									{
										OLED_Clear();
										SysTask.nShowTime	= REMOTE_SHOW_TIME; //5��ʱ�������ʾ
										SysTask.RemoteSub	= WAIT;
										SysTask.nWaitTime	= 3000; //��ʱһ��ʱ���˳�
										OLED_ShowString(0, 3, "Add User Success", 16);
									}
									else 
									{ //ʧ��
										OLED_Clear();
										OLED_ShowString(0, 3, "Store Failed", 16);
									}
								}
								else 
								{ //ʧ��
									OLED_Clear();
									OLED_ShowString(0, 3, "Generate Template  Failed", 12);
								}
							}
							else 
							{ //ʧ��
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
					SysTask.nShowTime	= REMOTE_SHOW_TIME; //ˢ����ʾʱ��

					key 				= Remote_Process();

					if (key != REMOTE_KEY_ERR)
					{
						if (key == REMOTE_KEY_OK) //ȷ����
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
						else if (key == REMOTE_KEY_RETURN) //���ؼ�
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
* ����: 
* ����: 
* �β�:		
* ����: ��
* ˵��: 
*******************************************************************************/
void RemoteDelUser(void)
{
	/*
		u8 key;
		u8 ensure;

		//	u8 u8Result 		= 0;
		u16 u8Shownum;
		u16 u16FingerCnt	= 0;						//ָ��ģ�����
		static u16 u16FingerID = 0; 					//ָ��ģ��ID
		static u8 u8Position = 0;						//��ʾλ��
		static u8 u8InputCnt = 0;						//�������

		switch (SysTask.RemoteSub)
		{
			case INIT: // *
				if (SysTask.nWaitTime == 0)
				{
					PS_ValidTempleteNum(&u16FingerCnt); //����ָ�Ƹ���

					if (u16FingerCnt == 0)
					{
						SysTask.nShowTime	= REMOTE_SHOW_TIME; //5��ʱ�������ʾ
						SysTask.RemoteSub	= WAIT;
						SysTask.nWaitTime	= 2000; 	//��ʱһ��ʱ���˳�
						OLED_Clear();
						OLED_ShowString(0, 3, "Fingerprint template empty��", 12);
					}
					else 
					{
						SysTask.nShowTime	= REMOTE_SHOW_TIME; //5��ʱ�������ʾ
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

			case ENTER: //ѡ��
				if (u16FingerID == 300) //ȫ��ɾ��
					ensure = PS_Empty();
				else 
					ensure = PS_DeletChar(u16FingerID, 1);

				if (ensure == 0)
				{

					OLED_Clear();
					SysTask.nShowTime	= REMOTE_SHOW_TIME; //5��ʱ�������ʾ
					SysTask.RemoteSub	= WAIT;
					SysTask.nWaitTime	= 3000; 		//��ʱһ��ʱ���˳�
					OLED_ShowString(0, 3, "Del User Success", 16);

				}
				else 
				{
					OLED_Clear();
					SysTask.nShowTime	= REMOTE_SHOW_TIME; //5��ʱ�������ʾ
					SysTask.RemoteSub	= INIT;
					SysTask.nWaitTime	= 3000; 		//��ʱһ��ʱ���˳�
					OLED_ShowString(0, 3, "Del User Failed", 16);

				}

				//PS_ValidTempleteNum(&u16FingerCnt); //����ָ�Ƹ���
				break;

			case GETKEY: // *
				if (Remote_Rdy)
				{
					SysTask.nShowTime	= REMOTE_SHOW_TIME; //ˢ����ʾʱ��

					key 				= Remote_Process();

					if (key != REMOTE_KEY_ERR)
					{
						if (key == REMOTE_KEY_OK) //ȷ����
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
						else if (key == REMOTE_KEY_RETURN) //���ؼ�
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


#define DEBOUNCE_TIME			40 //40ms ����
void FingerTouchTask(void)
{
	/*SearchResult seach;
	u8 ensure;
	static u8 u8MatchCnt = 0;						//ƥ��ʧ�ܴ�����Ĭ��ƥ��MATCH_FINGER_CNT�� 
	static u8 u8MotoSelect = 0;
	static u8 u8SelectPosi = 0;
	static u16 u16FingerID = 0; 					//ָ��ģ��ID
	u16 u16FingerCnt	= 0;						//ָ��ģ�����


	if ((SysTask.RemoteState == SAN_DEF) || (SysTask.RemoteState == WRITE_PASSWD))
	{
		switch (SysTask.TouchState)
		{
			case TOUCH_INIT:
				if (PS_Sta) //��ָ�ư���
				{
					SysTask.nWaitTime	= 100;		//һ��ʱ���ټ��
					SysTask.TouchState	= TOUCH_CHECK;
					u8MatchCnt			= 0;
				}

				break;

			case TOUCH_CHECK:
				if (SysTask.nWaitTime == 0) //��ָ�ư���
				{
					ensure				= PS_GetImage();

					if (ensure == 0x00) //���������ɹ�
					{
						//PS_ValidTempleteNum(&u16FingerID); //����ָ�Ƹ���
						ensure				= PS_GenChar(CharBuffer1);

						if (ensure == 0x00) //���������ɹ�
						{
							ensure				= PS_Search(CharBuffer1, 0, FINGER_MAX_CNT, &seach);

							if (ensure == 0) //ƥ��ɹ�
							{
								u16FingerID 		= seach.pageID;
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5��ʱ�������ʾ
								SysTask.nWaitTime	= SysTask.nTick + 1000; //��ʱһ��ʱ�� ʵ���ʱֻ����?
		?300ms
								SysTask.TouchSub	= TOUCH_SUB_INIT;

								if (u16FingerID == 0) //�����û�����
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
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5��ʱ�������ʾ
								SysTask.nWaitTime	= SysTask.nTick + 2000; //��ʱһ��ʱ���˳�
								SysTask.TouchState	= TOUCH_WAIT;


								OLED_Clear();
								OLED_ShowString(0, 3, "Can't fined fingerprin", 16);
							}
						}
					}

					if (ensure) //ƥ��ʧ��
					{
						u8MatchCnt++;
					}
				}

				break;

			case TOUCH_MANAGE_DISPLAY:
				if ((SysTask.nWaitTime == 0) && (GPIO_ReadInputDataBit(KEY5_GPIO, KEY5_GPIO_PIN) != 0) //��ʱ��ѡ?
		??�� ���ּ�⣬��Ȼһֱ��
				&& (GPIO_ReadInputDataBit(KEY6_GPIO, KEY6_GPIO_PIN) != 0) //��HOME��
				&& (GPIO_ReadInputDataBit(KEY7_GPIO, KEY7_GPIO_PIN) != 0)) //ȷ����
				{
					SysTask.nShowTime	= REMOTE_SHOW_TIME; //5��ʱ�������ʾ
					SysTask.nWaitTime	= 20;		//2s��ʾIDʱ��
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
				if (GPIO_ReadInputDataBit(KEY5_GPIO, KEY5_GPIO_PIN) == 0) //��ʱ��ѡ���
				{
					SysTask.nShowTime	= REMOTE_SHOW_TIME; //5��ʱ�������ʾ
					SysTask.TouchState	= TOUCH_MANAGE_CHOOSE; //����,���ּ��
				}

				if (GPIO_ReadInputDataBit(KEY6_GPIO, KEY6_GPIO_PIN) == 0) //��HOME��
				{
						   SysTask.nShowTime = 0;
				
							   OledInitTask();
				}

				if (GPIO_ReadInputDataBit(KEY7_GPIO, KEY7_GPIO_PIN) == 0) //ȷ����
				{
					SysTask.nShowTime	= REMOTE_SHOW_TIME; //5��ʱ�������ʾ
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

			case TOUCH_MANAGE_CHOOSE: //����,���ּ��
				if (SysTask.nFingerSubWaitT != 0)
				{
					break;
				}

				if ((GPIO_ReadInputDataBit(KEY5_GPIO, KEY5_GPIO_PIN) == 0) //��ʱ��ѡ���
				|| (GPIO_ReadInputDataBit(KEY6_GPIO, KEY6_GPIO_PIN) == 0) //��HOME��
				|| (GPIO_ReadInputDataBit(KEY7_GPIO, KEY7_GPIO_PIN) == 0)) //ȷ����
				{
					SysTask.nFingerSubWaitT = 20;	//20ms���һ��
				}
				else 
				{
					SysTask.nShowTime	= REMOTE_SHOW_TIME; //5��ʱ�������ʾ

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
				if (GPIO_ReadInputDataBit(KEY7_GPIO, KEY7_GPIO_PIN) == 0) //ȷ�������ּ��
				{
					break;
				}

				{
					switch (SysTask.TouchSub)
					{
						case TOUCH_SUB_INIT: // *
							if (SysTask.nWaitTime == 0)
							{
								PS_ValidTempleteNum(&u16FingerCnt); //����ָ�Ƹ���

								if (u16FingerCnt >= FINGER_MAX_CNT)
								{
									SysTask.nShowTime	= REMOTE_SHOW_TIME; //5��ʱ�������ʾ
									SysTask.nWaitTime	= 2000; //��ʱһ��ʱ���˳�
									SysTask.TouchState	= TOUCH_MANAGE_DISPLAY;
									OLED_Clear();
									OLED_ShowString(0, 3, "Fingerprint template full��", 12);
								}
								else 
								{
									SysTask.nShowTime	= REMOTE_SHOW_TIME; //5��ʱ�������ʾ
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
							if (GPIO_ReadInputDataBit(KEY5_GPIO, KEY5_GPIO_PIN) == 0) //��ʱ��ѡ���
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5��ʱ�������ʾ

								SysTask.TouchSub	= TOUCH_GETKEY_DEBOUNCE; //����,���ּ��
							}

							if (GPIO_ReadInputDataBit(KEY6_GPIO, KEY6_GPIO_PIN) == 0) //��HOME��
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5��ʱ�������ʾ
								SysTask.TouchState	= TOUCH_MANAGE_DISPLAY;
							}

							if (GPIO_ReadInputDataBit(KEY7_GPIO, KEY7_GPIO_PIN) == 0) //ȷ����
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5��ʱ�������ʾ
								SysTask.TouchSub	= TOUCH_SUB_ENTER;
								u16FingerID 		= u8SelectPosi;
								SysTask.nWaitTime	= 1000;
								OLED_Clear();
								OLED_ShowString(0, 3, "Please press", 12);
							}

							break;

						case TOUCH_GETKEY_DEBOUNCE: //����,���ּ��
							if (SysTask.nFingerSubWaitT != 0)
							{
								break;
							}

							if ((GPIO_ReadInputDataBit(KEY5_GPIO, KEY5_GPIO_PIN) == 0)) //��ʱ��ѡ���
							{
								SysTask.nFingerSubWaitT = 20; //20ms���һ��
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

						case TOUCH_SUB_ENTER: //ѡ��
							if ((PS_Sta) && (SysTask.nWaitTime == 0)) //�����ָ�ư���
							{

								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5��ʱ�������ʾ
								ensure				= PS_GetImage();

								if (ensure == 0x00)
								{
									ensure				= PS_GenChar(CharBuffer1); //��������

									if (ensure == 0x00)
									{
										OLED_Clear();
										OLED_ShowString(0, 3, "Please press  agin", 12);
										SysTask.nWaitTime	= 1000; //��ʱһ��ʱ����ȥ�ɼ�
										SysTask.TouchSub	= TOUCH_SUB_AGAIN; //�����ڶ���						
									}
								}
							}

							if (GPIO_ReadInputDataBit(KEY6_GPIO, KEY6_GPIO_PIN) == 0) //��HOME��
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5��ʱ�������ʾ
								SysTask.TouchState	= TOUCH_MANAGE_DISPLAY;
							}

							break;

						case TOUCH_SUB_AGAIN:
							if ((PS_Sta) && (SysTask.nWaitTime == 0)) //�����ָ�ư���
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5��ʱ�������ʾ
								ensure				= PS_GetImage();

								if (ensure == 0x00)
								{
									ensure				= PS_GenChar(CharBuffer2); //��������

									if (ensure == 0x00)
									{
										ensure				= PS_Match(); //�Ա�����ָ��

										if (ensure == 0x00) //�ɹ�
										{

											ensure				= PS_RegModel(); //����ָ��ģ��

											if (ensure == 0x00)
											{

												ensure				= PS_StoreChar(CharBuffer2, u16FingerID); //����ģ��

												if (ensure == 0x00)
												{
													OLED_Clear();
													SysTask.nShowTime	= REMOTE_SHOW_TIME; //5��ʱ�������ʾ
													SysTask.TouchSub	= TOUCH_SUB_WAIT;
													SysTask.nWaitTime	= 3000; //��ʱһ��ʱ���˳�
													OLED_ShowString(0, 3, "Add User Success", 16);
												}
												else 
												{ //ʧ��
													OLED_Clear();
													OLED_ShowString(0, 3, "Store Failed", 16);
													SysTask.TouchSub	= TOUCH_SUB_ENTER;
													SysTask.nWaitTime	= 3000; //��ʱһ��ʱ���˳�
												}
											}
											else 
											{ //ʧ��
												OLED_Clear();
												OLED_ShowString(0, 3, "Generate Template  Failed", 12);
												SysTask.TouchSub	= TOUCH_SUB_ENTER;
												SysTask.nWaitTime	= 3000; //��ʱһ��ʱ���˳�
											}
										}
										else 
										{ //ʧ��
											OLED_Clear();
											OLED_ShowString(0, 3, "Match  Failed", 12);
											SysTask.TouchSub	= TOUCH_SUB_ENTER;
											SysTask.nWaitTime	= 3000; //��ʱһ��ʱ���˳�
										}
									}
								}
							}

							if (GPIO_ReadInputDataBit(KEY6_GPIO, KEY6_GPIO_PIN) == 0) //��HOME��
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5��ʱ�������ʾ
								SysTask.TouchState	= TOUCH_MANAGE_DISPLAY;
							}

							break;

						case TOUCH_SUB_WAIT: // *
							if ((SysTask.nWaitTime == 0))
							{
								SysTask.TouchState	= TOUCH_MANAGE;
							}

							if (GPIO_ReadInputDataBit(KEY6_GPIO, KEY6_GPIO_PIN) == 0) //��HOME��
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5��ʱ�������ʾ
								SysTask.TouchState	= TOUCH_MANAGE_DISPLAY;
							}

							break;

						default:
							break;
					}
				}
				break;

			case TOUCH_DEL_USER:
				if (GPIO_ReadInputDataBit(KEY7_GPIO, KEY7_GPIO_PIN) == 0) //ȷ�������ּ��
				{
					break;
				}

				{
					switch (SysTask.TouchSub)
					{
						case TOUCH_SUB_INIT: // *
							if (SysTask.nWaitTime == 0)
							{
								PS_ValidTempleteNum(&u16FingerCnt); //����ָ�Ƹ���

								if (u16FingerCnt == 0)
								{
									SysTask.nShowTime	= REMOTE_SHOW_TIME; //5��ʱ�������ʾ
									SysTask.nWaitTime	= 2000; //��ʱһ��ʱ���˳�
									SysTask.TouchState	= TOUCH_MANAGE_DISPLAY;
									OLED_Clear();
									OLED_ShowString(0, 3, "Fingerprint template Null��", 12);
								}
								else 
								{
									SysTask.nShowTime	= REMOTE_SHOW_TIME; //5��ʱ�������ʾ
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

							if (GPIO_ReadInputDataBit(KEY5_GPIO, KEY5_GPIO_PIN) == 0) //��ʱ��ѡ���
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5��ʱ�������ʾ

								SysTask.TouchSub	= TOUCH_GETKEY_DEBOUNCE; //����,���ּ��
							}

							if (GPIO_ReadInputDataBit(KEY6_GPIO, KEY6_GPIO_PIN) == 0) //��HOME��
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5��ʱ�������ʾ
								SysTask.TouchState	= TOUCH_MANAGE_DISPLAY;
							}

							if (GPIO_ReadInputDataBit(KEY7_GPIO, KEY7_GPIO_PIN) == 0) //ȷ����
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5��ʱ�������ʾ
								SysTask.TouchSub	= TOUCH_SUB_WAIT;
								u16FingerID 		= u8SelectPosi;
								SysTask.nWaitTime	= 1000;
								OLED_Clear();
								OLED_ShowString(0, 3, "Sure Del ?", 12);
								OLED_ShowString(0, 5, "ID : ", 12);
								OLED_ShowChar(48, 5, '0' + u16FingerID, 12);
							}

							break;

						case TOUCH_GETKEY_DEBOUNCE: //����,���ּ��
							if (SysTask.nFingerSubWaitT != 0)
							{
								break;
							}

							if ((GPIO_ReadInputDataBit(KEY5_GPIO, KEY5_GPIO_PIN) == 0)) //��ʱ��ѡ���
							{
								SysTask.nFingerSubWaitT = 20; //20ms���һ��
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

						case TOUCH_SUB_ENTER: //ѡ��
							if (GPIO_ReadInputDataBit(KEY6_GPIO, KEY6_GPIO_PIN) == 0) //��HOME��
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5��ʱ�������ʾ
								SysTask.TouchSub	= TOUCH_GETKEY;
							}

							if (GPIO_ReadInputDataBit(KEY7_GPIO, KEY7_GPIO_PIN) == 0) //ȷ����
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5��ʱ�������ʾ
								SysTask.TouchSub	= TOUCH_SUB_AGAIN;
							}

							break;

						case TOUCH_SUB_AGAIN:
							if (GPIO_ReadInputDataBit(KEY7_GPIO, KEY7_GPIO_PIN) != 0) //ȷ�������ּ��
							{
								ensure				= PS_DeletChar(u16FingerID, 1);

								if (ensure == 0)
								{

									OLED_Clear();
									SysTask.nShowTime	= REMOTE_SHOW_TIME; //5��ʱ�������ʾ
									SysTask.TouchSub	= TOUCH_GETKEY;
									SysTask.nWaitTime	= 1000; //��ʱһ��ʱ���˳�
									OLED_ShowString(0, 3, "Del User Success", 16);

								}
								else 
								{
									OLED_Clear();
									SysTask.nShowTime	= REMOTE_SHOW_TIME; //5��ʱ�������ʾ
									SysTask.TouchSub	= TOUCH_GETKEY;
									SysTask.nWaitTime	= 1000; //��ʱһ��ʱ���˳�
									OLED_ShowString(0, 3, "Del User Failed", 16);

								}
							}

							break;

						case TOUCH_SUB_WAIT: // *
							if ((GPIO_ReadInputDataBit(KEY7_GPIO, KEY7_GPIO_PIN) != 0)) //ȷ�������ּ��
							{
								SysTask.TouchSub	= TOUCH_SUB_ENTER;
							}

							if (GPIO_ReadInputDataBit(KEY6_GPIO, KEY6_GPIO_PIN) == 0) //��HOME��
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5��ʱ�������ʾ
								SysTask.TouchState	= TOUCH_MANAGE_DISPLAY;
							}

							break;

						default:
							break;
					}
				}
				break;

			case TOUCH_DISPLAY_ID:
				if ((SysTask.nWaitTime == 0) && (GPIO_ReadInputDataBit(KEY5_GPIO, KEY5_GPIO_PIN) != 0) //��ʱ��ѡ?
		??�� ���ּ�⣬��Ȼһֱ��
				&& (GPIO_ReadInputDataBit(KEY6_GPIO, KEY6_GPIO_PIN) != 0) //��HOME��
				&& (GPIO_ReadInputDataBit(KEY7_GPIO, KEY7_GPIO_PIN) != 0)) //ȷ����
				{
					SysTask.nShowTime	= REMOTE_SHOW_TIME; //5��ʱ�������ʾ
					SysTask.nWaitTime	= 20;		//2s��ʾIDʱ��
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
							if (GPIO_ReadInputDataBit(KEY6_GPIO, KEY6_GPIO_PIN) == 0) //��HOME��
							{
			
			SysTask.nShowTime = 0;

				OledInitTask();
							}
							if (GPIO_ReadInputDataBit(KEY1_GPIO, KEY1_GPIO_PIN) == 0) //��������
							{
								SysTask.nShowTime	= MOTO_ZERO_DETECT; //5��ʱ�������ʾ
								SysTask.TouchSub	= TOUCH_SUB_WAIT; //������
								SysTask.nWaitTime	= DEBOUNCE_TIME;

								//SysTask.AClockState = ~(SysTask.AClockState) & 0x01;//ȡ��
								if ((SysTask.MotoAState == MOTO_STATE_STOP) ||
									 (SysTask.MotoAState == MOTO_STATE_IDLE))
								{
									SysTask.MotoAState	= MOTO_STATE_INIT;
									ShowLockMode();
									SysTask.AClockState = CLOCK_OFF; //����״̬

								}
								else 
								{
									SysTask.AClockState = CLOCK_ON; //����״̬
								}

							}

							if (GPIO_ReadInputDataBit(KEY2_GPIO, KEY2_GPIO_PIN) == 0) //��������
							{
								SysTask.nShowTime	= MOTO_ZERO_DETECT; //5��ʱ�������ʾ
								SysTask.TouchSub	= TOUCH_SUB_WAIT; //������
								SysTask.nWaitTime	= DEBOUNCE_TIME;

								if ((SysTask.MotoBState == MOTO_STATE_STOP) ||
									 (SysTask.MotoBState == MOTO_STATE_IDLE))
								{
									SysTask.MotoBState	= MOTO_STATE_INIT;
									ShowLockMode();
									SysTask.BClockState = CLOCK_OFF; //����״̬

								}
								else 
								{
									SysTask.BClockState = CLOCK_ON; //����״̬
								}
							}

							if (GPIO_ReadInputDataBit(KEY3_GPIO, KEY3_GPIO_PIN) == 0) //��������
							{
								SysTask.nShowTime	= MOTO_ZERO_DETECT; //5��ʱ�������ʾ
								SysTask.TouchSub	= TOUCH_SUB_WAIT; //������
								SysTask.nWaitTime	= DEBOUNCE_TIME;

								if ((SysTask.MotoCState == MOTO_STATE_STOP) ||
									 (SysTask.MotoCState == MOTO_STATE_IDLE))
								{
									SysTask.MotoCState	= MOTO_STATE_INIT;
									ShowLockMode();
									SysTask.CClockState = CLOCK_OFF; //����״̬

								}
								else 
								{
									SysTask.CClockState = CLOCK_ON; //����״̬
								}
							}

							if (GPIO_ReadInputDataBit(KEY4_GPIO, KEY4_GPIO_PIN) == 0) //�������� 
							{
								SysTask.nShowTime	= MOTO_ZERO_DETECT; //5��ʱ�������ʾ
								SysTask.TouchSub	= TOUCH_SUB_WAIT; //������
								SysTask.nWaitTime	= DEBOUNCE_TIME;

								if ((SysTask.MotoDState == MOTO_STATE_STOP) ||
									 (SysTask.MotoDState == MOTO_STATE_IDLE))
								{
									SysTask.MotoDState	= MOTO_STATE_INIT;
									ShowLockMode();
									SysTask.DClockState = CLOCK_OFF; //����״̬

								}
								else 
								{
									SysTask.DClockState = CLOCK_ON; //����״̬
								}
							}

							// *******************************************************************
							// *	 PAUSE		 home		  RATION
							// *	 ʱ��ѡ��		 ���� 			����תѡ��  
							// *
							// *
							
							//���ܼ�����
							if (GPIO_ReadInputDataBit(KEY5_GPIO, KEY5_GPIO_PIN) == 0) //��ʱ��ѡ���
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5��ʱ�������ʾ
								SysTask.TouchSub	= TOUCH_SUB_TIMER_S;
								u8MotoSelect		= 0;

								//								u8FunSelect 		= 0;
								OLED_Clear();
								OLED_ShowString(0, 0, "Choose which one", 12);
								OLED_DrawBMP(0, 3, 128, 7, BMP_FourClock); //��ʾ������

							}

							if (GPIO_ReadInputDataBit(KEY6_GPIO, KEY6_GPIO_PIN) == 0) //��HOME��
							{
							}

							if (GPIO_ReadInputDataBit(KEY7_GPIO, KEY7_GPIO_PIN) == 0) //����תѡ��
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5��ʱ�������ʾ
								SysTask.TouchSub	= TOUCH_SUB_FR_S;
								u8MotoSelect		= 0;

								//								u8FunSelect 		= 0;
								OLED_Clear();
								OLED_ShowString(0, 0, "Choose which one", 12);
								OLED_DrawBMP(0, 3, 128, 7, BMP_FourClock); //��ʾ������
							}

							break;

						case TOUCH_SUB_TIMER_S: //ʱ�书��ѡ��
							if (GPIO_ReadInputDataBit(KEY1_GPIO, KEY1_GPIO_PIN) == 0) //��������
							{
								u8MotoSelect		= 1;
							}

							if (GPIO_ReadInputDataBit(KEY2_GPIO, KEY2_GPIO_PIN) == 0) //��������
							{
								u8MotoSelect		= 2;
							}

							if (GPIO_ReadInputDataBit(KEY3_GPIO, KEY3_GPIO_PIN) == 0) //��������
							{
								u8MotoSelect		= 3;
							}

							if (GPIO_ReadInputDataBit(KEY4_GPIO, KEY4_GPIO_PIN) == 0) //�������� 
							{
								u8MotoSelect		= 4;
							}

							if (u8MotoSelect != 0) //�а�������
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5��ʱ�������ʾ
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

							if (GPIO_ReadInputDataBit(KEY6_GPIO, KEY6_GPIO_PIN) == 0) //��HOME��
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5��ʱ�������ʾ
								SysTask.TouchState	= TOUCH_DISPLAY_ID;
							}

							break;

						case TOUCH_SUB_FR_S: //����תѡ��
							if (GPIO_ReadInputDataBit(KEY1_GPIO, KEY1_GPIO_PIN) == 0) //��������
							{
								SysTask.nShowTime	= MOTO_ZERO_DETECT; //5��ʱ�������ʾ
								u8MotoSelect		= 1;
								SysTask.TouchSub	= TOUCH_SUB_CH_DI;
							}

							if (GPIO_ReadInputDataBit(KEY2_GPIO, KEY2_GPIO_PIN) == 0) //��������
							{
								SysTask.nShowTime	= MOTO_ZERO_DETECT; //5��ʱ�������ʾ
								u8MotoSelect		= 2;
								SysTask.TouchSub	= TOUCH_SUB_CH_DI;
							}

							if (GPIO_ReadInputDataBit(KEY3_GPIO, KEY3_GPIO_PIN) == 0) //��������
							{
								SysTask.nShowTime	= MOTO_ZERO_DETECT; //5��ʱ�������ʾ
								u8MotoSelect		= 3;
								SysTask.TouchSub	= TOUCH_SUB_CH_DI;
							}

							if (GPIO_ReadInputDataBit(KEY4_GPIO, KEY4_GPIO_PIN) == 0) //�������� 
							{
								SysTask.nShowTime	= MOTO_ZERO_DETECT; //5��ʱ�������ʾ
								u8MotoSelect		= 4;
								SysTask.TouchSub	= TOUCH_SUB_CH_DI;
							}

							if (u8MotoSelect != 0) //�а�������
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5��ʱ�������ʾ
								SysTask.nFingerSubWaitT = 0;
								u8SelectPosi		= 0;
								OLED_Clear();
								OLED_ShowChar(0, 0, 'A' + u8MotoSelect - 1, 16);
								OLED_DrawHorizontal(0, 16, 2, 4, 1);

								OLED_ShowString(48, 1, "->FWD", 12);
								OLED_ShowString(48, 3, "REV", 12);
								OLED_ShowString(48, 5, "F-R", 12);
							}

							if (GPIO_ReadInputDataBit(KEY6_GPIO, KEY6_GPIO_PIN) == 0) //��HOME��
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5��ʱ�������ʾ
								SysTask.TouchState	= TOUCH_DISPLAY_ID;
							}

							break;

						case TOUCH_SUB_CH_TI: //�ı�ʱ��
							if (GPIO_ReadInputDataBit(KEY5_GPIO, KEY5_GPIO_PIN) == 0) //��ʱ��ѡ���
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5��ʱ�������ʾ
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

							if (GPIO_ReadInputDataBit(KEY6_GPIO, KEY6_GPIO_PIN) == 0) //��HOME��
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5��ʱ�������ʾ
								SysTask.TouchState	= TOUCH_DISPLAY_ID;
							}

							if (GPIO_ReadInputDataBit(KEY7_GPIO, KEY7_GPIO_PIN) == 0) //ȷ����
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5��ʱ�������ʾ
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

						case TOUCH_SUB_CH_TI_DE: //�ı�ʱ�䣬����,���ּ��
							if (SysTask.nFingerSubWaitT != 0)
							{
								break;
							}

							if ((GPIO_ReadInputDataBit(KEY5_GPIO, KEY5_GPIO_PIN) == 0) //��ʱ��ѡ���
							|| (GPIO_ReadInputDataBit(KEY6_GPIO, KEY6_GPIO_PIN) == 0) //��HOME��
							|| (GPIO_ReadInputDataBit(KEY7_GPIO, KEY7_GPIO_PIN) == 0)) //ȷ����
							{
								SysTask.nFingerSubWaitT = 20; //20ms���һ��
							}
							else 
							{
								SysTask.TouchSub	= TOUCH_SUB_CH_TI;
							}

							break;

						case TOUCH_SUB_CH_DI: //�ı䷽��
							if (SysTask.nFingerSubWaitT != 0)
							{
								break;
							}

							if (GPIO_ReadInputDataBit(KEY5_GPIO, KEY5_GPIO_PIN) == 0) //��ʱ��ѡ���
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5��ʱ�������ʾ
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

							if (GPIO_ReadInputDataBit(KEY6_GPIO, KEY6_GPIO_PIN) == 0) //��HOME��
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5��ʱ�������ʾ
								SysTask.TouchState	= TOUCH_DISPLAY_ID;
							}

							if (GPIO_ReadInputDataBit(KEY7_GPIO, KEY7_GPIO_PIN) == 0) //ȷ����
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
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5��ʱ�������ʾ
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

						case TOUCH_SUB_CH_DI_DE: //�ı䷽�� debounce
							if (SysTask.nFingerSubWaitT != 0)
							{
								break;
							}

							if ((GPIO_ReadInputDataBit(KEY5_GPIO, KEY5_GPIO_PIN) == 0) //��ʱ��ѡ���
							|| (GPIO_ReadInputDataBit(KEY6_GPIO, KEY6_GPIO_PIN) == 0) //��HOME��
							|| (GPIO_ReadInputDataBit(KEY7_GPIO, KEY7_GPIO_PIN) == 0)) //ȷ����
							{
								SysTask.nFingerSubWaitT = 20; //20ms���һ��
							}
							else 
							{
								SysTask.TouchSub	= TOUCH_SUB_CH_DI;
							}

							break;

						case TOUCH_SUB_WAIT: //����
							if (SysTask.nWaitTime != 0)
								break;

							if ((GPIO_ReadInputDataBit(KEY1_GPIO, KEY1_GPIO_PIN) != 0) &&
								 (GPIO_ReadInputDataBit(KEY2_GPIO, KEY2_GPIO_PIN) != 0) &&
								 (GPIO_ReadInputDataBit(KEY3_GPIO, KEY3_GPIO_PIN) != 0) &&
								 (GPIO_ReadInputDataBit(KEY4_GPIO, KEY4_GPIO_PIN) != 0))
							{
								SysTask.TouchSub	= TOUCH_SUB_INIT;
								SysTask.nWaitTime	= DEBOUNCE_TIME; //��ֹһ��ʼ�ͼ�⵽0��
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
* ����: 
* ����: 
* �β�:		
* ����: ��
* ˵��: 
*******************************************************************************/
void LedTask(void)
{
	//LedTask();
}

/*******************************************************************************
* ����: 
* ����: У���
* �β�:		
* ����: ��
* ˵��: 
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
* ����: 
* ����: �ӻ�������
*		��ͷ + �ӻ���ַ +      �ֱ��ַ + ���� + ���ݰ�[3] + У��� + ��β
* �β�:		
* ����: ��
* ˵��: 
*******************************************************************************/
void SlavePackageSend(u8 u8SlaveAddr, u8 u8GlassAddr, u8 u8Cmd, u8 *u8Par)
{
	u8 i				= 0;
	u8 u8SendArr[9] 	=
	{
		0
	};

	u8SendArr[0]		= 0xAA; 					//��ͷ
	u8SendArr[1]		= u8SlaveAddr;					//�ӻ���ַ
	u8SendArr[2]		= u8GlassAddr;					//�ֱ��ַ
	u8SendArr[3]		= u8Cmd;					//����
	u8SendArr[4]		= u8Par[0]; 				//����1
	u8SendArr[5]		= u8Par[1]; 				//����2
	u8SendArr[6]		= u8Par[2]; 				//����3

    u8SendArr[7] = CheckSum(u8SendArr, 7);         //У���
	u8SendArr[8]		= 0x55; 					//��β


	for (i = 0; i < 8; i++)
	{
		USART_SendData(USART3, u8SendArr[i]);
	}
}

/*******************************************************************************
* ����: ƽ�巢�����������������
* ����: �ж��жϽ��յ�������û�и������ݰ� ���߾�����
* �β�:		
* ����: -1, ʧ�� 0:�ɹ�
* ˵��: 
*******************************************************************************/
static int Usart1JudgeStr(void)
{
    //    ��ͷ + �ӻ���ַ + �ֱ��ַ��+ ���� + ���ݰ�[3] * 8 + У��� + ��β
    //    ��ͷ��0XAA
    //    �ӻ���ַ��0XFF��ȫ��         0x01����һ���ӻ�
    //    �ֱ��ַ��0XFF��ȫ��         0x01����һ���ֱ�
    //    ��� CMD_UPDATE   �� CMD_READY
    //    ���ݣ��������� + ���� + ʱ�䣩
    //    У��ͣ�
    //    ��β�� 0X55

	const char *data;
	u8 i, j;
    u8 str[8] = {0}; 
	str[0]=0xAA;

    u8 u8CheckSum = 0;

	data=strstr((const char*)USART1_RX_BUF,(const char*)str);
	if(!data)
        return -1;

    if(*(data + 3) == CMD_READY)   //����Ǿ���������֪ͨ���������ϴ�������
    {

        if(     (*(data + 1) == 0xFF)   //�ӻ���ַ
            &&  (*(data + 2) >= 0xFF) //�ֱ��ַ
            &&  (*(data + 29) == 0X55)) //��β
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
    else if((*(data + 1) >= 0x01)  && (*(data + 1) <= 0x04))   //�ӻ���ַ
        if(((*(data + 2) >= 0x01)  && (*(data + 2) <= 0x08)) ||(*(data + 2) ==  0xFF)) //�ֱ��ַ
            if((*(data + 3) == CMD_UPDATE)) //�������ݰ�
                if((*(data + 29) == 0X55)) //��β
                {
                    for(i = 0; i < 27; i++)
                        u8CheckSum += *(data + i);

                    if(u8CheckSum == 0)
                    {
                        if(*(data + 2) ==  0xFF)   //�ôӻ������ȫ���ֱ�
                        {
                            for(j = 0 ; j < 8; j++)
                            {
                                SysTask.SlaveStateSave[*(data + 1) - 1].GlassState[j].u8LockState  = *(data + 4 + 3 * j);
                                SysTask.SlaveStateSave[*(data + 1) - 1].GlassState[j].u8Dir        = *(data + 5 + 3 * j);
                                SysTask.SlaveStateSave[*(data + 1) - 1].GlassState[j].MotoTime     = (unsigned char)(*(data + 6 + 3 * j));
                            }
                        }
                        else  //�����ֱ��״̬�ϴ�
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
   return -1;   //������
                
}


/*******************************************************************************
* ����: 
* ����: ƽ����������͵����������
* �β�:		
* ����: ��
* ˵��: 
*******************************************************************************/
void TabletToHostTask(void)
{
    if(USART3_RX_STA & 0X8000)//���յ��ӻ�����
    {
        Usart1JudgeStr();
    }
}


/*******************************************************************************
* ����: 
* ����: �ж��жϽ��յ�������û��Ӧ���, �ӻ���������������
* �β�:		
* ����: -1, ʧ�� 0:�ɹ�
* ˵��: 
*******************************************************************************/
static int Usart3JudgeStr(void)
{
    //    ��ͷ + �ӻ���ַ + �ֱ��ַ��+ ���� + ���ݰ�[3] * 8 + У��� + ��β
    //    ��ͷ��0XAA
    //    �ӻ���ַ��0XFF��ȫ��         0x01����һ���ӻ�
    //    �ֱ��ַ��0XFF��ȫ��         0x01����һ���ֱ�
    //    ���CMD_ACK
    //    ���ݣ��������� + ���� + ʱ�䣩
    //    У��ͣ�
    //    ��β�� 0X55

	const char *data;
	u8  i, j;
    u8 str[8] = {0};
	str[0]=0xAA;

    u8 u8CheckSum = 0;

	data=strstr((const char*)USART3_RX_BUF,(const char*)str);
	if(!data)
        return -1;

    if((*(data + 1) >= 0x01)  && (*(data + 1) <= 0x04))   //�ӻ���ַ
        if(((*(data + 2) >= 0x01)  && (*(data + 2) <= 0x08)) ||(*(data + 2) ==  0xFF)) //�ֱ��ַ
            if((*(data + 3) == CMD_ACK)) //Ӧ���
                if((*(data + 29) == 0X55)) //��β
                {
                    for(i = 0; i < 27; i++)
                        u8CheckSum += *(data + i);

                    if(u8CheckSum == 0)
                    {
                        if(*(data + 2) ==  0xFF)   //�ôӻ������ȫ���ֱ�
                        {
                            for(j = 0 ; j < 8; j++)
                            {
                                SysTask.SlaveState[*(data + 1) - 1].GlassState[j].u8LockState  = *(data + 4 + 3 * j);
                                SysTask.SlaveState[*(data + 1) - 1].GlassState[j].u8Dir        = *(data + 5 + 3 * j);
                                SysTask.SlaveState[*(data + 1) - 1].GlassState[j].MotoTime     = (int)*(data + 6 + 3 * j);
                            }
                        }
                        else  //�����ֱ��״̬�ϴ�
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
   return -1;   //������
                
}


/*******************************************************************************
* ����: 
* ����: ����м����ӻ�������ȡ�ӻ�״̬
* �β�:		
* ����: ��
* ˵��: 
*******************************************************************************/
void SlaveDetected(void)
{
	static u8 u8Addr			= 1;						//0xFF : �㲥��ַ�� 0x00 ��������ַ
    u8 u8aSendData[3]   = {0};
    
	switch (SysTask.SendSubState)
	{
		case SEND_SUB_INIT:
            SlavePackageSend(u8Addr, 0xFF, CMD_GET_ATTR, u8aSendData);
            SysTask.SendSubState = SEND_SUB_WAIT;
            SysTask.u16SubWaitTime = SLAVE_WAIT_TIME;
			break;
            
        case SEND_SUB_WAIT:
            if(SysTask.u16SubWaitTime == 0)  //��ʱ��Ӧ��
            {
                SysTask.u8SlaveCount = u8Addr - 1;
                g_SlaveSendAddr      = SLAVE_SEND_ALL | SysTask.u8SlaveCount;  
                g_GlassSendAddr      = GLASS_SEND_ALL; 
                SysTask.SendState    = SEND_TABLET_ALL; //����ȫ���ӻ�״̬��ƽ��
                SysTask.SendSubState = SEND_SUB_INIT;
            }
            else if(USART3_RX_STA & 0X8000)//���յ��ӻ�����
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
                        SysTask.SendState    = SEND_TABLET_ALL; //����ȫ���ӻ�״̬��ƽ��
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
* ����: 
* ����: ��������ȫ��״̬��ƽ��
* �β�:		
* ����: ��
* ˵��: 
*******************************************************************************/
void SendToTablet(u8 u8SlaveAddr, u8 u8GlassAddr)
{
    //    ��ͷ + ��ַ + ���� + ���ݰ�[96�� + У��� + ��β
    //    ��ͷ��0XAA
    //    �ӻ���ַ��0XFF��ȫ��         0x01����һ���ӻ�
    //    �ֱ��ַ��0XFF��ȫ��         0x01����һ���ֱ�
    //    ���
    //    ���ݣ��������� + ���� + ʱ�䣩* 4 * 8 = 96
    //    У��ͣ�
    //    ��β�� 0X55

    u8 i;
    u8 u8aSendArr[102] = {0};
    SendArrayUnion_u SendArrayUnion;

    memcpy(SendArrayUnion.SlaveState, SysTask.SlaveState, sizeof(SysTask.SlaveState));
    memcpy(SysTask.SlaveStateSave, SysTask.SlaveState, sizeof(SysTask.SlaveState));

    u8aSendArr[0] = 0xAA;
    u8aSendArr[1] = u8SlaveAddr;
    u8aSendArr[2] = u8GlassAddr;
    u8aSendArr[3] = 0x11;       //�������


    for(i = 0; i < 96; i++)
    {
        u8aSendArr[4 + i] = SendArrayUnion.u8SendArray[i];
    }

    u8aSendArr[100] = CheckSum(u8aSendArr, 100); //У���
    u8aSendArr[101] = 0x55;  //��βs 
}

/*******************************************************************************
* ����: 
* ����: ���ͳ�ʼ��   ��ȡȫ���ӻ�״̬
* �β�:		
* ����: ��
* ˵��: 
*******************************************************************************/
void SendInitTask(void)
{
	switch (SysTask.SendState)
	{
		case SEND_INIT:
            if(SysTask.u16BootTime == 0)   //�ȴ��ӻ�������ȫ
            {
                SlaveDetected();
            }
			break;
        
		case SEND_TABLET_ALL: //����ȫ���ӻ���ƽ��
		    if(SysTask.bTabReady == TRUE)
            
{
                g_SlaveSendAddr      = SLAVE_SEND_ALL | SysTask.u8SlaveCount; 
    			SendToTablet(g_SlaveSendAddr, g_GlassSendAddr);  //0x1? ���дӻ�  ������ӻ�����
    			SysTask.SendState = SEND_IDLE;
            }
			break;

        /* ���ڽ׶�û��ʹ���ⲿ�ֵ����ݣ���Ϊ����Ӧ����飬���������ⲿ����ʱ�ò���
		case SEND_TABLET_SINGLE://���͵����ӻ�״̬��ƽ��
			SendToTablet(g_SlaveSendAddr, g_GlassSendAddr);
			break;
        
        
		case SEND_SLAVE_ALL: //��������Ⱥ��������ӻ�
            g_SlaveSendAddr      = SLAVE_SEND_ALL | SysTask.u8SlaveCount; 
			SendToTablet(g_SlaveSendAddr, g_GlassSendAddr);  //0x1? ���дӻ�  ������ӻ�����
			break;
        
		case SEND_SLAVE_SINGLE://�������͵����ӻ���������
			SendToTablet(g_SlaveSendAddr, g_GlassSendAddr);
			break;
        */
		case SEND_IDLE:
		default:
			break;
	}
}


/*******************************************************************************
* ����: 
* ����: �����ӷ��͸��ӻ����͵����������
* �β�:		
* ����: ��
* ˵��: 
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
            SysTask.u16HtoSWaitTime = 50; //��ʱ50ms
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
* ����: 
* ����: 
* �β�:		
* ����: ��
* ˵��: 
*******************************************************************************/
void MainTask(void)
{
	LedTask();
	SendInitTask();								//���ͳ�ʼ��
	TabletToHostTask(); 							//����ƽ������������
	HostToTabletTask(); 					//�������͸��ӻ�������������

	//SlaveToHostTask();   //�ӻ����͵������˵Ľ���������,  ��Ϊ����Ӧ��У�飬�����ⲿ�������ò�����
	                        //ֻ����detect ���������·�ʱ�ᷢ�����ݸ�����
	FingerTouchTask();								//ָ��ģ��
}


/*******************************************************************************
* ����: 
* ����: 
* �β�:		
* ����: ��
* ˵��: 
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
    SysTask.u16BootTime = 3000;   //3��ȴ��ӻ�������ȫ

}


