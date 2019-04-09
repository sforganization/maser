

/**
******************************************************************************
* @file 	
* @authors	
* @version	V1.0.0
* @date 	
* @brief	
******************************************************************************
*/
#include "san_flash.h"
#include "stm32f10x_flash.h"



/**********************************************************************
* ����: 
* ����:
* �β�: 
* ����: ��
* ˵��: 
**********************************************************************/

//��ȡָ����ַ�İ���(16λ����)
u16 FLASH_ReadHalfWord(u32 address)
{
	return * (__IO u16 *)address;
}

//��ָ����ַ��ʼ��ȡ�������
void FLASH_ReadMoreData(u32 startAddress, u16 * readData, u16 countToRead)
{
	u16 			dataIndex;

	for (dataIndex = 0; dataIndex < countToRead; dataIndex++)
	{
		readData[dataIndex] = FLASH_ReadHalfWord(startAddress + dataIndex * 2);
	}
}



//��ȡָ����ַ��ȫ��(32λ����)
u32 FLASH_ReadWord(u32 address)
{
	u32 			temp1, temp2;

	temp1				= * (__IO u16 *)
	address;
	temp2				= * (__IO u16 *) (address + 2);
	return (temp2 << 16) +temp1;
}


//��ָ����ַ��ʼд��������
s8 FLASH_WriteMoreData(u32 startAddress, u16 * writeData, u16 countToWrite)
{
    FLASH_Status status;
	if (startAddress < FLASH_BASE || ((startAddress + countToWrite * 2) >= (FLASH_BASE + 1024 * FLASH_SIZE)))
	{
        return -1; //�Ƿ���ַ
	}

	FLASH_Unlock(); 								//����д����
	u32 			offsetAddress = startAddress - FLASH_BASE; //����ȥ��0X08000000���ʵ��ƫ�Ƶ�ַ
	u32 			sectorPosition = offsetAddress / SECTOR_SIZE; //����������ַ������STM32F103VET6Ϊ0~255

	u32 			sectorStartAddress = sectorPosition * SECTOR_SIZE + FLASH_BASE; //��Ӧ�������׵�ַ


	status = FLASH_ErasePage(sectorStartAddress);			//�����������
    if(status != FLASH_COMPLETE)
        goto END;
        
	u16 			dataIndex;

	for (dataIndex = 0; dataIndex < countToWrite; dataIndex++)
	{
		status = FLASH_ProgramHalfWord(startAddress + dataIndex * 2, writeData[dataIndex]);
        
        if(status != FLASH_COMPLETE)
            goto END;
	}

END:
	FLASH_Lock();									//����д����
    
    if(status != FLASH_COMPLETE)
        return -1;
    else
        return 0; 
}


