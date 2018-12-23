#ifndef __UART_API_H
#define	__UART_API_H

#include "stm32f10x.h"
#include <stdbool.h>

#define RX_BUF_MAX_LEN     1024                                     //�����ջ����ֽ���

typedef struct  __STRUCT_USARTx_Fram                                  //��������֡�Ĵ���ṹ��
{
	char  Data_RX_BUF[ RX_BUF_MAX_LEN ];
	struct 
	{
			__IO u16 FramLength       :15;                               // 14:0 
			__IO u16 FramFinishFlag   :1;                                // 15 
	} InfBit;
}STRUCT_USARTx_Fram;

bool FucCheckSum(const u8 *p, u8 ln);
bool FucCheckSum2(const u8 *p, u8 ln);

#endif

