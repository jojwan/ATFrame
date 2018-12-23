#include "app_gsm.h"
#include "NB_IOT.h"
#include "bsp_usart.h"
#include "app_fifo.h"
#include "string.h"
#include "ATHandleFrame.h"
#include "bsp_port.h"

#define  APP_GSM_MSG    DebugMsg

extern app_fifo_t       m_usart_rx_fifo;

ATFrameStageMem           GsmATProc;

extern AtFrameStage       GsmWholeStageProc[];
extern Gsm_Control_Union	    Gsm_At_Flag;
extern u8 IsRevTimeout(void);
extern void SetATCMDRom (const char *cmd, uint16_t timeout);
extern ATFrameStrMem GsmTcpATFrameStr;

extern void InitStrProcLeng (void);
extern void InitStrProcInserAllNode (void);


void NB_Send_TestData(void);

void Init_Task_NBIOT(void)
{
    Gsm_Init ();

    InitATStageFrame (  &GsmATProc,    
                        (u32 *)&Gsm_At_Flag,
                        NULL,
                        IsRevTimeout,
                        SetATCMDRom);
    InitStrProcLeng ();
}

void Run_Task_NBIOT(void)
{
    uint8_t msg;
    AT_Cmd_Task (&GsmATProc, GsmWholeStageProc);
    if (app_fifo_get(&m_nb_rx_fifo, &msg) == FIFO_SUCCESS)
    {
        APP_GSM_MSG ("%c", msg);
//            GsmUartProcess (msg, DataHandle);
        AT_Cmd_String_Process (&GsmTcpATFrameStr, msg);
    }
}

u32 calcSum (u8 *buf, u16 len)
{
    u32 sum = 0, i = 0;
    if (buf == NULL)
        return 0;
    for (i = 0; i < len; i ++)
    {
        sum += buf[i];
    }
    return sum;
}

void debug_msg_show_hex (u8 *buf, u32 len, u32 seq)
{
    u32 i = 0;
    APP_GSM_MSG("send data %d-%d: ", seq, len);
    for (i = 0; i < len; i ++)
        {
            APP_GSM_MSG("0x%02X ", buf[i]);
        }
    APP_GSM_MSG("\r\n");
}

u8 lengthFlag;
void NB_Send_TestData(void)
{  
    u16 DATA_LENGTH = 10;
    u8 buffer[256] = {0};
    static u32  seq = 0;
    u32 * pTemp;
    
    if (!GetGsmConnectStatus() || !GetGsmSendStatus())
        return ;
    
    if (lengthFlag == 0)
        DATA_LENGTH = 10;
    else
        DATA_LENGTH = 120;
    seq ++;
    buffer[0] = 0x02;
    buffer[1] = (DATA_LENGTH-3);
    
    pTemp = (u32 *) &buffer[3];
    *pTemp = seq;
    
    memset (&buffer[3+4], 0x31, (DATA_LENGTH-3-4) );
    
    buffer[2] = (u8)calcSum (&buffer[3], (DATA_LENGTH-3) );
    
//    debug_msg_show_hex (buffer, DATA_LENGTH, seq);
    if (IsChinaMobileCard())
        GsmTcpSendData (buffer, DATA_LENGTH);
    else
        GsmCNCSendData (buffer, DATA_LENGTH);
}

u16 bufLen = 0;

u16 RcvCnt = 0;
u32 tm_cntT2;
void DataHandle ( unsigned char dat)
{
    static u8  RcvSum, step = 0;
    static u32 CalcSum = 0;
    static u16 RcvLength, CalcLength = 0;

    switch (step)
    {
        case 0:
            if (dat == 0x03)
            {
                step = 3;
                RcvLength = 0;
                RcvSum = 0;
                CalcLength = 0;
                CalcSum = 0;
            }
            break;
//        case 2:
//            if (dat == 0x00)
//            {
//                step ++;
//                RcvLength = 0;
//                RcvSum = 0;
//                CalcLength = 0;
//                CalcSum = 0;
//            }
//            break;
        case 3:
            RcvLength = dat;
            step ++;
            break;
        case 4:
            RcvSum = dat;
            step ++;
            break;
        case 5:
            CalcLength ++;
            CalcSum += dat;
            if (CalcLength == RcvLength)
            {
                if ((CalcSum&0xFF) == RcvSum)
                {
                    APP_GSM_MSG("\r\n==>Rev_Ser_Suc\r\n");
                }
                else
                    APP_GSM_MSG("\r\n==>Rev_Ser_Err:CheckSum\r\n");
                
                NB_Send_TestData();
                step = 0;
                RcvCnt ++;
                tm_cntT2 = 0;
            }
            else if (CalcLength > 255)
                step = 0;
            break;
        default: 
            step = 0; 
        break;   
    }
    
}


