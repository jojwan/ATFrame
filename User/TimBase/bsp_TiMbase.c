/**

  ******************************************************************************
  * @file    bsp_TimBase.c
  * @author  fire
  * @version V1.0
  * @date    2013-xx-xx
  * @brief   TIM2 1ms 定时应用bsp
  ******************************************************************************
  * @attention
  *
  * 实验平台:野火 ISO-MINI STM32 开发板 
  * 论坛    :http://www.chuxue123.com
  * 淘宝    :http://firestm32.taobao.com
  *
  ******************************************************************************
  */ 
  

#include "bsp_TiMbase.h" 
#include "stm32f10x_iwdg.h"


//static 

__IO u32 WatitTimingOut[TIME_MAX_TASKS] = {0};


/// TIM2中断优先级配置
void TIM2_NVIC_Configuration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure; 
    
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);  													
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;	  
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;	
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/// TIM3中断优先级配置
void TIM3_NVIC_Configuration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure; 
    
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);  													
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;	  
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;	
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/*
 * TIM_Period / Auto Reload Register(ARR) = 1000   TIM_Prescaler--71 
 * 中断周期为 = 1/(72MHZ /72) * 1000 = 1ms
 *
 * TIMxCLK/CK_PSC --> TIMxCNT --> TIM_Period(ARR) --> 中断 且TIMxCNT重置为0重新计数 
 */
void TIM2_Configuration(void)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
		
		/* 设置TIM2CLK 为 72MHZ */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 , ENABLE);
    //TIM_DeInit(TIM2);
	
	/* 自动重装载寄存器周期的值(计数值) */
    TIM_TimeBaseStructure.TIM_Period = 1000*TIM2_Wait_Period;     
	
    /* 累计 TIM_Period个频率后产生一个更新或者中断 */
	  /* 时钟预分频数为72 */
    TIM_TimeBaseStructure.TIM_Prescaler= 71;
	
		/* 对外部时钟进行采样的时钟分频,这里没有用到 */
    TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;
	
    TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up; 
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
	
    TIM_ClearFlag(TIM2, TIM_FLAG_Update);
	
    TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);
		
    TIM_Cmd(TIM2, ENABLE);																		
    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 , DISABLE);		/*先关闭等待使用*/    
}

/*
 * TIM_Period / Auto Reload Register(ARR) = 1000   TIM_Prescaler--71 
 * 中断周期为 = 1/(72MHZ /719) * 1000 = 1ms
 *
 * TIMxCLK/CK_PSC --> TIMxCNT --> TIM_Period(ARR) --> 中断 且TIMxCNT重置为0重新计数 
 */
void TIM3_Configuration(void)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
		
		/* 设置TIM3CLK 为 72MHZ */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3 , ENABLE);
    //TIM_DeInit(TIM3);
	
	/* 自动重装载寄存器周期的值(计数值) */
    TIM_TimeBaseStructure.TIM_Period = TIM3_Wait_Period*2000;     //TIM3_Wait_Period S
	
    /* 累计 TIM_Period个频率后产生一个更新或者中断 */
	/* 时钟预分频数为 (72*500-1) 500ms */
    TIM_TimeBaseStructure.TIM_Prescaler= 7200*5-1;
	
	/* 对外部时钟进行采样的时钟分频,这里没有用到 */
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	
    TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up; 
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
	
    TIM_ClearFlag(TIM3, TIM_FLAG_Update);
	
    TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);
		
    TIM_Cmd(TIM3, ENABLE);																		
    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3 , DISABLE);		/*先关闭等待使用*/    
}

void Init_TIM2(void)
{
    /* TIM2 定时配置 */	
    TIM2_Configuration();
    /* 实战定时器的中断优先级 */
    TIM2_NVIC_Configuration();
    /* TIM2 重新开时钟，开始计时 */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 , ENABLE);		
}

void Init_TIM3(void)
{
    /* TIM3 定时配置 */	
    TIM3_Configuration();
    /* 实战定时器的中断优先级 */
    TIM3_NVIC_Configuration();
    /* TIM3 重新开时钟，开始计时 */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3 , ENABLE);		
}

/**
  * @brief  获取节拍程序
  * @param  无
  * @retval 无
  * @attention  
  */
void TIM2_Delay_Decrement(void)
{
    uint8_t i;
    for (i = 0; i < TIME_MAX_TASKS; i++)
    {
        if ( WatitTimingOut[i] != 0x00 )
        { 
            WatitTimingOut[i] --;
        }
    }
}

void SetTimeOut( __IO u32 nTime , u8 task)
{
    WatitTimingOut[task] = nTime / TIM2_Wait_Period;
}

bool Check_TimOut(u8 task)
{
	if ( WatitTimingOut[task] == 0 )
		return true;
	else
		return false;
}

void Sys_Delay_ms(__IO u32 nTime)
{
	SetTimeOut( nTime , Sys_Delay_Task );
    
    while( !Check_TimOut(Sys_Delay_Task) );
}

#ifdef WATCH_DOG_FUNCTION_ENABLE
void IWDG_Configuration(void)
{
    if (RCC_GetFlagStatus(RCC_FLAG_IWDGRST) != RESET)
    {
        /* Clear reset flags */
        RCC_ClearFlag();
    }
    
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

    /* IWDG counter clock: LSI/64 */
    IWDG_SetPrescaler(IWDG_Prescaler_256);

    // 40KHZ 的输入时钟 LSI
    /* Set counter reload value to obtain 64ms IWDG TimeOut.
    40*1000/256 = 156.25HZ = 6.4ms
    6.4ms * 1562 = 9.99 s
    */
    // 0-4095
    IWDG_SetReload(1562); // 10s

    /* Reload IWDG counter */
    IWDG_ReloadCounter();

    /* Enable IWDG (the LSI oscillator will be enabled by hardware) */
    IWDG_Enable();
}
#endif

// 机械延时
void Delay_fun_ms(__IO uint32_t nCount)
{
  nCount = 0xfafeee/3000*nCount;
  for(; nCount != 0; nCount--);
}


/*********************************************END OF FILE**********************/
