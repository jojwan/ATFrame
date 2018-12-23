/**
  ******************************************************************************
  * @file    bsp_usart1.c
  * @author  fire
  * @version V1.0
  * @date    2013-xx-xx
  * @brief   重现c库printf函数到usart端口
  ******************************************************************************
  * @attention
  *
  * 实验平台:野火 ISO-MINI STM32 开发板 
  * 论坛    :http://www.chuxue123.com
  * 淘宝    :http://firestm32.taobao.com
  *
  ******************************************************************************
  */ 
  
#include "bsp_usart.h"
#include <string.h>
#include <stdarg.h>

#include <stdbool.h>

 /**
  * @brief  USART1 GPIO 配置,工作模式配置。9600-8-N-1
  * @param  无
  * @retval 无
  */
void USART1_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    
    /* config USART1 clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);
    
//    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1| RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO,ENABLE);
//    GPIO_PinRemapConfig(GPIO_Remap_USART1,ENABLE); //I/O口重映射开启
    
    /* USART1 GPIO config */
    /* Configure USART1 Tx (PA.09) as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);    
    /* Configure USART1 Rx (PA.10) as input floating */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
        
    /* USART1 mode config */
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No ;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure); 
    
    /* 使能串口1接收中断 */
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    /* 使能串口1总线空闲中断 */
    USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);

    USART_Cmd(USART1, ENABLE);
}

/// 配置NVIC中的UART1中断
void USART1_NVIC_Configuration( void )
{
	NVIC_InitTypeDef NVIC_InitStructure; 
	
	/* Configure the NVIC Preemption Priority Bits */  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

	/* Enable the USART1 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;	 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = UART1_PreemptionPriority;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = UART1_SubPriority;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

}

void USART1_Send_Byte(u8 *ch, u16 length)
{
	u16 i;
	for(i = 0; i < length; i ++)
	{
        /* 发送一个字节数据到USART1 */
        USART_SendData(USART1, (uint8_t) ch[i]);
        
        /* 等待发送完毕 */
        while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);			
	}
}


 /**
  * @brief  USART2 GPIO 配置,工作模式配置。9600-8-N-1
  * @param  无
  * @retval 无
  */
void USART2_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    
    /* config USART1 clock */
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA,  ENABLE );
    RCC_APB1PeriphClockCmd( RCC_APB1Periph_USART2, ENABLE );

    /* USART2 GPIO config */
    /* Configure USART1 Tx (PA.02) as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);    
    /* Configure USART2 Rx (PA.03) as input floating */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
        
    /* USART2 mode config */
    USART_InitStructure.USART_BaudRate = 9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No ;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART2, &USART_InitStructure); 
    
    /* 使能串口2接收中断 */
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
    /* 使能串口2总线空闲中断 */
    USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);

    USART_Cmd(USART2, ENABLE);
}

/// 配置NVIC中的UART1中断
void USART2_NVIC_Configuration( void )
{
	NVIC_InitTypeDef NVIC_InitStructure; 
	
	/* Configure the NVIC Preemption Priority Bits */  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

	/* Enable the USART1 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;	 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = UART2_PreemptionPriority;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = UART2_SubPriority;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

}

void USART2_Send_Byte(u8 *ch, u16 length)
{
	u16 i;
	for(i = 0; i < length; i ++)
	{
        USART_SendData(USART2, (uint8_t) ch[i]);
        while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);			
	}
}

 /**
  * @brief  USART3 GPIO 配置,工作模式配置。9600-8-N-1
  * @param  无
  * @retval 无
  */
void USART3_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    
    /* config USART3 clock */
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB,  ENABLE );
    RCC_APB1PeriphClockCmd( RCC_APB1Periph_USART3, ENABLE );

    /* USART3 GPIO config */
    /* Configure USART3 Tx (PB.10) as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);    
    /* Configure USART3 Rx (PB.11) as input floating */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
        
    /* USART3 mode config */
    USART_InitStructure.USART_BaudRate = 9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No ;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART3, &USART_InitStructure); 
    
    /* 使能串口3接收中断 */
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
    /* 使能串口3总线空闲中断 */
    USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);

    USART_Cmd(USART3, ENABLE);
}

