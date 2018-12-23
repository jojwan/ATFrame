#ifndef _BSP_PORT_H
#define _BSP_PORT_H

#include "app_fifo.h"

#define  RCC_NBIOT_PWERKEY     RCC_APB2Periph_GPIOD
#define  NBIOT_PWERKEY_PORT    GPIOD
#define  NBIOT_PWERKEY_PIN     GPIO_Pin_2

#define  RCC_NBIOT_RST         RCC_APB2Periph_GPIOD
#define  NBIOT_RST_PORT        GPIOD
#define  NBIOT_RST_PIN         GPIO_Pin_1



extern app_fifo_t      m_ble_rx_fifo;
extern app_fifo_t      m_ble_tx_fifo;
extern app_fifo_t      m_433m_rx_fifo;
extern app_fifo_t      m_debug_rx_fifo;
extern app_fifo_t      m_nb_rx_fifo;

void Init_GSM_Port(void);
void init_fifo_buffer(void);

#endif
