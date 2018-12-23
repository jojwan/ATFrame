#ifndef __SYSTICK_H_
#define __SYSTICK_H_

#include "stm32f10x.h"

void Init_SysTick( void );
void TimingDelay_Decrement( void );
void Tim_Delay_ms( __IO u32 nTime );

void SysRunTime_Enable(void);
void SysRunTime_Disable(void);

u32 GetRunTime(void);

#endif /* __SYSTICK_H */
