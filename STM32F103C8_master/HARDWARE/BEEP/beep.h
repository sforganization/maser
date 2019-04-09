#ifndef __BEEP_H__
#define __BEEP_H__

/* ����ͷ�ļ� ----------------------------------------------------------------*/
#include <stm32f10x.h>

/* ���Ͷ��� ------------------------------------------------------------------*/
typedef enum
{
  BEEPState_OFF = 0,
  BEEPState_ON,
}BEEPState_TypeDef;
#define IS_BEEP_STATE(STATE)           (((STATE) == BEEPState_OFF) || ((STATE) == BEEPState_ON))

/* �궨�� --------------------------------------------------------------------*/
/*
 * ���º궨�����ݸ�������Ӳ��ϢϢ��أ���Ҫ�鿴�������·ԭ��ͼ������ȷ��д��
 * ���磬��ԭ��ͼ���ڷ���������stm32f103оƬ��PB0�����ϣ������йط��������ŵ�
 * �궨�嶼����GPIOB��GPIO_Pin_0��صģ�����ר�Ű���Щ�뿪����Ӳ����ص����ݶ�
 * ��Ϊ�꣬������޸Ļ�����ֲ����ǳ����㡣
 */
#define BEEP_RCC_CLOCKCMD             RCC_APB2PeriphClockCmd
#define BEEP_RCC_CLOCKGPIO            RCC_APB2Periph_GPIOB
#define BEEP_GPIO_PIN                 GPIO_Pin_14
#define BEEP_GPIO                     GPIOB

#if 0         /* ���ñ�׼�⺯������ */

#define BEEP_ON                       GPIO_SetBits(BEEP_GPIO,BEEP_GPIO_PIN)
#define BEEP_OFF                      GPIO_ResetBits(BEEP_GPIO,BEEP_GPIO_PIN)
#define BEEP_TOGGLE                   {BEEP_GPIO->ODR ^=BEEP_GPIO_PIN;}  //�����ת

#else       /* ֱ�Ӳ����Ĵ������� */

#define BEEP_ON                       {BEEP_GPIO->BSRR=BEEP_GPIO_PIN;}    //����ߵ�ƽ
#define BEEP_OFF                      {BEEP_GPIO->BRR=BEEP_GPIO_PIN;}   //����͵�ƽ
#define BEEP_TOGGLE                   {BEEP_GPIO->ODR ^=BEEP_GPIO_PIN;}  //�����ת

#endif

/* ��չ���� ------------------------------------------------------------------*/
/* �������� ------------------------------------------------------------------*/
void BEEP_GPIO_Init(void);
void BEEP_StateSet(BEEPState_TypeDef state);
  
#endif  // __BSP_BEEP_H__

/******************* (C) COPYRIGHT 2015-2020 ӲʯǶ��ʽ�����Ŷ� *****END OF FILE****/