/// 配置NVIC中的UART1中断
void USART3_NVIC_Configuration( void )
{
	NVIC_InitTypeDef NVIC_InitStructure; 
	
	/* Configure the NVIC Preemption Priority Bits */  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

	/* Enable the USART1 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;	 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = UART3_PreemptionPriority;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = UART3_SubPriority;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

}

void USART3_Send_Byte(u8 *ch, u16 length)
{
	u16 i;
	for(i = 0; i < length; i ++)
	{
        USART_SendData(USART3, (uint8_t) ch[i]);
        while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);			
	}
}

 /**
  * @brief  UART4 GPIO 配置,工作模式配置。9600-8-N-1
  * @param  无
  * @retval 无
  */
void UART4_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    
    /* config UART1 clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE );
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC , ENABLE);
    
    /* USART1 GPIO config */
    /* Configure UART1 Tx (PC.10) as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);    
    /* Configure UART4 Rx (PC.11) as input floating */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
        
    /* UART4 mode config */
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No ;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(UART4, &USART_InitStructure); 
    
    /* 使能串口4接收中断 */
    USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);
    /* 使能串口4总线空闲中断 */
    USART_ITConfig(UART4, USART_IT_IDLE, ENABLE);

    USART_Cmd(UART4, ENABLE);
}

/// 配置NVIC中的UART4中断
void UART4_NVIC_Configuration( void )
{
	NVIC_InitTypeDef NVIC_InitStructure; 
	
	/* Configure the NVIC Preemption Priority Bits */  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

	/* Enable the USART1 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;	 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = UART4_PreemptionPriority;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = UART4_SubPriority;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

}

void UART4_Send_Byte(u8 *ch, u16 length)
{
	u16 i;
	for(i = 0; i < length; i ++)
	{
        /* 发送一个字节数据到USART1 */
        USART_SendData(UART4, (uint8_t) ch[i]);
        
        /* 等待发送完毕 */
        while (USART_GetFlagStatus(UART4, USART_FLAG_TXE) == RESET);			
	}
}

 /**
  * @brief  UART5 GPIO 配置,工作模式配置。9600-8-N-1
  * @param  无
  * @retval 无
  */
void UART5_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    
    /* config UART1 clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE );
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD , ENABLE);
    
    /* UART5 GPIO config */
    /* Configure UART5 Tx (PC.12) as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);    
    /* Configure UART5 Rx (PD.02) as input floating */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
        
    /* UART5 mode config */
    USART_InitStructure.USART_BaudRate = 9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No ;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(UART5, &USART_InitStructure); 
    
    /* 使能串口5接收中断 */
    USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);
    /* 使能串口5总线空闲中断 */
    USART_ITConfig(UART5, USART_IT_IDLE, ENABLE);
	
	USART_ClearFlag(UART5,USART_FLAG_TC);

    USART_Cmd(UART5, ENABLE);
}

/// 配置NVIC中的UART5中断
void UART5_NVIC_Configuration( void )
{
	NVIC_InitTypeDef NVIC_InitStructure; 
	
	/* Configure the NVIC Preemption Priority Bits */  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

	/* Enable the USART1 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;	 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = UART5_PreemptionPriority;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = UART5_SubPriority;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

}

void UART5_Send_Byte(u8 *ch, u16 length)
{
	u16 i;
	for(i = 0; i < length; i ++)
	{
        /* 发送一个字节数据到USART1 */
        USART_SendData(UART5, (uint8_t) ch[i]);
        
        /* 等待发送完毕 */
        while (USART_GetFlagStatus(UART5, USART_FLAG_TXE) == RESET);			
	}
}

void UART_Send_Byte( USART_TypeDef* USARTx, char *ch, u16 length )
{
	u16 i;
	for(i = 0; i < length; i ++)
	{
        /* 发送一个字节数据到USART1 */
        USART_SendData(USARTx, (uint8_t) ch[i]);
        
        /* 等待发送完毕 */
        while (USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET);			
	}
}


////重定向c库函数printf到USART1
int fputc(int ch, FILE *f)
{
    /* 发送一个字节数据到USART1 */
    USART_SendData(UART_Debug_Msg, (uint8_t) ch);
    
    /* 等待发送完毕 */
    while (USART_GetFlagStatus(UART_Debug_Msg, USART_FLAG_TXE) == RESET);		

    return (ch);
}

/////重定向c库函数scanf到USART1
//int fgetc(FILE *f)
//{
//    /* 等待串口1输入数据 */
//    while (USART_GetFlagStatus(UART_BLE, USART_FLAG_RXNE) == RESET);

//    return (int)USART_ReceiveData(UART_BLE);
//}

#define UART_STR_BUF_SIZE	512
void Uart_Send_String(USART_TypeDef* USARTx, const char * str, ...)
{
	uint16_t len;
	va_list ap;
    char uart_str_buf[UART_STR_BUF_SIZE];
	memset(uart_str_buf, 0, UART_STR_BUF_SIZE);

	va_start( ap, str );
	vsprintf( uart_str_buf, str, ap );
	va_end( ap );

	len = strlen(uart_str_buf);

	UART_Send_Byte(USARTx, uart_str_buf, len);
}

void UART_Config(void)
{
	UART_Config_GSM ();
	NVIC_Configuration_GSM();
    
    UART_Config_Debug_Msg();
	NVIC_Configuration_Debug_Msg();
}




