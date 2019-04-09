/**
******************************************************************************
* @file    
* @authors  
* @version  V1.0.0
* @date     2015.09.15
* @brief    
******************************************************************************
*/

#ifndef  __SAN_FLASH_H__ 
#define  __SAN_FLASH_H__


#ifdef _MAININC_
#define EXTERN
#else
#define EXTERN extern
#endif

/* ͷ�ļ�  ------------------------------------------------------------*/
#include "stm32f10x.h"


/*
 *   Flash�Ĳ���Ҫ�������ҳ����������Ҳ������ҳд�룬������ܻᶪʧ����
 *   ��Ҫ�������˽���Բο���STM32F10xxx �����̲ο��ֲᡷ
 */



#define FLASH_SIZE 64          //��ѡMCU��FLASH������С(��λΪK)          STM32F103C8 64k 

#if FLASH_SIZE<256
  #define SECTOR_SIZE           1024    //�ֽ�С��MCU 1kÿҳ
#else 
  #define SECTOR_SIZE           2048    //�ֽ�
#endif

#define FLASH_SAVE_ADDR         0X0800C004 	    //����FLASH �����ַ(����Ϊż��������������,Ҫ���ڱ�������ռ�õ�������.
										        //����,д������ʱ��,���ܻᵼ�²�����������,�Ӷ����𲿷ֳ���ʧ.��������.

										

u32 FLASH_ReadWord(u32 address);

void FLASH_ReadMoreData(u32 startAddress, u16 * readData, u16 countToRead);

s8 FLASH_WriteMoreData(u32 startAddress, u16 * writeData, u16 countToWrite);

#endif
/******************************* FILE END ***********************************/
