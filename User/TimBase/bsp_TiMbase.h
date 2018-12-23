#ifndef TIME_TEST_H
#define TIME_TEST_H

#include "stm32f10x.h"
#include <stdbool.h>

#define WATCH_DOG_FUNCTION_ENABLE

enum
{
    Sys_Delay_Task,
    SYS_RST_Task,
    SYS_Time_ms_Task,
    SYS_Time_1s_Task,
	
	TIME_MAX_TASKS,
};

//#define   FilterSelfCheck_Time_Task   2

extern __IO u32 WatitTimingOut[];

//#define Check_TimOut()   !WatitTimingOut
#define TIM2_Wait_Period      1        //1ms 更新一次
#define TIM3_Wait_Period      1        //1s 更新一次  <取值 1-32>

void TIM2_NVIC_Configuration(void);
void TIM2_Configuration(void);
void Init_TIM2(void);

void TIM2_Delay_Decrement(void);
void SetTimeOut( __IO u32 nTime , u8 task);
bool Check_TimOut(u8 task);
void Sys_Delay_ms(__IO u32 nTime);

void Init_TIM3(void);

#ifdef WATCH_DOG_FUNCTION_ENABLE
void IWDG_Configuration(void);
#endif

#endif	/* TIME_TEST_H */
