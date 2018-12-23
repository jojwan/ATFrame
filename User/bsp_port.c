#include "app_fifo.h"
#include "stm32f10x.h"
#include "bsp_port.h"

#define  NB_RX_BUFFER_LENGTH   1024 
app_fifo_t      m_nb_rx_fifo;
u8       m_nb_rx_buffer[NB_RX_BUFFER_LENGTH];

#define  BLE_RX_BUFFER_LENGTH   512 
app_fifo_t      m_ble_rx_fifo;
u8       m_ble_rx_buffer[BLE_RX_BUFFER_LENGTH];

#define  BLE_TX_BUFFER_LENGTH   512 
app_fifo_t      m_ble_tx_fifo;
static u8       m_ble_tx_buffer[BLE_TX_BUFFER_LENGTH];

#define  M433_RX_BUFFER_LENGTH   128 
app_fifo_t      m_433m_rx_fifo;
u8       m_433m_rx_buffer[M433_RX_BUFFER_LENGTH];

#define  DEBUG_BUFFER_LENGTH   128
app_fifo_t      m_debug_rx_fifo;
u8       m_debug_rx_buffer[DEBUG_BUFFER_LENGTH];

void init_fifo_buffer(void)
{
    app_fifo_init( &m_ble_rx_fifo,   m_ble_rx_buffer,   BLE_RX_BUFFER_LENGTH);
    app_fifo_init( &m_ble_tx_fifo,   m_ble_tx_buffer,   BLE_TX_BUFFER_LENGTH);
    app_fifo_init( &m_433m_rx_fifo,  m_433m_rx_buffer,  M433_RX_BUFFER_LENGTH);
    app_fifo_init( &m_debug_rx_fifo, m_debug_rx_buffer, DEBUG_BUFFER_LENGTH);
    app_fifo_init( &m_nb_rx_fifo,    m_nb_rx_buffer,    NB_RX_BUFFER_LENGTH);
}

void temp_delay(void)
{
    uint32_t i;
    for (i=0;i<10000000;i++) ;
}

/*
SIM-开机: 
SIM-PWERKEY拉低后恢复
SIM-STATUS 电平为高
SIM-关机: 
SIM-PWERKEY拉低后恢复
SIM-STATUS 电平为低

MCU-PWERKEY 置高 SIM-PWERKEY 为低
MCU-PWERKEY 置低 SIM-PWERKEY 为高
*/

/*
SIM-reset: 
SIM-RST low 
MCU-RST 置高 SIM-RST 为低
*/

void Init_GSM_Port(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
	RCC_APB2PeriphClockCmd(RCC_NBIOT_PWERKEY, ENABLE);	//使能GPIOA时钟	
	GPIO_InitStructure.GPIO_Pin    = NBIOT_PWERKEY_PIN ;//8
    /*设置引脚模式为通用推挽输出*/
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   
	GPIO_InitStructure.GPIO_Speed  = GPIO_Speed_50MHz;	
	GPIO_Init(NBIOT_PWERKEY_PORT, &GPIO_InitStructure);    
	//GSM_PWRKEY -- low work
	//拉低启动   
    GPIO_ResetBits(NBIOT_PWERKEY_PORT, NBIOT_PWERKEY_PIN );
    temp_delay();
    #if 1  // 核心
    GPIO_SetBits(NBIOT_PWERKEY_PORT, NBIOT_PWERKEY_PIN );
    temp_delay();
    GPIO_ResetBits(NBIOT_PWERKEY_PORT, NBIOT_PWERKEY_PIN );
    #endif
    temp_delay();
//    GPIO_ResetBits(NBIOT_PWERKEY_PORT, NBIOT_PWERKEY_PIN );
    
    #if 1
	GPIO_InitStructure.GPIO_Pin    = NBIOT_RST_PIN ;//
    /*设置引脚模式为通用推挽输出*/
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
	GPIO_InitStructure.GPIO_Speed  = GPIO_Speed_50MHz;

    GPIO_Init(NBIOT_RST_PORT, &GPIO_InitStructure);
    
//    GPIO_SetBits(NBIOT_RST_PORT, NBIOT_RST_PIN );
    GPIO_ResetBits(NBIOT_RST_PORT, NBIOT_RST_PIN );
    #endif
}


