/**
  ******************************************************************************
  * @file    bsp_SysTick.c
  * @author  fire
  * @version V1.0
  * @date    2013-xx-xx
  * @brief   SysTick ϵͳ�δ�ʱ��10us�жϺ�����,�ж�ʱ����������ã�
  *          ���õ��� 1us 10us 1ms �жϡ�     
  ******************************************************************************
  */
  
#include "bsp_SysTick.h"


static __IO u32 TimingDelay = 0;
 
 
/**
  * @brief  ����ϵͳ�δ�ʱ�� SysTick
  * @param  ��
  * @retval ��
  */
void Init_SysTick( void )
{
    
	/* SystemFrequency / 1000    1ms�ж�һ��
	 * SystemFrequency / 100000	 10us�ж�һ��
	 * SystemFrequency / 1000000 1us�ж�һ��
	 */
	if ( SysTick_Config(SystemCoreClock / 1000000) )	// ST3.5.0��汾
	{ 
		/* Capture error */ 
		while (1);
	}
    TimingDelay = 0;
	// �رյδ�ʱ��  
	SysTick->CTRL &= ~ SysTick_CTRL_ENABLE_Msk;
    
}


///**
//  * @brief   ms��ʱ����,1msΪһ����λ
//  * @param  
//  *		@arg nTime: Delay_ms( 1 ) ��ʵ�ֵ���ʱΪ 1 * 1ms = 1ms
//  * @retval  ��
//  */
//void Tim_Delay_ms( __IO u32 nTime )
//{ 
//	TimingDelay = nTime;	

//	// ʹ�ܵδ�ʱ��  
//	SysTick->CTRL |=  SysTick_CTRL_ENABLE_Msk;

//	while( TimingDelay != 0 );
//	
//}

void SysRunTime_Enable(void)
{
    TimingDelay = 0xfffffff0;

	// ʹ�ܵδ�ʱ��  
	SysTick->CTRL |=  SysTick_CTRL_ENABLE_Msk;
    
}

void SysRunTime_Disable(void)
{
    TimingDelay = 0;

	// �رյδ�ʱ��  
	SysTick->CTRL &= ~ SysTick_CTRL_ENABLE_Msk; 
    
}

u32 GetRunTime(void)
{
	// �رյδ�ʱ��  
	SysTick->CTRL &= ~ SysTick_CTRL_ENABLE_Msk;    
    return (0xfffffff0-TimingDelay);
}

/**
  * @brief  ��ȡ���ĳ���
  * @param  ��
  * @retval ��
  * @attention  �� SysTick �жϺ��� SysTick_Handler()����
  */
void TimingDelay_Decrement(void)
{
	if ( TimingDelay )
	{ 
		TimingDelay -- ;
	}
}


/*********************************************END OF FILE**********************/
