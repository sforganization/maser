/**

  */

/* ����ͷ�ļ� ----------------------------------------------------------------*/
#include "key.h"

/* ˽�����Ͷ��� --------------------------------------------------------------*/
/* ˽�к궨�� ----------------------------------------------------------------*/
/* ˽�б��� ------------------------------------------------------------------*/
/* ��չ���� ------------------------------------------------------------------*/
/* ˽�к���ԭ�� --------------------------------------------------------------*/
/* ������ --------------------------------------------------------------------*/

/**
  * ��������: ���ذ���IO���ų�ʼ��.
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ����ʹ�ú궨�巽������������źţ����������ֲ��ֻҪ���޸�key.h
  *           �ļ���غ궨��Ϳ��Է����޸����š�
  */
void KEY_GPIO_Init(void)
{
   /* ����IOӲ����ʼ���ṹ����� */
  GPIO_InitTypeDef GPIO_InitStructure;

  

  //KEY1 �������룬jtagĬ�ϣ���Ϊ��ͨIO��Ҫ���� ��Ҫ�������Уɣϳ�ʼ�������Ϊ���ε���RCC_APB2PeriphClockCmd��ָ�jtag����
  //���԰�ָ��P15  IO  ��ʼ��Ҳ�ŵ�������

    //һ.��ʱ�������д򿪸���ʱ�ӣ�
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO,ENABLE);
    //��.��PB3���Ž�����ӳ�䣺
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE); 

	/* ʹ��(����)KEY1���Ŷ�ӦIO�˿�ʱ�� */  
  RCC_APB2PeriphClockCmd(KEY1_RCC_CLOCKGPIO|KEY2_RCC_CLOCKGPIO|KEY3_RCC_CLOCKGPIO | KEY4_RCC_CLOCKGPIO, ENABLE);
   
  /* �趨KEY1��Ӧ����IO��� */
  GPIO_InitStructure.GPIO_Pin = KEY1_GPIO_PIN;  
  /* �趨KEY1��Ӧ����IO�������ٶ� ��GPIO_Speed_50MHz */
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
  /* �趨KEY1��Ӧ����IOΪ��������ģʽ */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;  
  /* ��ʼ��KEY1��Ӧ����IO */
    
  GPIO_Init(KEY1_GPIO, &GPIO_InitStructure);

  
  /* �趨KEY2��Ӧ����IO��� */
  GPIO_InitStructure.GPIO_Pin = KEY2_GPIO_PIN;  
  /* ��ʼ��KEY2��Ӧ����IO */
  GPIO_Init(KEY2_GPIO, &GPIO_InitStructure);

  
  /* �趨KEY3��Ӧ����IO��� */
  GPIO_InitStructure.GPIO_Pin = KEY3_GPIO_PIN | KEY4_GPIO_PIN | KEY5_GPIO_PIN | KEY6_GPIO_PIN | KEY7_GPIO_PIN;  
  /* ��ʼ��KEY3��Ӧ����IO */
  GPIO_Init(KEY3_GPIO, &GPIO_InitStructure);


  //ָ��P15   IO��ʼ�� 
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;//��������ģʽ
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//50MHz
  GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIO	
  
}



/**
  * ��������: �򵥴ֱ�����ʱ����
  * �������: time����ʱʱ������
  * �� �� ֵ: ��
  * ˵    ���������ڰ����뵯�����̴��ڶ�����һ����Ҫ������������������򵥷���
  *           ���Ǽ���ʱ��
  */
void KEY_ScanDelay(void)
{  
  uint32_t i,j;
  for(i=0;i<100;++i)
    for(j=0;j<1000;++j){ }		
}

#if 0
/**
  * ��������: ��ȡ����KEY1��״̬
  * �����������
  * �� �� ֵ: KEY_DOWN�����������£�
  *           KEY_UP  ������û������
  * ˵    �����ޡ�
  */
KEYState_TypeDef KEY1_StateRead(void)
{
//  /* ��ȡ��ʱ����ֵ���ж��Ƿ��Ǳ�����״̬������Ǳ�����״̬���뺯���� */
//  if(GPIO_ReadInputDataBit(KEY1_GPIO,KEY1_GPIO_PIN)==KEY1_DOWN_LEVEL)
//  {
//    /* ��ʱһС��ʱ�䣬�������� */
//    KEY_ScanDelay();
//    /* ��ʱʱ��������жϰ���״̬��������ǰ���״̬˵������ȷʵ������ */
//    if(GPIO_ReadInputDataBit(KEY1_GPIO,KEY1_GPIO_PIN)==KEY1_DOWN_LEVEL)
//    {
//      /* �ȴ������������˳�����ɨ�躯�� */
//      while(GPIO_ReadInputDataBit(KEY1_GPIO,KEY1_GPIO_PIN)==KEY1_DOWN_LEVEL);
//       /* ����ɨ����ϣ�ȷ�����������£����ذ���������״̬ */
//      return KEY_DOWN;
//    }
//  }
//  /* ����û�����£�����û������״̬ */
//  return KEY_UP;
    return GPIO_ReadInputDataBit(KEY1_GPIO,KEY1_GPIO_PIN);
}

/**
  * ��������: ��ȡ����KEY2��״̬
  * �����������
  * �� �� ֵ: KEY_DOWN�����������£�
  *           KEY_UP  ������û������
  * ˵    �����ޡ�
  */
KEYState_TypeDef KEY2_StateRead(void)
{
//  /* ��ȡ��ʱ����ֵ���ж��Ƿ��Ǳ�����״̬������Ǳ�����״̬���뺯���� */
//  if(GPIO_ReadInputDataBit(KEY2_GPIO,KEY2_GPIO_PIN)==KEY2_DOWN_LEVEL)
//  {
//    /* ��ʱһС��ʱ�䣬�������� */
//    KEY_ScanDelay();
//    /* ��ʱʱ��������жϰ���״̬��������ǰ���״̬˵������ȷʵ������ */
//    if(GPIO_ReadInputDataBit(KEY2_GPIO,KEY2_GPIO_PIN)==KEY2_DOWN_LEVEL)
//    {
//      /* �ȴ������������˳�����ɨ�躯�� */
//      while(GPIO_ReadInputDataBit(KEY2_GPIO,KEY2_GPIO_PIN)==KEY2_DOWN_LEVEL);
//       /* ����ɨ����ϣ�ȷ�����������£����ذ���������״̬ */
//      return KEY_DOWN;
//    }
//  }
//  /* ����û�����£�����û������״̬ */
//  return KEY_UP;
    return GPIO_ReadInputDataBit(KEY2_GPIO,KEY2_GPIO_PIN);
}
#endif
/******************* (C) COPYRIGHT 2015-2020 ӲʯǶ��ʽ�����Ŷ� *****END OF FILE****/
