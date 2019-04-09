

#ifndef __SYSCOMMENT_H__
#define __SYSCOMMENT_H__

#ifdef _MAININC_
#define EXTERN

#else

#define EXTERN					extern
#endif


/* ͷ�ļ�  ------------------------------------------------------------*/
#include "stm32f10x.h"



/*
*   �������ڹ�ռ��40k��û���Ż���
*   ��60k��ַ����������洢�� ���ݴ洢λ�ã���Ϊ���ݴ洢��������������д�������������ʱ����flash�Ѿ���
*   ������һ��flash���洢���� 61k  ��62k��63k ���ؿ���3W�� 
*   ���Լ���ָ��ģ��flash 16ҳ 
*/

/* PASSWD [0] [1] [2] [3] [4] [5]   [���ݴ洢��ַƫ��]*/

#define PASSWD_ADDR				0x0800F000		

#define DATA_ADDR				0x0800F400		

#define FLASH_READ_CNT    7
#define DATA_READ_CNT    8

#define LOCK_OFFTIME    4000




#define VOID					void	




#define VOID					void	


//�������������ݴ�С������--���б�������ʹ���ֽ�����
//�豸��Ϣ
#define LED_SHOWFRE_DNS 		300
#define LED_SHOWFRE_OK			1000


#define FINGER_MAX_CNT			10   //ָ��ģ�����洢����





#define PWM_RCC_CLOCKCMD             RCC_APB2PeriphClockCmd
#define PWM_RCC_CLOCKGPIO            RCC_APB2Periph_GPIOB

#define PWM_A_PIN_P                 GPIO_Pin_0  //PWM+
#define PWM_A_PIN_N                 GPIO_Pin_1  //PWM-

#define PWM_B_PIN_P                 GPIO_Pin_10
#define PWM_B_PIN_N                 GPIO_Pin_11

#define PWM_C_PIN_P                 GPIO_Pin_12
#define PWM_C_PIN_N                 GPIO_Pin_13

#define PWM_D_PIN_P                 GPIO_Pin_14
#define PWM_D_PIN_N                 GPIO_Pin_15

#define PWM_GPIO                  GPIOB


#define PASSWD_COUNT			0x6   //����λ�� 6λ
#define REMOTE_SHOW_TIME		20   //���յ�ң�� led��ʾʱ�䣬��ʱ���˳�
#define MOTO_ZERO_DETECT		20   //����⣬�ʱ�䣬��ⲻ��ֱ���˳� ,ʵ��ȫ��һȦΪ10S����

//    static u16 u16WriteCount = 0;  //��д����
//    static u16 u16EraseCount = 0;  //�ز�д����
//                FLASH_ReadMoreData(FLASH_SAVE_ADDR, readData, 8);
//                FLASH_WriteMoreData(FLASH_SAVE_ADDR, writeData, 8);
//                FLASH_ReadMoreData(FLASH_SAVE_ADDR, readData, 8);             





typedef enum 
{
    FALSE = 0, TRUE = !FALSE
} bool;


typedef enum 
{
    WRITE_PASSWD = 0, 
    MANAGE_MAIN, //����������
    MANAGE_CHOOSE, //ѡ��������
    MANAGE_MODE, //ģʽ����
    MANAGE_FINGER, //ָ�ƹ������
    MANAGE_PASSWD, //����������
    MANAGE_ADDUSR, //����ָ�ƹ������
    MANAGE_DELUSR, //ɾ��ָ�ƹ������
    MANAGE_CHANGE, //���Ĺ���Ա����
    SAN_DEF = 0XFF, 
} RemoteState_T;


typedef enum 
{
    INIT = 0, 
    ENTER,
    AGAIN,
    WAIT,
    GETKEY,
	SAN_ERR,
} RemoteSub_T;
    
typedef enum 
{
   TOUCH_INIT = 0, 
   TOUCH_CHECK,
   TOUCH_DISPLAY_ID,
   TOUCH_DISPLAY_ID_DEBOU,  //��������
   TOUCH_KEY_CHECK,  //��ⰴ������
   TOUCH_MANAGE_DISPLAY, //�����û�ָ�ƹ���
   TOUCH_MANAGE, //�����û�ָ�ƹ���
   TOUCH_MANAGE_CHOOSE, //�����û�ָ�ƹ���
   TOUCH_ADD_USER, //�����û�ָ�ƹ���
   TOUCH_DEL_USER, //�����û�ָ�ƹ���
   TOUCH_WAIT,
   TOUCH_DEF = 0XFF, 
} TouchState_T;


typedef enum 
{
    TOUCH_SUB_INIT = 0, 
    TOUCH_SUB_TIMER_S,  //ʱ�书��ѡ�� 
    TOUCH_SUB_FR_S,  //����תѡ��
    TOUCH_SUB_CH_TI,  //�ı�ʱ��
    TOUCH_SUB_CH_DI,  //�ı䷽��
    TOUCH_SUB_CH_TI_DE,  //�ı�ʱ��DEBOUNCE ����
    TOUCH_SUB_CH_DI_DE,  //�ı䷽��DEBOUNCE ����
    TOUCH_SUB_ENTER,  //
    TOUCH_SUB_AGAIN,  //
    TOUCH_GETKEY,
    TOUCH_GETKEY_DEBOUNCE, //���ּ��
    TOUCH_SUB_WAIT, 
    TOUCH_SUB_DEF = 0XFF,
} TouchSub_T;    



