/**
  ******************************************************************************
  * @file    Project/STM32F10x_StdPeriph_Template/stm32f10x_it.c 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTI
  
  AL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"
#include "bsp_SysTick.h"
#include "bsp_TiMbase.h" 
#include "bsp_usart.h"
#include "app_fifo.h"
#include "bsp_port.h"

/** @addtogroup STM32F10x_StdPeriph_Template
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

__asm void HardFault_wait()
{
      BX lr
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
    /* Go to infinite loop when Hard Fault exception occurs */
//    #if 1
//    HardFault_wait();
//    #else
//    NVIC_SystemReset();
//    #endif
}


/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles RTC interrupt request.
  * @param  None
  * @retval None
  */
void RTC_IRQHandler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
	TimingDelay_Decrement();
}


/**
  * @brief  This function handles TIM2 interrupt request.
  * @param  None
  * @retval None
  */
void TIM2_IRQHandler(void)
{
	if ( TIM_GetITStatus(TIM2 , TIM_IT_Update) != RESET ) 
	{	
		TIM2_Delay_Decrement();
		
		TIM_ClearITPendingBit(TIM2 , TIM_FLAG_Update);  		 
	}
}

uint32_t m_world_time;
/**
  * @brief  This function handles TIM2 interrupt request.
  * @param  None
  * @retval None
  */
void TIM3_IRQHandler(void)
{
	if ( TIM_GetITStatus(TIM3 , TIM_IT_Update) != RESET ) 
	{	
        m_world_time++;
//        printf("%d\r\n",m_world_time);	
        
//        #ifdef WATCH_DOG_FUNCTION_ENABLE
//        IWDG_ReloadCounter();
//        #endif
        
		TIM_ClearITPendingBit(TIM3 , TIM_FLAG_Update);  		 
	}
}

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */ 

u8 bdebug_prf = 0;
u8 sw_prtf_gsm = 0;
u8 sw_process_ota = 0xff;

static uint8_t us_stage;
static uint8_t temp[3];
static uint16_t tempx;

//void IRQHandler_Debug_Msg( void )
//{	
//	u8 ch = 0;
//	if(USART_GetITStatus(UART_Debug_Msg, USART_IT_RXNE) != RESET)
//	{
//		ch  = USART_ReceiveData( UART_Debug_Msg );
////		UART_Debug_Msg_Send_Byte( &ch, 1 );
////        UART_GSM_Send_Byte( &ch, 1 );
////        UART_BLE_Send_Byte( &ch, 1 );
////        UART_433M_Send_Byte( &ch, 1 );
//        
//        app_fifo_put( &m_debug_rx_fifo, ch );
//        
//	}
//	 	 
//	if ( USART_GetITStatus( UART_Debug_Msg, USART_IT_IDLE ) == SET )                                         //数据帧接收完毕
//	{
//		ch  = USART_ReceiveData( UART_Debug_Msg );                      //由软件序列清除中断标志位(先读USART_SR，然后读USART_DR)
//    }	

//}

uint16_t read_us_temp(void)
{
    if(us_stage==5)
    {
        return tempx;
//        us_stage = 0;
    }else
    {
        return 0;
    }
}

//433M
void IRQHandler_433M( void )
{	
	u8 ch;
	if(USART_GetITStatus(UART_433M, USART_IT_RXNE) != RESET)
	{
		ch  = USART_ReceiveData( UART_433M );
//        UART_Debug_Msg_Send_Byte( &ch, 1 );
        app_fifo_put( &m_433m_rx_fifo, ch );
        
	}	 
	if ( USART_GetITStatus( UART_433M, USART_IT_IDLE ) == SET )                                         //数据帧接收完毕
	{
		ch = USART_ReceiveData( UART_433M );                                                              //由软件序列清除中断标志位(先读USART_SR，然后读USART_DR)
  }	
}

//
void IRQHandler_BLE( void )
{	
	u8 ch;
	if(USART_GetITStatus(UART_BLE, USART_IT_RXNE) != RESET)
	{
		ch  = USART_ReceiveData( UART_BLE );
//		UART_BLE_Send_Byte( &ch, 1 );
        
        app_fifo_put( &m_ble_rx_fifo, ch );
//        UART_Debug_Msg_Send_Byte( &ch, 1 );

	}
	 	 
	if ( USART_GetITStatus( UART_BLE, USART_IT_IDLE ) == SET )                                         //数据帧接收完毕
	{
		ch = USART_ReceiveData( UART_BLE );                                                              //由软件序列清除中断标志位(先读USART_SR，然后读USART_DR)
    }
}

void IRQHandler_GSM( void )
{	
	u8 ch;
	if(USART_GetITStatus(UART_GSM, USART_IT_RXNE) != RESET)
	{
		ch  = USART_ReceiveData( UART_GSM );
        app_fifo_put( &m_nb_rx_fifo, ch );
	}
	 	 
	if ( USART_GetITStatus( UART_GSM, USART_IT_IDLE ) == SET )                                         //数据帧接收完毕
	{
		ch = USART_ReceiveData( UART_GSM );                                                              //由软件序列清除中断标志位(先读USART_SR，然后读USART_DR)
    }	

}



/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
