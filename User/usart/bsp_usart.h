#ifndef __USART1_H
#define	__USART1_H

#include "stm32f10x.h"
#include <stdio.h>

#define     GPRS_FUN_ENABLE
#define     ESP8266_FUN_ENABLE

//#define StrMaxLength   256

//typedef struct
//{
//    u8 StrLength;
//    u8 StrBuf[StrMaxLength];
//}StrData;

#define   UART_BLE_Priority            2
#define   UART_GPRS_Priority           0
#define   UART_Debug_Msg_Priority      3
#define   UART_433M_Priority           1
#define   UART_NH3_Priority            2

#define UART_BLE                         USART2
#define UART_Config_BLE                  USART2_Config
#define NVIC_Configuration_BLE           USART2_NVIC_Configuration
#define UART_BLE_Send_Byte               USART2_Send_Byte
#define IRQHandler_BLE                   USART2_IRQHandler
#define UART2_PreemptionPriority         UART_BLE_Priority
#define UART2_SubPriority                UART_BLE_Priority
                                         
#define UART_GSM                         UART4
#define UART_Config_GSM                  UART4_Config
#define NVIC_Configuration_GSM           UART4_NVIC_Configuration
#define UART_GSM_Send_Byte               UART4_Send_Byte
#define IRQHandler_GSM                   UART4_IRQHandler
#define UART4_PreemptionPriority         UART_NH3_Priority
#define UART4_SubPriority                UART_NH3_Priority

#define UART_Debug_Msg                   USART1
#define UART_Config_Debug_Msg            USART1_Config
#define NVIC_Configuration_Debug_Msg     USART1_NVIC_Configuration
#define UART_Debug_Msg_Send_Byte         USART1_Send_Byte
#define IRQHandler_Debug_Msg             USART1_IRQHandler
#define UART1_PreemptionPriority         UART_Debug_Msg_Priority
#define UART1_SubPriority                UART_Debug_Msg_Priority

#define UART_WIFI                        USART3
#define UART_Config_WIFI                 USART3_Config
#define NVIC_Configuration_WIFI          USART3_NVIC_Configuration
#define UART_WIFI_Send_Byte              USART3_Send_Byte
#define IRQHandler_WIFI                  USART3_IRQHandler
//#define UART3_PreemptionPriority         UART_WIFI_Priority
//#define UART3_SubPriority                UART_WIFI_Priority

#define UART_433M                        USART3
#define UART_Config_433M                 USART3_Config
#define NVIC_Configuration_433M          USART3_NVIC_Configuration
#define UART_433M_Send_Byte              USART3_Send_Byte
#define IRQHandler_433M                  USART3_IRQHandler
#define UART3_PreemptionPriority         UART_433M_Priority
#define UART3_SubPriority                UART_433M_Priority

#define UART_NH3                         UART5
#define UART_Config_NH3                  UART5_Config
#define NVIC_Configuration_NH3           UART5_NVIC_Configuration
#define UART_NH3_Send_Byte               UART5_Send_Byte
#define IRQHandler_NH3                   UART5_IRQHandler
#define UART5_PreemptionPriority         UART_NH3_Priority
#define UART5_SubPriority                UART_NH3_Priority

void USART1_Config(void);
void USART1_NVIC_Configuration( void );
void USART1_Send_Byte(u8 *ch, u16 length);

void USART2_Config(void);
void USART2_NVIC_Configuration( void );
void USART2_Send_Byte(u8 *ch, u16 length);

void USART3_Config(void);
void USART3_NVIC_Configuration( void );
void USART3_Send_Byte(u8 *ch, u16 length);

void UART4_Config(void);
void UART4_NVIC_Configuration( void );
void UART4_Send_Byte(u8 *ch, u16 length);

void UART5_Config(void);
void UART5_NVIC_Configuration( void );
void UART5_Send_Byte(u8 *ch, u16 length);

void UART_Send_Byte( USART_TypeDef* USARTx, char *ch, u16 length );
void Uart_Send_String(USART_TypeDef* USARTx, const char * str, ...);
void UART_Config(void);

#define     DEBUG_MSG_ENABLE                   1
#define     DebugMsg                        printf//

#endif /* __USART1_H */
