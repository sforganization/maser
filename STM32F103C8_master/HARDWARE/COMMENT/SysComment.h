

#ifndef __SYSCOMMENT_H__
#define __SYSCOMMENT_H__

#ifdef _MAININC_
#define EXTERN

#else

#define EXTERN					extern
#endif


/* 头文件  ------------------------------------------------------------*/
#include "stm32f10x.h"



/*
*   程序现在共占用40k，没有优化，
*   用60k地址部分做密码存储及 数据存储位置，因为数据存储经常擦除，所以写入与读出不符合时表明flash已经坏
*   跳到另一个flash区存储数据 61k  。62k。63k 保守可以3W次 
*   可以加入指纹模块flash 16页 
*/

/* PASSWD [0] [1] [2] [3] [4] [5]   [数据存储地址偏移]*/

#define PASSWD_ADDR				0x0800F000		

#define DATA_ADDR				0x0800F400		

#define FLASH_READ_CNT    7
#define DATA_READ_CNT    8

#define LOCK_OFFTIME    4000




#define VOID					void	




#define VOID					void	


//联合体由于数据大小端问题--所有变量必须使用字节声明
//设备信息
#define LED_SHOWFRE_DNS 		300
#define LED_SHOWFRE_OK			1000


#define FINGER_MAX_CNT			10   //指纹模块最多存储个数





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


#define PASSWD_COUNT			0x6   //密码位数 6位
#define REMOTE_SHOW_TIME		20   //接收到遥控 led显示时间，超时则退出
#define MOTO_ZERO_DETECT		20   //零点检测，最长时间，检测不到直接退出 ,实测全速一圈为10S左右

//    static u16 u16WriteCount = 0;  //重写次数
//    static u16 u16EraseCount = 0;  //重擦写次数
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
    MANAGE_MAIN, //管理主界面
    MANAGE_CHOOSE, //选择电机界面
    MANAGE_MODE, //模式界面
    MANAGE_FINGER, //指纹管理界面
    MANAGE_PASSWD, //密码管理界面
    MANAGE_ADDUSR, //增加指纹管理界面
    MANAGE_DELUSR, //删除指纹管理界面
    MANAGE_CHANGE, //更改管理员界面
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
   TOUCH_DISPLAY_ID_DEBOU,  //消抖过后
   TOUCH_KEY_CHECK,  //检测按键输入
   TOUCH_MANAGE_DISPLAY, //超级用户指纹管理
   TOUCH_MANAGE, //超级用户指纹管理
   TOUCH_MANAGE_CHOOSE, //超级用户指纹管理
   TOUCH_ADD_USER, //超级用户指纹管理
   TOUCH_DEL_USER, //超级用户指纹管理
   TOUCH_WAIT,
   TOUCH_DEF = 0XFF, 
} TouchState_T;


typedef enum 
{
    TOUCH_SUB_INIT = 0, 
    TOUCH_SUB_TIMER_S,  //时间功能选择 
    TOUCH_SUB_FR_S,  //正反转选择
    TOUCH_SUB_CH_TI,  //改变时间
    TOUCH_SUB_CH_DI,  //改变方向
    TOUCH_SUB_CH_TI_DE,  //改变时间DEBOUNCE 防抖
    TOUCH_SUB_CH_DI_DE,  //改变方向DEBOUNCE 防抖
    TOUCH_SUB_ENTER,  //
    TOUCH_SUB_AGAIN,  //
    TOUCH_GETKEY,
    TOUCH_GETKEY_DEBOUNCE, //松手检测
    TOUCH_SUB_WAIT, 
    TOUCH_SUB_DEF = 0XFF,
} TouchSub_T;    



typedef enum 
{
    MOTO_TIME_TPD = 0, //每天的动作模式，工作12小时，停止12小时
    MOTO_TIME_650, //旋转2分钟，停止942S
    MOTO_TIME_750,  //旋转2分钟，停止800S
    MOTO_TIME_850,  //旋转2分钟，停止693S
    MOTO_TIME_1000, //旋转2分钟，停止570S
    MOTO_TIME_1950, //旋转2分钟，停止234S

    
    MOTO_TIME_OFF = 0XFE,  //开锁状态下，电机停止

    MOTO_TIME_DEF = 0XFF,
}MotoTime_e;  
    

