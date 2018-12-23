#include "bsp_SysTick.h"
#include "bsp_TiMbase.h"
#include "app_fifo.h"
#include "bsp_usart.h"
#include "bsp_port.h"
#include "NB_IOT.h"
#include "app_gsm.h"

#define     MAIN_DEBUG_Usart          DebugMsg

/**
  * @brief  ������
  * @param  ��
  * @retval ��
  */
int main(void)
{
    u8 ch;
	Init_TIM2();
    
    Init_GSM_Port();

    //������ڴ��ڳ�ʼ��ǰ����Ȼ���ڽ����ж�һ�����ͽ��յ����ݾͻ�����ܷ�
    init_fifo_buffer();
    Init_Task_NBIOT();
    
	/* UART config */
    UART_Config();

    while (1)
    {    
        if (app_fifo_get( &m_debug_rx_fifo, &ch ) == FIFO_SUCCESS)
        {
        }
        
        Run_Task_NBIOT();
        
        #define  SYS_TIME_TASK_TIME   100  // 5ms
        if ( Check_TimOut(SYS_Time_ms_Task) )
        {
            Gsm_Timer(SYS_TIME_TASK_TIME);
            
            SetTimeOut( SYS_TIME_TASK_TIME , SYS_Time_ms_Task );
        }
    }
    
}


/*********************************************END OF FILE**********************/

