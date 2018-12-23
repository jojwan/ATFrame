#include "bsp_usart_api.h"

/*
GPRS,O2���ݵ�У�鷽ʽ
*/
bool FucCheckSum(const u8 *p, u8 ln)
{
	u8 j,temp = 0;
	p ++;
	for(j=0;j<(ln-2);j++)
	{
			temp += *p;
			p ++;
	}
	temp = (~temp) + 1;
	return (temp == *p);
}

/*
PM���ݵ�У�鷽ʽ
*/
bool FucCheckSum2(const u8 *p, u8 ln)
{
	u8 j;
	u16 temp = 0;
	for(j=0;j<(ln-2);j++)
	{
			temp += p[j];
	}
	return ( temp == (u16)(p[ln-2]*256 + p[ln-1]) );
}