typedef enum 
{
    MOTO_FR_FWD = 0,  //正转
    MOTO_FR_REV, //反转
    MOTO_FR_FWD_REV, //正反转
    MOTO_FR_STOP, //停止
    MOTO_FR_DEF = 0XFF,
}MotoFR;  

typedef enum 
{
    SEND_INIT = 0,     // 初始状态为查询主机下的各从机状态   从机的状态只保存在各个从机中
    SEND_IDLE,         //空闲状态
    SEND_TABLET_ALL,         //发送全部从机给平板
    SEND_TABLET_SINGLE,      //发送单个从机给平板
    
    SEND_SLAVE_ALL,    //主机发送群控命令给从机
    SEND_SLAVE_SINGLE, //主机发送单个从机控制命令
    SEND_WAIT,         //等待状态 ，等待从机应答
    SEND_DEF = 0XFF,
}SendState_T;  


typedef enum 
{
    SEND_SUB_INIT = 0,     // 初始状态为查询主机下的各从机状态   从机的状态只保存在各个从机中
    SEND_SUB_IDLE,         //空闲状态
    SEND_SUB_WAIT,         //等待状态 ，等待从机应答
    SEND_SUB_DEF = 0XFF,
}SendSubState_T; 

typedef enum 
{
    HtoS_CHECK = 0,     // host to slave init
    HtoS_SEND,         //发送状态
    HtoS_WAIT,         //等待状态 ，等待一段时间，再发送，给时间从机响应
    HtoS_DEF = 0XFF,
}HtoS_State_T; 
    

typedef enum 
{
    CMD_GET_ATTR = 0,     // 获取属性
    CMD_ACK,     // 应答
    CMD_UPDATE,     // 更新包
    CMD_READY,     // 就绪包
    CMD_DEF = 0XFF, 
}SendCmd_T; 

typedef struct GLASSES_STATE
{
    u8 u8LockState; //锁状态,不用上传，因为开锁后一段时间是自动关闭，在平板端处理就可以
    u8 u8Dir; //转动方向
    MotoTime_e MotoTime; //转动时间   
}GlassState_t;  //手表状态

typedef struct SLAVESTATE
{
    GlassState_t GlassState[8];
}SlaveState_t;  //从机状态

typedef union SENDARRAY
{
    u8 u8SendArray[96];
    SlaveState_t SlaveState[4];
}SendArrayUnion_u; //发送数组联合体

typedef struct 
{
    bool			mUpdate;

    vu16			nTick;								//节拍器
    vu16			nLoadTime;							//显示加载时间

    vu8 			nShowTime;							//oled 切换显示时间 单位S
    vu32 			nWaitTime;							//
    vu32 			nSubWaitTime;						//子状态等待延时
    vu32 			nFingerSubWaitT;						//子状态等待延时  
    TouchState_T	TouchState;
    TouchSub_T	    TouchSub;

    SendState_T     SendState;
    SendSubState_T  SendSubState;
    vu16			u16SubWaitTime;							//等待从机应答时间
    u8              u8SlaveCount;
    vu16			u16BootTime;							//等待从机启动时间
    vu16			u16HtoSWaitTime;							//延时一段时间
    
    SlaveState_t    SlaveState[4];  //最大4 个从机接入   4 * 8 = 32个手表
    SlaveState_t    SlaveStateSave[4];  //保存需要更新到的状态


    
    bool 			bTabReady;							//平板上电准备就绪
} SYS_TASK;


/* 全局变量 -----------------------------------------------------------*/

/* 全局变量 -----------------------------------------------------------*/
EXTERN vu8		mSysIWDGDog; //软狗标记
EXTERN vu32 	mSysSoftDog; //软狗计数器 
EXTERN vu16 	mSysTick; //节拍器
EXTERN vu16 	mSysSec; //节拍器
EXTERN vu16 	mTimeRFRX; //接收间隔-仿真

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