typedef enum 
{
    MOTO_TIME_TPD = 0, //ÿ��Ķ���ģʽ������12Сʱ��ֹͣ12Сʱ
    MOTO_TIME_650, //��ת2���ӣ�ֹͣ942S
    MOTO_TIME_750,  //��ת2���ӣ�ֹͣ800S
    MOTO_TIME_850,  //��ת2���ӣ�ֹͣ693S
    MOTO_TIME_1000, //��ת2���ӣ�ֹͣ570S
    MOTO_TIME_1950, //��ת2���ӣ�ֹͣ234S

    
    MOTO_TIME_OFF = 0XFE,  //����״̬�£����ֹͣ

    MOTO_TIME_DEF = 0XFF,
}MotoTime_e;  
    

typedef enum 
{
    MOTO_FR_FWD = 0,  //��ת
    MOTO_FR_REV, //��ת
    MOTO_FR_FWD_REV, //����ת
    MOTO_FR_STOP, //ֹͣ
    MOTO_FR_DEF = 0XFF,
}MotoFR;  

typedef enum 
{
    SEND_INIT = 0,     // ��ʼ״̬Ϊ��ѯ�����µĸ��ӻ�״̬   �ӻ���״ֻ̬�����ڸ����ӻ���
    SEND_IDLE,         //����״̬
    SEND_TABLET_ALL,         //����ȫ���ӻ���ƽ��
    SEND_TABLET_SINGLE,      //���͵����ӻ���ƽ��
    
    SEND_SLAVE_ALL,    //��������Ⱥ��������ӻ�
    SEND_SLAVE_SINGLE, //�������͵����ӻ���������
    SEND_WAIT,         //�ȴ�״̬ ���ȴ��ӻ�Ӧ��
    SEND_DEF = 0XFF,
}SendState_T;  


typedef enum 
{
    SEND_SUB_INIT = 0,     // ��ʼ״̬Ϊ��ѯ�����µĸ��ӻ�״̬   �ӻ���״ֻ̬�����ڸ����ӻ���
    SEND_SUB_IDLE,         //����״̬
    SEND_SUB_WAIT,         //�ȴ�״̬ ���ȴ��ӻ�Ӧ��
    SEND_SUB_DEF = 0XFF,
}SendSubState_T; 

typedef enum 
{
    HtoS_CHECK = 0,     // host to slave init
    HtoS_SEND,         //����״̬
    HtoS_WAIT,         //�ȴ�״̬ ���ȴ�һ��ʱ�䣬�ٷ��ͣ���ʱ��ӻ���Ӧ
    HtoS_DEF = 0XFF,
}HtoS_State_T; 
    

typedef enum 
{
    CMD_GET_ATTR = 0,     // ��ȡ����
    CMD_ACK,     // Ӧ��
    CMD_UPDATE,     // ���°�
    CMD_READY,     // ������
    CMD_DEF = 0XFF, 
}SendCmd_T; 

typedef struct GLASSES_STATE
{
    u8 u8LockState; //��״̬,�����ϴ�����Ϊ������һ��ʱ�����Զ��رգ���ƽ��˴���Ϳ���
    u8 u8Dir; //ת������
    MotoTime_e MotoTime; //ת��ʱ��   
}GlassState_t;  //�ֱ�״̬

typedef struct SLAVESTATE
{
    GlassState_t GlassState[8];
}SlaveState_t;  //�ӻ�״̬

typedef union SENDARRAY
{
    u8 u8SendArray[96];
    SlaveState_t SlaveState[4];
}SendArrayUnion_u; //��������������

typedef struct 
{
    bool			mUpdate;

    vu16			nTick;								//������
    vu16			nLoadTime;							//��ʾ����ʱ��

    vu8 			nShowTime;							//oled �л���ʾʱ�� ��λS
    vu32 			nWaitTime;							//
    vu32 			nSubWaitTime;						//��״̬�ȴ���ʱ
    vu32 			nFingerSubWaitT;						//��״̬�ȴ���ʱ  
    TouchState_T	TouchState;
    TouchSub_T	    TouchSub;

    SendState_T     SendState;
    SendSubState_T  SendSubState;
    vu16			u16SubWaitTime;							//�ȴ��ӻ�Ӧ��ʱ��
    u8              u8SlaveCount;
    vu16			u16BootTime;							//�ȴ��ӻ�����ʱ��
    vu16			u16HtoSWaitTime;							//��ʱһ��ʱ��
    
    SlaveState_t    SlaveState[4];  //���4 ���ӻ�����   4 * 8 = 32���ֱ�
    SlaveState_t    SlaveStateSave[4];  //������Ҫ���µ���״̬


    
    bool 			bTabReady;							//ƽ���ϵ�׼������
} SYS_TASK;


/* ȫ�ֱ��� -----------------------------------------------------------*/

/* ȫ�ֱ��� -----------------------------------------------------------*/
EXTERN vu8		mSysIWDGDog; //�����
EXTERN vu32 	mSysSoftDog; //�������� 
EXTERN vu16 	mSysTick; //������
EXTERN vu16 	mSysSec; //������
EXTERN vu16 	mTimeRFRX; //���ռ��-����

EXTERN SYS_TASK SysTask;


EXTERN void Sys_DelayMS(uint16_t nms);
EXTERN void Sys_GetMac(u8 * mac);
EXTERN void Sys_LayerInit(void);
EXTERN void Sys_IWDGConfig(u16 time);
EXTERN void Sys_IWDGReloadCounter(void);
EXTERN void Sys_1s_Tick(void);

EXTERN void DelayUs(uint16_t nCount);
EXTERN void DelayMs(uint16_t nCount);
EXTERN void Strcpy(u8 * str1, u8 * str2, u8 len);
EXTERN bool Strcmp(u8 * str1, u8 * str2, u8 len);

EXTERN void MainTask(void);
EXTERN void SysInit(void);


#endif

