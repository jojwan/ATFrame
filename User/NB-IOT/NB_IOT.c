#include "NB_IOT.h"
#include "bsp_usart.h"
#include <string.h>
#include "ATHandleFrame.h"

#ifdef  USE_OS
extern uint8_t AppTaskPend  (uint32_t timeout);
extern void    AppTaskDelay (uint32_t tm);
extern void    GsmTaskPost  (void);
extern void    GsmTmrStart  (void);
extern void    GsmTmrStop   (void);
#else
uint8_t AppTaskPend  (uint32_t timeout) {return 0;};
void    AppTaskDelay (uint32_t tm) {};
void    GsmTaskPost  (void) {};
void    GsmTmrStart  (void) {};
void    GsmTmrStop   (void) {};
#endif

void SetATCMDVsRom (uint16_t timeout, const char *fmt, ...);
void intToHexStr(uint8_t * src, uint8_t * des, uint16_t len);
    
#define STATIC  static
#define  FORCE_CONNECT_CNIOT     1u

//#define  GSM_DebugMsg( fmt, ... )           Uart_Send_String( fmt, ##__VA_ARGS__ ) 
#define  GSM_DebugMsg         DebugMsg

#define  GSM_CmdStrTx( fmt, ... )           Uart_Send_String(UART4, fmt, ##__VA_ARGS__)  //UART3_Send_String( fmt, ##__VA_ARGS__ ) 
#define  GSM_UartTx                         
#define  GSM_UartTxByte(x)                  

//#define  AT_STAGE_MSG( fmt, ... )           Uart_Send_String( fmt, ##__VA_ARGS__ ) 
#define  AT_STAGE_MSG( fmt, ... )           DebugMsg

#define  ELSE_TASK_PEND()
#define  GsmTaskPend                        //AppTaskPend

//const static char   Q_At[]		=	{"AT\r\n"};        // 测试AT指令是否响应
const static char   Q_Pin[]		=	{"AT+CPIN?\r\n"};  // 检查SIM卡状态
const static char   Q_Csq[]		=	{"AT+CSQ\r\n"};    // 检查射频信号
const static char   Q_GReg[]    =	{"AT+CGREG?\r\n"}; // 查询PS网络附着
const static char   Q_GAct[]    =	{"AT+CGACT?\r\n"}; // PDN激活成功
const static char   Q_Cops[]    =	{"AT+COPS?\r\n"};  // 查询网络信息，运营商及网络制式，9即 NB-IOT 网络
const static char   Q_Gcontrdp[]=	{"AT+CGCONTRDP\r\n"}; // 查询网络下发 APN 和分配的 IP 地址

//初始化指令
const static char ATInit[][32]	=	{"AT\r\n",
									"ATZ\r\n",
                                    "ATE1&W\r\n",     // 开回显并保存设置
                                    "AT+CCID\r\n",  
                                    "AT+CIMI\r\n",    // IMSI
            						"ATE0&W\r\n",     // 关回显 SET COMMAND ECHO MODE   0 Echo mode off  
                                    "AT+CGSN\r\n",    // IMEI
                                    "AT+CFUN=1\r\n",
#if  ONENET_PLATFORM_ENABLE > 0u  
                                    "AT+MIPLBOOTSTRAPPARA=0,1\r\n",
#endif  
            						"\0"};

#define IP_ADDR        "180.101.147.115"
#define IP_COM         "5683"
#define DEVICE_ID      "1112223334447"
#define INTERNEL_TIME  "100"

/*ChinaNet Cloud AT cmd*/
                                        // 开始连接电信云，参数包括电信云 IP ,电信云端口，模块 ID , 
                                        //状态更新时间
const static char   AT_ClinNew[]    =	{"AT+CM2MCLINEW=\""IP_ADDR"\",\""IP_COM"\",\""DEVICE_ID"\""\
                                         ","INTERNEL_TIME"\r\n"}; 

const static char   AT_CliSend[]    =	{"AT+CM2MCLISEND=\""}; // 发送16进制数据，长度必须为偶数
                                                               //例如： AT+CM2MCLISEND="313233343536"
const static char   AT_STATUS[]    =	{"0"};    // 形式

#if 0
const static char   AT_CliRecv[]    =	{"+CM2MCLIRECV: 313233343536"}; // 收到服务器下发的数据(16进制输出)
#endif

typedef struct _ObjectType
{
    u32 IdCreateRsp;
    u32 IdObserveMsg;
    u32 IdObject;
    u32 IdInstance;
    u32 IdResource[1];
}ObjectType;

ObjectType TestObj = {0, 0, 3303, 1, 0};

/*OnetNET Cloud AT cmd*/
#if  ONENET_PLATFORM_ENABLE > 0u  
//const static char   AT_SetNoBootstrap[]    =	{"AT+MIPLBOOTSTRAPPARA=0,1\r\n"}; 
const static char   AT_MiplCreate[]        =    {"AT+MIPLCREATE=51,130033f10003f2002105"\
                                                  "0011000000000000000d3138332e3233302e3"\
                                                  "4302e343000044e554c4cf3000cea04000004"\
                                                  "4e554c4c,0,51,0\r\n"};
const static char   AT_MiplAddObj[]        =	{"AT+MIPLADDOBJ=%d,3303,1,1,7,3\r\n"};
const static char   AT_MiplOpen[]          =	{"AT+MIPLOPEN=%d,70,30\r\n"};
const static char   AT_MiplOberserveRsp[]  =	{"AT+MIPLOBSERVERSP=%d,%d,1\r\n"};
const static char   AT_MiplDiscoverRsp[]   =	{"AT+MIPLDISCOVERRSP=%d,%d,1,34,\"5700;"\
                                                  "5701;5601;5602;5603;5604;5605\"\r\n"};
const static char   AT_MiplNotify[]        =	{"AT+MIPLNOTIFY=%d,%d,3303,0,5700,4,2,%d,0,0\r\n"};

#endif

#define GSM_DATA_LENGTH         512
STATIC Gsm_Stage                Gsm_At_Stage = AT_GSM_NONE;
STATIC uint8_t                  Gsm_At_Retry = 0;
STATIC uint32_t                 Gsm_At_Time = 0;
//STATIC 
Gsm_Control_Union	    Gsm_At_Flag = {0};
STATIC Gsm_REC_Process_Stage    GSM_Recv_Sta = RECV_STAGE_DEF;
//STATIC uint8_t	                ATSWInitCont = 0;
STATIC Gsm_Status_Type          Gsm_Status = {0};
STATIC uint8_t                  GsmDataBuffer[GSM_DATA_LENGTH] = {0};
#define  SIM_CCID_CHAR_LENGTH    20  
STATIC u8                       SimCcidCh[SIM_CCID_CHAR_LENGTH+1] = {0};
#define  SIM_IMSI_CHAR_LENGTH    16    
STATIC u8                       SimImsiCh[SIM_IMSI_CHAR_LENGTH+1] = {0};  // IMSI 由字符或数字组成的字符串，最多不超过16个字符
#define  SIM_IMEI_CHAR_LENGTH    15  
STATIC u8                       SimImeiCh[SIM_IMEI_CHAR_LENGTH+1] = {0};  // IMEI 由数字组成的字符串，长度15个字符
#define  SIM_STR_LENGTH          20
STATIC u8                       RecBufTemp[SIM_STR_LENGTH+1] = {0}; // 接收AT命令回复的缓存
#define SO_RCV_TEMP   (*(u32 *)&RecBufTemp[0])
#define SO_RCV_LEN    (*(u32 *)&RecBufTemp[4])
#define SO_RCV_CNT    (*(u32 *)&RecBufTemp[8])
#define SO_RCV_ERR    (*(u32 *)&RecBufTemp[12])
#define SO_RCV_BYTE   (*(u8 *)&RecBufTemp[16])
STATIC u8                       RecBufLen = 0;
enum {NOCARD = 0, CHINA_MOBILE, CHINA_NET};
STATIC u8                       SimCardType = NOCARD;
STATIC u8                       IdCreateRsp = 0;   // AT+MIPLCREATE=... 的返回值
STATIC u32                      IdDiscoverMsg = 0;
STATIC u8                       GsmCsqValue = 0;
STATIC u8                       NewestSocketId = 0;

#define RECV_STAGE_JOY_ENABLE  0u
#if RECV_STAGE_JOY_ENABLE > 0u
STATIC u8 *pDest;
STATIC u8  DestStringLen;
STATIC Gsm_REC_Process_Stage NextStep;
STATIC u8  CurDoLen;
void AtuoCompareString (u8 *pDst, u8 StrLen, Gsm_REC_Process_Stage step)
{
    CurDoLen = 0;
    pDest = pDst;
    DestStringLen = StrLen;
    NextStep = step;
}
#endif

STATIC u8 IsRevEn_OK(void)   {return (Gsm_At_Flag.bit.OK);}
STATIC u8 IsRevEn_Cpin(void) {return (Gsm_At_Flag.bit.OK && Gsm_At_Flag.bit.PinReady);}
STATIC u8 IsRevEn_CSQ(void)  {return (Gsm_At_Flag.bit.OK && Gsm_At_Flag.bit.CSQ_En 
	                                    && (GsmCsqValue > 7));}
STATIC u8 IsRevEn_Reg(void)  {return (Gsm_At_Flag.bit.OK && Gsm_At_Flag.bit.Greg);}
STATIC u8 IsRevEn_Gact(void)  {return (Gsm_At_Flag.bit.OK && Gsm_At_Flag.bit.Gact);}
STATIC u8 IsRevEn_Cops(void)  {return (Gsm_At_Flag.bit.OK && Gsm_At_Flag.bit.Cops);}
STATIC u8 IsRevEn_ClinNew(void)  {return (Gsm_At_Flag.bit.OK && Gsm_At_Flag.bit.m2mcli_1 
	                                       && Gsm_At_Flag.bit.m2mcli_4);}
STATIC u8 IsRevEn_Status(void)  {return (Gsm_Status.bit.WaitSend && Gsm_Status.bit.connect);}
STATIC u8 IsRevEn_CliSend(void)  {return (Gsm_At_Flag.bit.OK && Gsm_At_Flag.bit.m2mcli_5);}

u8 IsRevTimeout(void)  {return (Gsm_At_Flag.bit.timeout);}

// 发送命令的序号
enum  _GSM_STAGE_PROC
{
AT_Q_AT,
AT_Q_ATZ,
AT_Q_ATE1_W,    
AT_Q_CCID,  
AT_Q_CIMI,   
AT_Q_ATE0_W,    
AT_Q_CGSN,   
AT_Q_CFUN,
    
AT_Q_Pin,	
AT_Q_Csq,		
AT_Q_GReg,  
AT_Q_GAct,  
AT_Q_Cops,  
AT_Q_Gcontrdp,

AT_Q_ClinNew,
AT_Q_CliSend,

AT_Q_SOSENDFLAG,
AT_Q_SOC,
AT_Q_SOCON,
AT_Q_SOSEND,

AT_Q_STATUS,
};

STATIC void DoTrue_CSQ(u8 *pNextCmd)  
{
	if (Gsm_Status.bit.connect)
		*pNextCmd = AT_Q_STATUS;
	else
		*pNextCmd = AT_Q_GReg;
}

STATIC void DoTrue_Reg(u8 *pNextCmd)  
{
	if (Gsm_Status.bit.connect)
		*pNextCmd = AT_Q_STATUS;
	else
		*pNextCmd = AT_Q_GAct;
}

STATIC void ATCmdSend_Status (u8 *pNextCmd)  
{
	if (Gsm_Status.bit.connect && Gsm_Status.bit.WaitSend)
	{
        // 在 wait 步骤跳到 AT_Q_CliSend
	}
	else
	{
		SetGsmTime(5000);
	}
}

STATIC void DoTrue_Status (u8 *pNextCmd)  
{
#if !(FORCE_CONNECT_CNIOT > 0u)
    if (SimCardType == CHINA_MOBILE)
        *pNextCmd = AT_Q_SOSEND;
    else
#endif
        *pNextCmd = AT_Q_CliSend;
}

STATIC void DoTimeout_Status (u8 *pNextCmd)  
{
	if (Gsm_Status.bit.gapQ)
		*pNextCmd = AT_Q_Csq;
	else 
		*pNextCmd = AT_Q_GReg;
	Gsm_Status.bit.gapQ = ~Gsm_Status.bit.gapQ;
}

STATIC void DoTrue_ClinNew(u8 *pNextCmd)
{
	*pNextCmd = AT_Q_STATUS;
	Gsm_Status.bit.connect = 1;
}
uint8_t IsChinaMobileCard(void)
{
    return (SimCardType == CHINA_MOBILE);
}
STATIC void DoTrue_ChoseDir(u8 *pNextCmd)
{
#if !(FORCE_CONNECT_CNIOT > 0u)
    if (SimCardType == CHINA_MOBILE)
        *pNextCmd = AT_Q_SOSENDFLAG;
    else
#endif
        *pNextCmd = AT_Q_ClinNew;
}

//STATIC void DoTrue_GotoStatus (u8 *pNextCmd)
//{
//	*pNextCmd = AT_Q_STATUS;
//}

STATIC void DoTrue_CliSend (u8 *pNextCmd)
{
	*pNextCmd = AT_Q_STATUS;
    Gsm_Status.bit.WaitSend = 0;
}

STATIC void DoTimeout_CliSend (u8 *pNextCmd)
{
	Gsm_Status.bit.connect = 0;
}

void ConfirmRsp_OK (void *p)      {Gsm_At_Flag.bit.OK=1;}
void ConfirmRsp_Ready (void *p)   {Gsm_At_Flag.bit.PinReady=1;}
void ConfirmRsp_CSQ (void *p)     {Gsm_At_Flag.bit.CSQ_En=1;}
void ConfirmRsp_GREG (void *p)    {Gsm_At_Flag.bit.Greg=1;}
void ConfirmRsp_GACT (void *p)    {Gsm_At_Flag.bit.Gact=1;}
void ConfirmRsp_COPS (void *p)    {Gsm_At_Flag.bit.Cops=1;}
void ConfirmRsp_SoCreat (void *p) {Gsm_At_Flag.bit.SoCreate=1;}
void ConfirmRsp_SoRcvDat (void *p){Gsm_At_Flag.bit.SoRcvDat=1;}
void ConfirmRsp_SOERR (void *p)   {Gsm_At_Flag.bit.SoERR=1;}
void ConfirmRsp_SEND (void *p)    {Gsm_At_Flag.bit.SoSend=1;}

u8 ProcessData_CSQ (u8 data, u8 isOver, u8 cnt)
{
    if (cnt == 1)
    {
        if (isOver)
        {
            GsmCsqValue = *(u32 *)&RecBufTemp[0];
            *(u32 *)&RecBufTemp[0] = 0;
        }
        else
        {
            *(u32 *)&RecBufTemp[0] = *(u32 *)&RecBufTemp[0]*10 + (data-0x30);
        }
        if (*(u32 *)&RecBufTemp[0] > 99)
        {
            *(u32 *)&RecBufTemp[0] = 0;
            return 1;  // return error
        }
        return 0;
    }
    else if (cnt == 2)
    {
        if (isOver)
        {
//             = *(u32 *)&RecBufTemp[0];
            *(u32 *)&RecBufTemp[0] = 0;
        }
        else
        {
            *(u32 *)&RecBufTemp[0] = *(u32 *)&RecBufTemp[0]*10 + (data-0x30);
        }
        if (*(u32 *)&RecBufTemp[0] > 7)
        {
            *(u32 *)&RecBufTemp[0] = 0;
            return 1;  // return error
        }
        return 0;
    }
    return 1; // return error
}

/*
处理服务器下发的应用数据
cnt=1 ： 数据长度
cnt=2 :  具体数据
*/

u8 ProcessData_SoRcvDat (u8 data, u8 isOver, u8 cnt)
{
    switch (cnt)
    {
        case 1:
        {
            if (isOver)
            {
                // 实际测试最多连4个socket
                // 只允许创建 0-9 的socket id
                if (NewestSocketId == SO_RCV_TEMP)
                {
                    SO_RCV_TEMP = 0;
                    SO_RCV_LEN  = 0;
                    SO_RCV_CNT  = 0;
                    SO_RCV_BYTE = 0;
                    return 0;
                }
                else
                    return 1;
            }
            else
            {
                SO_RCV_TEMP = data-0x30;
            }
            if (SO_RCV_TEMP > 9)
            {
                SO_RCV_TEMP = 0;
                return 1;  // return error
            }
        }
        break;
        case 2:
        {
            if (isOver)
            {
                // SO_RCV_LEN 保存接收的socket数据的长度
                SO_RCV_LEN = SO_RCV_TEMP;
                SO_RCV_TEMP = 0;
                GSM_DebugMsg("[DATA]:Socket_Rcv_Len: %d\r\n", SO_RCV_LEN);
            }
            else
            {
                SO_RCV_TEMP = SO_RCV_TEMP*10 + (data-0x30);
            }
            if (SO_RCV_TEMP > 999)
            {
                SO_RCV_TEMP = 0;
                return 1;  // return error
            }
        }
        break;
        case 3:
        {
            if (isOver && SO_RCV_CNT == SO_RCV_LEN)
            {
                GSM_DebugMsg("\r\n[DATA]:Socket_Rcv: SUC\r\n");
                return 0;
            }
            else if (SO_RCV_CNT > SO_RCV_LEN)
            {
                GSM_DebugMsg("\r\n[DATA]:Socket_Rcv: ERR_LEN\r\n");
                return 1;  // return error
            }
            SO_RCV_CNT ++;
            if (SO_RCV_CNT%2 == 0)
            {
                SO_RCV_BYTE *= 0x10;
            }
            if (data >= '0' && data <= '9')
                SO_RCV_BYTE += data - '0';
            else if (data >= 'a' && data <= 'f')
                SO_RCV_BYTE += data - 'a' + 10;
            else if (data >= 'A' && data <= 'F')
                SO_RCV_BYTE += data - 'A' + 10;
            else
            {
                return 1;  // return error
            }
            if (SO_RCV_CNT%2 == 0)
            {
//                GSM_DebugMsg("%02X ", SO_RCV_BYTE);
//                handle (SO_RCV_BYTE);
                SO_RCV_BYTE = 0;
            }

        }
        break;
        default: return 1; // return error
    }
    return 0; // return error
}

u8 ProcessData_ERR (u8 data, u8 isOver, u8 cnt)
{
    switch (cnt)
    {
        case 1:
        {
            if (isOver)
            {
                // 只允许创建 0-9 的socket id
                if (NewestSocketId == SO_RCV_TEMP)
                {
                    SO_RCV_TEMP = 0;
                    SO_RCV_LEN  = 0;
                    SO_RCV_CNT  = 0;
                    SO_RCV_BYTE = 0;
                    return 0;
                }
            }
            else
            {
                SO_RCV_TEMP = (data-0x30);
            }
            if (SO_RCV_TEMP > 9)
            {
                 SO_RCV_TEMP = 0;
                return 1;  // return error
            }
        }
        break;
        case 2:
        {
            if (isOver)
            {
                // SO_RCV_LEN 保存接收的socket数据的长度
                SO_RCV_ERR = SO_RCV_TEMP;
                SO_RCV_TEMP = 0;
                GSM_DebugMsg("[ERR]:Socket_Err %d: %d\r\n", NewestSocketId, SO_RCV_ERR);
            }
            else
            {
                SO_RCV_TEMP = SO_RCV_TEMP*10 + (data-0x30);
            }
            if (SO_RCV_TEMP > 99)
            {
                SO_RCV_TEMP = 0;
                return 1;  // return error
            }
        }
        break;
        default: return 1; // return error
    }
    return 0; // return error
}

u8 ProcessData_SoCreat (u8 data, u8 isOver, u8 cnt)
{
    if (cnt == 1)
    {
        if (isOver)
        {
            NewestSocketId = SO_RCV_TEMP;
            SO_RCV_TEMP = 0;
        }
        else
        {
            SO_RCV_TEMP = SO_RCV_TEMP*10 + (data-0x30);
        }
        if (SO_RCV_TEMP > 9)
        {
            SO_RCV_TEMP = 0;
            return 1;  // return error
        }
        return 0;
    }
    return 1; // return error
}

u8 ProcessData_Cops (u8 data, u8 isOver, u8 cnt)
{
    if (cnt == 1)
    {
        if (isOver)
        {
            if (SO_RCV_TEMP == 0)
                SimCardType = CHINA_MOBILE;
            else if (SO_RCV_TEMP == 11)
                SimCardType = CHINA_NET;
            else
                ;
            SO_RCV_TEMP = 0;
        }
        else
        {
            SO_RCV_TEMP = SO_RCV_TEMP*10 + (data-0x30);
        }
        if (SO_RCV_TEMP > 99)
        {
            SO_RCV_TEMP = 0;
            return 1;  // return error
        }
        return 0;
    }
    return 1; // return error
}

/*
AT+CSOSEND=<socket_id>,<data_len>,<data>
If CSOSENDFLAG is 1 and socket type is TCP.
OK
SEND: <socket_id>,<leN>
发送数据后会受到回执
*/
u8 ProcessData_SEND (u8 data, u8 isOver, u8 cnt)
{
    //SEND: 0,6
    switch (cnt)
    {
        case 1:
        {
            if (isOver)
            {
                // 只允许创建 0-9 的socket id
                if (NewestSocketId == SO_RCV_TEMP)
                {
                    SO_RCV_TEMP = 0;
                    SO_RCV_LEN  = 0;
                    SO_RCV_CNT  = 0;
                    SO_RCV_BYTE = 0;
                    return 0;
                }
            }
            else
            {
                SO_RCV_TEMP = (data-0x30);
            }
            if (SO_RCV_TEMP > 9)
            {
                 SO_RCV_TEMP = 0;
                return 1;  // return error
            }
        }
        break;
        case 2:
        {
            if (isOver)
            {
                // SO_RCV_LEN 保存接收的socket数据的长度
                SO_RCV_LEN = SO_RCV_TEMP;
                SO_RCV_TEMP = 0;
                GSM_DebugMsg("[DATA]:Socket_SEND %d: %d\r\n", NewestSocketId, SO_RCV_LEN);
            }
            else
            {
                SO_RCV_TEMP = SO_RCV_TEMP*10 + (data-0x30);
            }
            if (SO_RCV_TEMP > 99)
            {
                SO_RCV_TEMP = 0;
                return 1;  // return error
            }
        }
        break;
        default: return 1; // return error
    }
    return 0; // return error
}

u8 ProcessData_M2MCLI (u8 data, u8 isOver, u8 cnt)
{
    if (cnt == 1)
    {
        if (isOver)
        {
            switch (SO_RCV_TEMP)
            {
                case 1: Gsm_At_Flag.bit.m2mcli_1 = 1; break;
                case 2: Gsm_At_Flag.bit.m2mcli_2 = 1; break;
                case 4: Gsm_At_Flag.bit.m2mcli_4 = 1; break;
                case 5: Gsm_At_Flag.bit.m2mcli_5 = 1; break;
                default:break;
            }
            SO_RCV_TEMP = 0;
        }
        else
        {
            SO_RCV_TEMP = SO_RCV_TEMP*10 + (data-0x30);
        }
        if (SO_RCV_TEMP > 9)
        {
            SO_RCV_TEMP = 0;
            return 1;  // return error
        }
        return 0;
    }
    return 1; // return error
}

enum
{
AT_RSP_OK,
AT_RSP_READY,
AT_RSP_CSQ,
AT_RSP_CGREG,
AT_RSP_CGACT,
AT_RSP_COPS,
AT_RSP_CSOCREAT,
AT_RSP_CSORCVDAT,
AT_RSP_CSOERR,
AT_RSP_SOSEND,
AT_RSP_M2MCLI
};

ATFrameStrMem GsmTcpATFrameStr = {0};

AtFrameStringProcess GsmTcp[] =
{
{NULL,   "OK\r\n",                     ConfirmRsp_OK,           NULL, },
{NULL,   "+CPIN: READY\r\n",           ConfirmRsp_Ready,        NULL, },
{NULL,   "+CSQ: *,*\r\n",              ConfirmRsp_CSQ,          ProcessData_CSQ, },  
{NULL,   "+CGREG: 0,1\r\n",            ConfirmRsp_GREG,         NULL, },
{NULL,   "+CGACT: 1,1\r\n",            ConfirmRsp_GACT,         NULL, },
{NULL,   "+COPS: 0,2,\"460*\",9\r\n",  ConfirmRsp_COPS,         ProcessData_Cops, },
{NULL,   "+CSOC: *\r\n",               ConfirmRsp_SoCreat,      ProcessData_SoCreat, },
{NULL,   "+CSONMI: *,*,*\r\n",         ConfirmRsp_SoRcvDat,     ProcessData_SoRcvDat,     1,},
{NULL,   "+CSOERR: *,*\r\n",           ConfirmRsp_SOERR,        ProcessData_ERR,   },
{NULL,   "SEND: *,*\r\n",              ConfirmRsp_SEND,         ProcessData_SEND,  },
{NULL,   "+CM2MCLI: *\r\n",            NULL,                    ProcessData_M2MCLI,  1},
};

void InitStrProcLeng (void)
{
    int i = 0;
    for (i = 0; i < (sizeof(GsmTcp)/sizeof(GsmTcp[0])); i ++)
        {
            GsmTcp[i].AtRspTargetLen = strlen(GsmTcp[i].AtRspTarget);
        }
}
void InitStrProcNode (void)
{
    int i = 0;
    for (i = 0; i < (sizeof(GsmTcp)/sizeof(GsmTcp[0])); i ++)
        {
            GsmTcp[i].pNext = NULL;
        }
}

void InitStrProcInserAllNode (void)
{
    int i = 0;
    for (i = 0; i < (sizeof(GsmTcp)/sizeof(GsmTcp[0])); i ++)
        {
            InsertProcNode(&GsmTcpATFrameStr.pHead, &GsmTcp[i]); 
        }
}

const static char   AT_SocketSndFlg[]    =	{"AT+CSOSENDFLAG=1\r\n"}; 
const static char   AT_SocketCreate[]    =	{"AT+CSOC=1,1,1\r\n"}; 
const static char   AT_SocketConnect[]   =	{"AT+CSOCON=%d,8097,\"125.71.236.245\"\r\n"}; 
//const static char   AT_SocketSend[]      =	{"AT+CSOSEND=%d,%d,%s\r\n"}; 

STATIC u8 IsRevEn_SoCreat(void)  {return (Gsm_At_Flag.bit.OK && Gsm_At_Flag.bit.SoCreate);}
STATIC u8 IsRevEn_SoSnd(void)    {return (Gsm_At_Flag.bit.OK && Gsm_At_Flag.bit.SoSend);}
STATIC void DoTrue_SoConnect(u8 *pNextCmd)
{
	*pNextCmd = AT_Q_STATUS;
	Gsm_Status.bit.connect = 1;
}
STATIC void DoTrue_SoSnd(u8 *pNextCmd)
{
	*pNextCmd = AT_Q_STATUS;
    Gsm_Status.bit.WaitSend = 0;
}

uint8_t GsmTcpSendData (uint8_t *buf,uint16_t length)
{
    if (Gsm_Status.bit.WaitSend)
        return 0;
    if ( (sizeof("AT+CSOSEND=0,,\r\n") + length*2 + 4) > GSM_DATA_LENGTH - 1 )
        return 0;
    memset (GsmDataBuffer, 0, GSM_DATA_LENGTH);
    // AT+CSOSEND=0,%d,%s\r\n
    
    sprintf ((char *)GsmDataBuffer, "AT+CSOSEND=%d,%d,", NewestSocketId, length*2);
    intToHexStr (buf, &GsmDataBuffer[strlen((char *)GsmDataBuffer)], length);
    memcpy (&GsmDataBuffer [strlen((char *)GsmDataBuffer)], "\r\n\0", sizeof ("\r\n\0"));
    
    Gsm_Status.bit.WaitSend = 1;
    GsmTaskPost ();
    return 1;
}

STATIC void ATCmdSend_SoCon (u8 *pNextCmd)
{
    SetATCMDVsRom (3000, AT_SocketConnect, NewestSocketId);
}

void SetReq_Init (void) 
{ 
    InitStrProcNode ();
    GsmTcpATFrameStr.pHead = NULL;
    
    InsertProcNode(&GsmTcpATFrameStr.pHead, &GsmTcp[AT_RSP_OK]); 
}

void SetReq_OK (void) { InsertProcNode(&GsmTcpATFrameStr.pHead, &GsmTcp[AT_RSP_OK]); }
void SetReq_CPin (void) 
{ 
    InsertProcNode(&GsmTcpATFrameStr.pHead, &GsmTcp[AT_RSP_OK]); 
    InsertProcNode(&GsmTcpATFrameStr.pHead, &GsmTcp[AT_RSP_READY]); 
}
void SetReq_CSQ (void) 
{
    SO_RCV_TEMP = 0;
    InsertProcNode(&GsmTcpATFrameStr.pHead, &GsmTcp[AT_RSP_OK]); 
    InsertProcNode(&GsmTcpATFrameStr.pHead, &GsmTcp[AT_RSP_CSQ]); 
}
void SetReq_Reg (void) 
{
    SO_RCV_TEMP = 0;
    InsertProcNode(&GsmTcpATFrameStr.pHead, &GsmTcp[AT_RSP_OK]); 
    InsertProcNode(&GsmTcpATFrameStr.pHead, &GsmTcp[AT_RSP_CGREG]); 
}
void SetReq_Gact (void) 
{
    SO_RCV_TEMP = 0;
    InsertProcNode(&GsmTcpATFrameStr.pHead, &GsmTcp[AT_RSP_OK]); 
    InsertProcNode(&GsmTcpATFrameStr.pHead, &GsmTcp[AT_RSP_CGACT]); 
}
void SetReq_Cops (void) 
{
    SO_RCV_TEMP = 0;
    InsertProcNode(&GsmTcpATFrameStr.pHead, &GsmTcp[AT_RSP_OK]); 
    InsertProcNode(&GsmTcpATFrameStr.pHead, &GsmTcp[AT_RSP_COPS]); 
}
void SetReq_SoCreat (void) 
{ 
    Gsm_Status.bit.connect = 0;
    SO_RCV_TEMP = 0;
    InsertProcNode(&GsmTcpATFrameStr.pHead, &GsmTcp[AT_RSP_OK]); 
    InsertProcNode(&GsmTcpATFrameStr.pHead, &GsmTcp[AT_RSP_CSOCREAT]); 
    InsertProcNode(&GsmTcpATFrameStr.pHead, &GsmTcp[AT_RSP_CSORCVDAT]); 
}
void SetReq_SoSnd (void) 
{ 
    InsertProcNode(&GsmTcpATFrameStr.pHead, &GsmTcp[AT_RSP_OK]); 
    InsertProcNode(&GsmTcpATFrameStr.pHead, &GsmTcp[AT_RSP_SOSEND]); 
}
//void SetReq_SoRcv (void) 
//{
//    InsertProcNode(&GsmTcpATFrameStr.pHead, &GsmTcp[AT_RSP_CSORCVDAT]); 
//}
void SetReq_CliNew (void) 
{ 
    InsertProcNode(&GsmTcpATFrameStr.pHead, &GsmTcp[AT_RSP_OK]); 
    InsertProcNode(&GsmTcpATFrameStr.pHead, &GsmTcp[AT_RSP_M2MCLI]); 
}
void SetReq_CliSnd (void) 
{ 
    InsertProcNode(&GsmTcpATFrameStr.pHead, &GsmTcp[AT_RSP_OK]); 
//    InsertProcNode(&GsmTcpATFrameStr.pHead, &GsmTcp[AT_RSP_M2MCLI]); //在SetReq_CliNew()中添加
}

AtFrameStage  GsmWholeStageProc[] =
{
//  DebugMsg         AtCmdStr         SendCmdFun            SetReqFun           IsTrueFun            DoTrueFun              IsFalseFun    DoFalseFun   DoTimeoutFun       Timeout RetryTime
    {NULL,           "AT\r\n",        NULL,                 SetReq_OK,          IsRevEn_OK,          NULL,                  NULL,         NULL,        NULL,              1000,   3},
    {NULL,           "ATZ\r\n",       NULL,                 SetReq_OK,          IsRevEn_OK,          NULL,                  NULL,         NULL,        NULL,              1000,   3},
    {NULL,           "ATE1&W\r\n",    NULL,                 SetReq_OK,          IsRevEn_OK,          NULL,                  NULL,         NULL,        NULL,              1000,   3},
    {NULL,           "AT+CCID\r\n",   NULL,                 SetReq_OK,          IsRevEn_OK,          NULL,                  NULL,         NULL,        NULL,              1000,   3},  
    {NULL,           "AT+CIMI\r\n",   NULL,                 SetReq_OK,          IsRevEn_OK,          NULL,                  NULL,         NULL,        NULL,              1000,   3},
    {NULL,           "AT+CGSN\r\n",   NULL,                 SetReq_OK,          IsRevEn_OK,          NULL,                  NULL,         NULL,        NULL,              1000,   3},
    {NULL,           "ATE0&W\r\n",    NULL,                 SetReq_OK,          IsRevEn_OK,          NULL,                  NULL,         NULL,        NULL,              1000,   3},
    {NULL,           "AT+CFUN=1\r\n", NULL,                 SetReq_OK,          IsRevEn_OK,          NULL,                  NULL,         NULL,        NULL,              1000,   3},
    {"AT_Pin",       Q_Pin,           NULL,                 SetReq_CPin,        IsRevEn_Cpin,        NULL,                  NULL,         NULL,        NULL,              1000,   3},
	{"AT_Csq",       Q_Csq,           NULL,                 SetReq_CSQ,         IsRevEn_CSQ,         DoTrue_CSQ,            NULL,         NULL,        NULL,              1000,   20},
	{"AT_GReg",      Q_GReg,          NULL,                 SetReq_Reg,         IsRevEn_Reg,         DoTrue_Reg,            NULL,         NULL,        NULL,              1000,   20},
	{"AT_GAct",      Q_GAct,          NULL,                 SetReq_Gact,        IsRevEn_Gact,        NULL,                  NULL,         NULL,        NULL,              1000,   3},
	{"AT_Cops",      Q_Cops,          NULL,                 SetReq_Cops,        IsRevEn_Cops,        NULL,                  NULL,         NULL,        NULL,              1000,   3},
	{"AT_Gcontrdp",  Q_Gcontrdp,      NULL,                 SetReq_OK,          IsRevEn_OK,          DoTrue_ChoseDir,       NULL,         NULL,        NULL,              1000,   3},
	// China Net Cloud iot
    {"AT_ClinNew",   AT_ClinNew,      NULL,                 SetReq_CliNew,      IsRevEn_ClinNew,     DoTrue_ClinNew,        NULL,         NULL,        NULL,              30000,  3},
	{"AT_CliSend",  (const char *)GsmDataBuffer,    NULL,   SetReq_CliSnd,      IsRevEn_CliSend,     DoTrue_CliSend,        NULL,         NULL,        DoTimeout_CliSend, 8000,   5},
    // China Mobile TCP
    {"AT_CSOSENDFLAG",AT_SocketSndFlg, NULL,                SetReq_OK,          IsRevEn_OK,          NULL,                  NULL,         NULL,        NULL,              3000,   3},
    {"AT_SOCREAT",   AT_SocketCreate,  NULL,                SetReq_SoCreat,     IsRevEn_SoCreat,     NULL,                  NULL,         NULL,        NULL,              10000,  3},
    {"AT_SOCON",     AT_SocketConnect, ATCmdSend_SoCon,     SetReq_OK,          IsRevEn_OK,          DoTrue_SoConnect,      NULL,         NULL,        NULL,              3000,  3},
    {"AT_SOSEND",    (const char*)GsmDataBuffer, NULL,      SetReq_SoSnd,       IsRevEn_SoSnd,       DoTrue_SoSnd,          NULL,         NULL,        NULL,              30000,  3},    
    
    {"AT_Status",    AT_STATUS,       ATCmdSend_Status,     NULL,               IsRevEn_Status,      DoTrue_Status,         NULL,         NULL,        DoTimeout_Status,  5000,   3},
    
    {NULL},
};

/*===========================================================================*/

/**********************************************************************
** 发送AT指令，同时设置超时时间
**********************************************************************/
//STATIC 
void SetATCMDRom (const char *cmd, uint16_t timeout)
{
	GSM_CmdStrTx((char*)cmd);
	GSM_DebugMsg("[CMD]:%s\r\n", (char*)cmd);
    SetGsmTime(timeout);
}

#include <stdarg.h>
void SetATCMDVsRom (uint16_t timeout, const char *fmt, ...)
{
    static char GsmCmdBuf[256];
    va_list ap;
    
    va_start( ap, fmt );
    vsprintf( GsmCmdBuf, fmt, ap );
    va_end( ap );
   
	GSM_CmdStrTx(GsmCmdBuf);
	GSM_DebugMsg("[CMD]:%s\r\n", GsmCmdBuf);

    SetGsmTime(timeout);
}

void SetGsmTime(uint32_t timeout)
{
	Gsm_At_Time = timeout;
	Gsm_At_Flag.value = 0;
    Gsm_At_Flag.bit.timeout = 0;
    GsmTmrStart  ();
}

void  Gsm_Timer(uint32_t seconds)
{
	if(Gsm_At_Time > seconds)
		Gsm_At_Time -= seconds;
	else
    {
        GsmTaskPost ();
		Gsm_At_Flag.bit.timeout = 1;
        GsmTmrStop ();
    }
}

void GsmDelayTime(uint32_t	timeout)
{
    AppTaskDelay (timeout);
	Gsm_At_Flag.bit.timeout = 0;
}

void Gsm_Init (void)
{
    Gsm_At_Stage = AT_GSM_PWROFF;
    Gsm_At_Flag.value = 0;
}

void intToHexStr(uint8_t * src, uint8_t * des, uint16_t len)
{
	uint16_t i = 0;
	uint8_t tmp = 0;
	if (src == NULL || des == NULL)
		return;
	for (i = 0; i < len; i ++)
		{
		tmp = 0;
		tmp = src[i]>>4;
		if (tmp < 10)
			des[i*2] = '0' + tmp;
		else
			des[i*2] = 'A' + tmp - 10;

		tmp = src[i]&0x0F;
		if (tmp < 10)
			des[i*2+1] = '0' + tmp;
		else
			des[i*2+1] = 'A' + tmp - 10;	
		}
}
#if 0
uint8_t hexStrToInt(uint8_t * src, uint8_t * des, uint16_t len)
{
	uint16_t i = 0;
	uint8_t tmp = 0;
	if (src == NULL || des == NULL || len%2)
		return 0;
	for (i = 0; i < len; i += 2)
		{
            if (src[i] >= '0' && src[i] <= '9')
                des[i/2] = src[i] - '0';
            else if (src[i] >= 'a' && src[i] <= 'f')
                des[i/2] = src[i] - 'a' + 10;
            else if (src[i] >= 'A' && src[i] <= 'F')
                des[i/2] = src[i] - 'A' + 10;
            else
            {
                return 0;
            }
             
            i ++;
            if (src[i] >= '0' && src[i] <= '9')
                des[i/2] = des[i/2]*0x0F + src[i] - '0';
            else if (src[i] >= 'a' && src[i] <= 'f')
                des[i/2] = des[i/2]*0x0F + src[i] - 'a' + 10;
            else if (src[i] >= 'A' && src[i] <= 'F')
                des[i/2] = des[i/2]*0x0F + src[i] - 'A' + 10;
            else
            {
                return 0;
            }
		}
    return 1;
}
#endif
uint8_t GetGsmSendStatus (void)
{
    if (Gsm_Status.bit.WaitSend)
        return 0;  // 缓存发送满
    else
        return 1;  // 空闲
}

uint8_t GetGsmConnectStatus (void)
{
    return (Gsm_Status.bit.connect);
}

uint8_t GsmCNCSendData (uint8_t *buf,uint16_t length)
{
    if (Gsm_Status.bit.WaitSend)
        return 0;
    if ( (sizeof(AT_CliSend)-1 + length*2 + sizeof ("\"\r\n") ) > GSM_DATA_LENGTH - 1 )
        return 0;
    memset (GsmDataBuffer, 0, GSM_DATA_LENGTH);
    // AT+CM2MCLISEND="0002" 
    memcpy (GsmDataBuffer, AT_CliSend, sizeof (AT_CliSend));
    intToHexStr (buf, &GsmDataBuffer[sizeof (AT_CliSend)-1], length);
    memcpy (&GsmDataBuffer [sizeof(AT_CliSend)-1+length*2], "\"\r\n", sizeof ("\"\r\n"));
    Gsm_Status.bit.WaitSend = 1;
    GsmTaskPost ();
    return 1;
}

#if  ONENET_PLATFORM_ENABLE > 0u 
uint8_t GsmOneNETSendData (uint8_t tmp)
{
    if (Gsm_Status.bit.WaitSend)
        return 0;
    GsmDataBuffer[0] = tmp;
    Gsm_Status.bit.WaitSend = 1;
    GsmTaskPost ();
    return 1;
}
 
void Gsm_OneNET_Task (void)   // Connect China mobile cloud platform OneNET task
{
    switch (Gsm_At_Stage)
    {
        case AT_GSM_NONE: break;
        case AT_GSM_PWROFF: 
            AT_STAGE_MSG ("AT_GSM_PWROFF\r\n");
            Gsm_At_Stage = AT_GSM_PWROFF_WAIT;
        break;
        case AT_GSM_PWROFF_WAIT: 
            AT_STAGE_MSG ("AT_GSM_PWROFF_WAIT\r\n");
            Gsm_At_Stage = AT_GSM_PWRON;
        break;
        case AT_GSM_PWRON: 
            AT_STAGE_MSG ("AT_GSM_PWRON\r\n");
            Gsm_At_Stage = AT_GSM_PWRON_WAIT;
        break;
        case AT_GSM_PWRON_WAIT: 
            AT_STAGE_MSG ("AT_GSM_PWRON_WAIT\r\n");
            Gsm_At_Stage = AT_GSM_READY;
            Gsm_At_Retry = 0;
            ATSWInitCont = 0;
            Gsm_Status.bit.connect = 0;
        break;
        case AT_GSM_RESTART: break;
        case AT_GSM_RESTART_WAIT: break;

        case AT_GSM_READY: 
            AT_STAGE_MSG ("AT_GSM_READY: %s\r\n", ATInit[ATSWInitCont]);
            if (ATInit[ATSWInitCont][0] != 0)
            {
                SetATCMDRom (ATInit[ATSWInitCont], 3000);           
                Gsm_At_Stage = AT_GSM_READY_WAIT;
            }
            else
            {
                Gsm_At_Stage = AT_GSM_PIN;
                Gsm_At_Retry = 0;
                Gsm_At_Flag.value = 0;
            }

        break;
        case AT_GSM_READY_WAIT: 
            
            if (Gsm_At_Flag.bit.OK)
            {
                Gsm_At_Flag.value = 0;
                Gsm_At_Stage = AT_GSM_READY;
                Gsm_At_Retry = 0;
                ATSWInitCont ++;
            }
            else if (Gsm_At_Flag.bit.timeout)
            {
                Gsm_At_Retry ++;
                if (Gsm_At_Retry < 5)
                {
                    Gsm_At_Stage = AT_GSM_READY;
                    GsmDelayTime(2000);
                }
                else
                {
                    Gsm_At_Retry = 0;
                    Gsm_At_Stage = AT_GSM_PWROFF;
                }
            }
            else
                GsmTaskPend (0);
        break;
        case AT_GSM_PIN: 
            AT_STAGE_MSG ("AT_GSM_PIN\r\n");
            SetATCMDRom (Q_Pin, 1000);
            Gsm_At_Stage = AT_GSM_PIN_WAIT;        
        break;
        case AT_GSM_PIN_WAIT: 
            if (Gsm_At_Flag.bit.OK && Gsm_At_Flag.bit.PinReady) 
            {
                Gsm_At_Flag.value = 0;
                Gsm_At_Stage = AT_GSM_CSQ;
                Gsm_At_Retry = 0;
            }
            else if (Gsm_At_Flag.bit.timeout)
            {
                Gsm_At_Retry ++;
                if (Gsm_At_Retry < 3)
                {
                    Gsm_At_Stage = AT_GSM_PIN;
                    GsmDelayTime(1000);
                }
                else
                {
                    Gsm_At_Retry = 0;
                    Gsm_At_Stage = AT_GSM_PWROFF;
                }
            }
            else
                GsmTaskPend (0);
        break;
        case AT_GSM_CSQ: 
            AT_STAGE_MSG ("AT_GSM_CSQ\r\n");
            SetATCMDRom (Q_Csq, 1000);
            Gsm_At_Stage = AT_GSM_CSQ_WAIT;              
        break;
        case AT_GSM_CSQ_WAIT:
            if (Gsm_At_Flag.bit.OK && Gsm_At_Flag.bit.CSQ_En && 
                GsmCsqValue > 7)
            {
                Gsm_At_Flag.value = 0;
                if (Gsm_Status.bit.connect)
                    Gsm_At_Stage = AT_GSM_STATUS;
                else
                    Gsm_At_Stage = AT_GSM_REG;
                Gsm_At_Retry = 0;
            }
            else  if (Gsm_At_Flag.bit.timeout)
            {
                Gsm_At_Retry ++;
                if (Gsm_At_Retry < 20)
                {
                    Gsm_At_Stage = AT_GSM_CSQ;
                    GsmDelayTime(1000);
                }
                else
                {
                    Gsm_At_Retry = 0;
                    Gsm_At_Stage = AT_GSM_PWROFF;
                }
            }
            else
                GsmTaskPend (0);
        break;
        case AT_GSM_REG: 
            AT_STAGE_MSG ("AT_GSM_REG\r\n");
            SetATCMDRom (Q_GReg, 1000);
            Gsm_At_Stage = AT_GSM_REG_WAIT;
        break;
        case AT_GSM_REG_WAIT: 
            if (Gsm_At_Flag.bit.OK && Gsm_At_Flag.bit.Greg)
            {
                Gsm_At_Flag.value = 0;
                if (Gsm_Status.bit.connect)
                    Gsm_At_Stage = AT_GSM_STATUS;
                else
                    Gsm_At_Stage = AT_GSM_GACT;
                Gsm_At_Retry = 0;
            }
            else if (Gsm_At_Flag.bit.timeout)
            {
                Gsm_At_Retry ++;
                if (Gsm_At_Retry < 3)
                {
                    Gsm_At_Stage = AT_GSM_REG;
                    GsmDelayTime(1000);
                }
                else
                {
                    Gsm_At_Retry = 0;
                    Gsm_At_Stage = AT_GSM_PWROFF;
                }
            }
            else
                GsmTaskPend (0);
        break;
        case AT_GSM_GACT: 
            AT_STAGE_MSG ("AT_GSM_GACT\r\n");
            SetATCMDRom (Q_GAct, 1000);
            Gsm_At_Stage = AT_GSM_GACT_WAIT;            
        break;
        case AT_GSM_GACT_WAIT: 
            if (Gsm_At_Flag.bit.OK && Gsm_At_Flag.bit.Gact)
            {
                Gsm_At_Flag.value = 0;
                Gsm_At_Stage = AT_GSM_COPS;
                Gsm_At_Retry = 0;
            }
            else if (Gsm_At_Flag.bit.timeout)
            {
                Gsm_At_Retry ++;
                if (Gsm_At_Retry < 3)
                {
                    Gsm_At_Stage = AT_GSM_GACT;
                    GsmDelayTime(1000);
                }
                else
                {
                    Gsm_At_Retry = 0;
                    Gsm_At_Stage = AT_GSM_PWROFF;
                }
            }
            else
                GsmTaskPend (0);
        break;
        case AT_GSM_COPS: 
            AT_STAGE_MSG ("AT_GSM_COPS\r\n");
            SetATCMDRom (Q_Cops, 1000);
            Gsm_At_Stage = AT_GSM_COPS_WAIT;            
        break;
        case AT_GSM_COPS_WAIT: 
            if (Gsm_At_Flag.bit.OK && Gsm_At_Flag.bit.Cops)
            {
                if (SimCardType == CHINA_MOBILE)
                {
                    GSM_DebugMsg ("SimCard: ChinaMobile\r\n");
                }
                else
                {
                    GSM_DebugMsg ("SimCard: ChinaNet\r\n");
                }
                Gsm_At_Flag.value = 0;
                Gsm_At_Stage = AT_GSM_GCONTRDP;
                Gsm_At_Retry = 0;
            }
            else if (Gsm_At_Flag.bit.timeout)
            {
                Gsm_At_Retry ++;
                if (Gsm_At_Retry < 3)
                {
                    Gsm_At_Stage = AT_GSM_COPS;
                    GsmDelayTime(1000);
                }
                else
                {
                    Gsm_At_Retry = 0;
                    Gsm_At_Stage = AT_GSM_PWROFF;
                }
            }
            else
                GsmTaskPend (0);
        break;
        case AT_GSM_GCONTRDP: 
            AT_STAGE_MSG ("AT_GSM_GCONTRDP\r\n");
            SetATCMDRom (Q_Gcontrdp, 1000);
            Gsm_At_Stage = AT_GSM_GCONTRDP_WAIT;             
        break;
        case AT_GSM_GCONTRDP_WAIT: 
            AT_STAGE_MSG ("AT_GSM_GCONTRDP_WAIT\r\n");
            if (Gsm_At_Flag.bit.OK)//if (Gsm_At_Flag.bit.OK && Gsm_At_Flag.bit.Gcontrdp)
            {
                Gsm_At_Flag.value = 0;
                Gsm_At_Stage = AT_GSM_MIPLCREATE;
                Gsm_At_Retry = 0;
            }
            else if (Gsm_At_Flag.bit.timeout)
            {
                Gsm_At_Retry ++;
                if (Gsm_At_Retry < 3)
                {
                    Gsm_At_Stage = AT_GSM_GCONTRDP;
                    GsmDelayTime(1000);
                }
                else
                {
                    Gsm_At_Retry = 0;
                    Gsm_At_Stage = AT_GSM_PWROFF;
                }
            }
            else
                GsmTaskPend (0);
        break;
            
        case AT_GSM_MIPLCREATE:
            AT_STAGE_MSG ("AT_GSM_MIPLCREATE\r\n");
            SetATCMDRom (AT_MiplCreate, 5000);
            Gsm_At_Stage = AT_GSM_MIPLCREATE_WAIT;   
        break;
        case AT_GSM_MIPLCREATE_WAIT:
            AT_STAGE_MSG ("AT_GSM_MIPLCREATE_WAIT\r\n");
            if (Gsm_At_Flag.bit.OK && Gsm_At_Flag.bit.IdCrtRsp)
            {
                Gsm_At_Flag.value = 0;
                Gsm_At_Stage = AT_GSM_MIPLADDOBJ;
                Gsm_At_Retry = 0;
            }
            else if (Gsm_At_Flag.bit.timeout)
            {
                Gsm_At_Retry ++;
                if (Gsm_At_Retry < 3)
                {
                    Gsm_At_Stage = AT_GSM_MIPLCREATE;
                    GsmDelayTime(1000);
                }
                else
                {
                    Gsm_At_Retry = 0;
                    Gsm_At_Stage = AT_GSM_PWROFF;
                }
            }
            else
                GsmTaskPend (0);
        break;
        case AT_GSM_MIPLADDOBJ:
            AT_STAGE_MSG ("AT_GSM_MIPLADDOBJ\r\n");
            SetATCMDVsRom (5000, AT_MiplAddObj, IdCreateRsp);
            Gsm_At_Stage = AT_GSM_MIPLADDOBJ_WAIT;   
        break;
        case AT_GSM_MIPLADDOBJ_WAIT:
            AT_STAGE_MSG ("AT_GSM_MIPLADDOBJ_WAIT\r\n");
            if (Gsm_At_Flag.bit.OK)
            {
                Gsm_At_Flag.value = 0;
                Gsm_At_Stage = AT_GSM_MIPLOPEN;
                Gsm_At_Retry = 0;
            }
            else if (Gsm_At_Flag.bit.timeout)
            {
                Gsm_At_Retry ++;
                if (Gsm_At_Retry < 3)
                {
                    Gsm_At_Stage = AT_GSM_MIPLADDOBJ;
                    GsmDelayTime(1000);
                }
                else
                {
                    Gsm_At_Retry = 0;
                    Gsm_At_Stage = AT_GSM_PWROFF;
                }
            }
            else
                GsmTaskPend (0);
        break;
        case AT_GSM_MIPLOPEN:
            AT_STAGE_MSG ("AT_GSM_MIPLOPEN\r\n");
            SetATCMDVsRom (20000, AT_MiplOpen, IdCreateRsp);
            Gsm_At_Stage = AT_GSM_MIPLOPEN_WAIT;   
        break;
        case AT_GSM_MIPLOPEN_WAIT:
            AT_STAGE_MSG ("AT_GSM_MIPLOPEN_WAIT\r\n");
            if (Gsm_At_Flag.bit.miplObserve && Gsm_At_Flag.bit.miplEvt_4 
                && Gsm_At_Flag.bit.miplEvt_6 && Gsm_At_Flag.bit.OK)
            {
                Gsm_At_Flag.value = 0;
                Gsm_At_Stage = AT_GSM_MIPLOBSERVERSP;
                Gsm_At_Retry = 0;
            }
            else if (Gsm_At_Flag.bit.timeout)
            {
                Gsm_At_Retry ++;
                if (Gsm_At_Retry < 3)
                {
                    Gsm_At_Stage = AT_GSM_MIPLOPEN;
                    GsmDelayTime(1000);
                }
                else
                {
                    Gsm_At_Retry = 0;
                    Gsm_At_Stage = AT_GSM_PWROFF;
                }
            }
            else
                GsmTaskPend (0);
        break;
        case AT_GSM_MIPLOBSERVERSP:
            AT_STAGE_MSG ("AT_GSM_MIPLOBSERVERSP\r\n");
            SetATCMDVsRom (10000, AT_MiplOberserveRsp, IdCreateRsp, TestObj.IdObserveMsg);
            Gsm_At_Stage = AT_GSM_MIPLOBSERVERSP_WAIT;   
        break;    
        case AT_GSM_MIPLOBSERVERSP_WAIT:
            AT_STAGE_MSG ("AT_GSM_MIPLOBSERVERSP_WAIT\r\n");
            if (Gsm_At_Flag.bit.miplDiscover && Gsm_At_Flag.bit.OK)
            {
                Gsm_At_Flag.value = 0;
                Gsm_At_Stage = AT_GSM_MIPLDISCOVERRSP;
                Gsm_At_Retry = 0;
            }
            else if (Gsm_At_Flag.bit.timeout)
            {
                Gsm_At_Retry ++;
                if (Gsm_At_Retry < 3)
                {
                    Gsm_At_Stage = AT_GSM_MIPLOBSERVERSP;
                    GsmDelayTime(1000);
                }
                else
                {
                    Gsm_At_Retry = 0;
                    Gsm_At_Stage = AT_GSM_PWROFF;
                }
            }
            else
                GsmTaskPend (0);
        break;
        case AT_GSM_MIPLDISCOVERRSP:
            AT_STAGE_MSG ("AT_GSM_MIPLDISCOVERRSP\r\n");
            SetATCMDVsRom (5000, AT_MiplDiscoverRsp, IdCreateRsp, IdDiscoverMsg);
            Gsm_At_Stage = AT_GSM_MIPLDISCOVERRSP_WAIT;   
        break;  
        case AT_GSM_MIPLDISCOVERRSP_WAIT:
            AT_STAGE_MSG ("AT_GSM_MIPLDISCOVERRSP_WAIT\r\n");
//            if (Gsm_At_Flag.bit.miplEvt_14 && Gsm_At_Flag.bit.miplEvt_11 && Gsm_At_Flag.bit.OK)
            if (Gsm_At_Flag.bit.OK)
            {
                Gsm_At_Flag.value = 0;
                Gsm_Status.bit.connect = 1;
                Gsm_At_Stage = AT_GSM_STATUS;
                Gsm_At_Retry = 0;
            }
            else if (Gsm_At_Flag.bit.timeout)
            {
                Gsm_At_Retry ++;
                if (Gsm_At_Retry < 3)
                {
                    Gsm_At_Stage = AT_GSM_MIPLDISCOVERRSP;
                    GsmDelayTime(1000);
                }
                else
                {
                    Gsm_At_Retry = 0;
                    Gsm_At_Stage = AT_GSM_PWROFF;
                }
            }
            else
                GsmTaskPend (0);
        break;
        case AT_GSM_MIPLNOTIFY:
            AT_STAGE_MSG ("AT_GSM_MIPLNOTIFY\r\n");
            if (Gsm_Status.bit.connect)
            {
                SetATCMDVsRom (10000, AT_MiplNotify, IdCreateRsp, TestObj.IdObserveMsg, GsmDataBuffer[0]);
                Gsm_At_Stage = AT_GSM_MIPLNOTIFY_WAIT; 
                Gsm_At_Flag.value = 0;
            }
            else
            {
                Gsm_At_Stage = AT_GSM_CSQ;
                Gsm_At_Flag.value = 0;
                Gsm_At_Retry = 0;
            }
        case AT_GSM_MIPLNOTIFY_WAIT:
            AT_STAGE_MSG ("AT_GSM_MIPLNOTIFY_WAIT\r\n");
            if (Gsm_At_Flag.bit.OK)
            {
                Gsm_At_Flag.value = 0;
                Gsm_At_Stage = AT_GSM_STATUS;
                Gsm_At_Retry = 0;
                Gsm_Status.bit.WaitSend = 0;
            }
            else if (Gsm_At_Flag.bit.miplEvt_25 || Gsm_At_Flag.bit.timeout)
            {
                Gsm_At_Flag.value = 0;
                Gsm_At_Retry ++;
                if (Gsm_At_Retry < 5)
                {
                    Gsm_At_Stage = AT_GSM_MIPLNOTIFY;
                }
                else
                {
                    Gsm_At_Retry = 0;
                    Gsm_At_Stage = AT_GSM_PWROFF;
                    Gsm_Status.bit.connect = 0;
                }
            }
            else
                GsmTaskPend (0);
        break;

        case AT_GSM_STATUS:
            AT_STAGE_MSG ("AT_GSM_STATUS\r\n");
            if (Gsm_Status.bit.WaitSend)
            {
                Gsm_At_Stage = AT_GSM_MIPLNOTIFY;
            }
            else
            {
                SetGsmTime(5000);
                Gsm_At_Stage = AT_GSM_STATUS_WAIT;
            }
            Gsm_At_Flag.value = 0;
            Gsm_At_Retry = 0;
        break;
        case AT_GSM_STATUS_WAIT: 
            AT_STAGE_MSG ("AT_GSM_STATUS_WAIT\r\n");
            if (Gsm_Status.bit.WaitSend)
            {
                Gsm_At_Stage = AT_GSM_MIPLNOTIFY;
                Gsm_At_Flag.value = 0;
                Gsm_At_Retry = 0;
            }
            else if (Gsm_At_Flag.bit.timeout)  // 定时间隔查询 CSQ 和 REG
            {
                if (Gsm_Status.bit.gapQ)
                    Gsm_At_Stage = AT_GSM_CSQ;
                else 
                    Gsm_At_Stage = AT_GSM_REG;
                Gsm_Status.bit.gapQ = ~Gsm_Status.bit.gapQ;
                Gsm_At_Flag.value = 0;
                Gsm_At_Retry = 0;
            }
            else
                GsmTaskPend (0);
        break;         
        
        default: break;
    }
}
#endif

#define  RECV_FORMAT_HEX_STRING  1
void GsmUartProcess (uint8_t ch, RecvHandle handle) 
{
    static uint8_t RcvChar = 0;
	switch(GSM_Recv_Sta)
	{
        case RECV_STAGE_RECV_DATA_1:   // +CM2MCLIRECV: 0301
            GSM_Recv_Sta = RECV_STAGE_RECV_DATA_2;
            if (ch >= '0' && ch <= '9')
                RcvChar = ch - '0';
            else if (ch >= 'a' && ch <= 'f')
                RcvChar = ch - 'a' + 10;
            else if (ch >= 'A' && ch <= 'F')
                RcvChar = ch - 'A' + 10;
            else
            {
                GSM_Recv_Sta = RECV_STAGE_DEF;
                RcvChar = 0;
            }
        break;
        case RECV_STAGE_RECV_DATA_2:
            GSM_Recv_Sta = RECV_STAGE_RECV_DATA_1;
            RcvChar *= 0x10;
            if (ch >= '0' && ch <= '9')
                RcvChar += ch - '0';
            else if (ch >= 'a' && ch <= 'f')
                RcvChar += ch - 'a' + 10;
            else if (ch >= 'A' && ch <= 'F')
                RcvChar += ch - 'A' + 10;
            else
            {
                GSM_Recv_Sta = RECV_STAGE_DEF;
                RcvChar = 0;
                break;
            }
            handle (RcvChar);
            RcvChar = 0;
        break;
            
		case RECV_STAGE_DEF:
            if (ch == 'O')
                GSM_Recv_Sta = RECV_STAGE_OK_1;
            else if (ch == '+')
                GSM_Recv_Sta = RECV_STAGE_PLUS_1;
            else if (ch == 'E')
                GSM_Recv_Sta = RECV_STAGE_ERR_1;
            else if (ch == 'S')
                GSM_Recv_Sta = RECV_STAGE_CSOSNDM_1;
        break;

        GSM_SW_CASE_1_FIND( RECV_STAGE_OK_1, 'K');
        GSM_SW_CASE_1_FIND( RECV_STAGE_OK_2, '\r');
        GSM_SW_CASE_OVER  ( RECV_STAGE_OK_3, '\n', Gsm_At_Flag.bit.OK);

		case RECV_STAGE_PLUS_1:
            if (ch == 'C')
                GSM_Recv_Sta = RECV_STAGE_PLUS_2;
#if  ONENET_PLATFORM_ENABLE > 0u
            else if (ch == 'M')
                GSM_Recv_Sta = RECV_STAGE_MIPLCREATE_0;   
#endif 
            else
                GSM_Recv_Sta = RECV_STAGE_DEF;
        break;
            
		case RECV_STAGE_PLUS_2:
            if (ch == 'P')
                GSM_Recv_Sta = RECV_STAGE_CPIN_1;
            else if (ch == 'S')
                GSM_Recv_Sta = RECV_STAGE_PLUS_4;
            else if (ch == 'G')
                GSM_Recv_Sta = RECV_STAGE_PLUS_3; 
            else if (ch == 'O')
                GSM_Recv_Sta = RECV_STAGE_COPS_1;
            else if (ch == 'M')
                GSM_Recv_Sta = RECV_STAGE_M2MCLI_1;
            else if (ch == 'C')
                GSM_Recv_Sta = RECV_STAGE_CCID_1;
            else if (ch == 'I')
                GSM_Recv_Sta = RECV_STAGE_IMSI_1;
            else
                GSM_Recv_Sta = RECV_STAGE_DEF;
        break;

		case RECV_STAGE_PLUS_3:
            if (ch == 'R')
                GSM_Recv_Sta = RECV_STAGE_GREG_1;
            else if (ch == 'A')
                GSM_Recv_Sta = RECV_STAGE_GACT_1;
            else if (ch == 'S')
                GSM_Recv_Sta = RECV_STAGE_IMEI_1;
//            else if (ch == 'C')
//                GSM_Recv_Sta = RECV_STAGE_GREG_1;            
            else
                GSM_Recv_Sta = RECV_STAGE_DEF;
        break;

		case RECV_STAGE_PLUS_4:
            if (ch == 'Q')
                GSM_Recv_Sta = RECV_STAGE_CSQ_2;
            else if (ch == 'O')
                GSM_Recv_Sta = RECV_STAGE_CSOC_1;
//            else if (ch == 'E')
//                GSM_Recv_Sta = RECV_STAGE_CSOERR_1;
            else
                GSM_Recv_Sta = RECV_STAGE_DEF;
        break;
            
        GSM_SW_CASE_1_FIND( RECV_STAGE_CPIN_1, 'I');
        GSM_SW_CASE_1_FIND( RECV_STAGE_CPIN_2, 'N');
        GSM_SW_CASE_1_FIND( RECV_STAGE_CPIN_3, ':');
        GSM_SW_CASE_1_FIND( RECV_STAGE_CPIN_4, ' ');
        GSM_SW_CASE_1_FIND( RECV_STAGE_CPIN_5, 'R');
        GSM_SW_CASE_1_FIND( RECV_STAGE_CPIN_6, 'E');
        GSM_SW_CASE_1_FIND( RECV_STAGE_CPIN_7, 'A');
        GSM_SW_CASE_1_FIND( RECV_STAGE_CPIN_8, 'D');
        GSM_SW_CASE_OVER  ( RECV_STAGE_CPIN_9, 'Y', Gsm_At_Flag.bit.PinReady);
        
        // +CSQ: 17,0
//        GSM_SW_CASE_1_FIND( RECV_STAGE_CSQ_1, 'Q');
        GSM_SW_CASE_1_FIND( RECV_STAGE_CSQ_2, ':');
        GSM_SW_CASE_1_FIND( RECV_STAGE_CSQ_3, ' ');
        case RECV_STAGE_CSQ_4:
            if (ch >= '0' && ch <= '9')
            {
                GsmCsqValue = ch - '0'; 
                GSM_Recv_Sta = RECV_STAGE_CSQ_5;
            }                
            else
                GSM_Recv_Sta = RECV_STAGE_DEF; 
        break;
        case RECV_STAGE_CSQ_5:
            if (ch >= '0' && ch <= '9')
            {
                GsmCsqValue = GsmCsqValue*10 + ch - '0';
                GSM_Recv_Sta = RECV_STAGE_CSQ_6;
                break;
            }
        case RECV_STAGE_CSQ_6:
            if (ch == ',')
                GSM_Recv_Sta = RECV_STAGE_CSQ_7;
            else
            {
                GSM_Recv_Sta = RECV_STAGE_DEF;
                GsmCsqValue = 0;
            }
        break;
        case RECV_STAGE_CSQ_7:
#ifdef  DEBUG_CSQ_EQUAL_1        // 调试 在 +CSQ: 17,x x!=0的情况 
            if (ch == '0')
            {
                Gsm_At_Flag.bit.CSQ_En = 1;
            }
            else
                GsmCsqValue = 0;
#endif
            Gsm_At_Flag.bit.CSQ_En = 1;
            GSM_Recv_Sta ++;
        break;
        GSM_SW_CASE_1_FIND( RECV_STAGE_CSQ_8, '\r');
        case RECV_STAGE_CSQ_9:
            GsmTaskPost ();
            GSM_Recv_Sta = RECV_STAGE_DEF;
        break;
        
        GSM_SW_CASE_1_FIND( RECV_STAGE_GREG_1, 'E');
        GSM_SW_CASE_1_FIND( RECV_STAGE_GREG_2, 'G');
        GSM_SW_CASE_1_FIND( RECV_STAGE_GREG_3, ':');
        GSM_SW_CASE_1_FIND( RECV_STAGE_GREG_4, ' ');
        GSM_SW_CASE_1_FIND( RECV_STAGE_GREG_5, '0');
        GSM_SW_CASE_1_FIND( RECV_STAGE_GREG_6, ',');
        GSM_SW_CASE_1_FIND( RECV_STAGE_GREG_7, '1');
        GSM_SW_CASE_1_FIND( RECV_STAGE_GREG_8, '\r');
        GSM_SW_CASE_OVER  ( RECV_STAGE_GREG_9, '\n', Gsm_At_Flag.bit.Greg);

        GSM_SW_CASE_1_FIND( RECV_STAGE_GACT_1, 'C');
        GSM_SW_CASE_1_FIND( RECV_STAGE_GACT_2, 'T');
        GSM_SW_CASE_1_FIND( RECV_STAGE_GACT_3, ':');
        GSM_SW_CASE_1_FIND( RECV_STAGE_GACT_4, ' ');
        GSM_SW_CASE_1_FIND( RECV_STAGE_GACT_5, '1');
        GSM_SW_CASE_1_FIND( RECV_STAGE_GACT_6, ',');
        GSM_SW_CASE_OVER  ( RECV_STAGE_GACT_7, '1', Gsm_At_Flag.bit.Gact);

        GSM_SW_CASE_1_FIND( RECV_STAGE_COPS_1, 'P');
        GSM_SW_CASE_1_FIND( RECV_STAGE_COPS_2, 'S');
        GSM_SW_CASE_1_FIND( RECV_STAGE_COPS_3, ':');
        GSM_SW_CASE_1_FIND( RECV_STAGE_COPS_4, ' ');
        GSM_SW_CASE_1_FIND( RECV_STAGE_COPS_5, '0');
        GSM_SW_CASE_1_FIND( RECV_STAGE_COPS_6, ',');
        GSM_SW_CASE_1_FIND( RECV_STAGE_COPS_7, '2');
        GSM_SW_CASE_1_FIND( RECV_STAGE_COPS_8, ',');
        GSM_SW_CASE_1_FIND( RECV_STAGE_COPS_9, '"');
        GSM_SW_CASE_1_FIND( RECV_STAGE_COPS_10, '4');
        GSM_SW_CASE_1_FIND( RECV_STAGE_COPS_11, '6');
        GSM_SW_CASE_1_FIND( RECV_STAGE_COPS_12, '0');
        GSM_SW_CASE_2_FIND( RECV_STAGE_COPS_13, '0', RECV_STAGE_COPS_14, '1', RECV_STAGE_COPS_15);
        case RECV_STAGE_COPS_14:
            if (ch == '0')
            {
                SimCardType = CHINA_MOBILE;
                GSM_Recv_Sta +=2;
            }
            else
                GSM_Recv_Sta = RECV_STAGE_DEF;
        break;            
        case RECV_STAGE_COPS_15:
            if (ch == '1')
            {
                SimCardType = CHINA_NET;
                GSM_Recv_Sta ++;
            }
            else
                GSM_Recv_Sta = RECV_STAGE_DEF;
        break;
        GSM_SW_CASE_1_FIND( RECV_STAGE_COPS_16, '"');
        GSM_SW_CASE_1_FIND( RECV_STAGE_COPS_17, ',');
        GSM_SW_CASE_OVER  ( RECV_STAGE_COPS_18, '9', Gsm_At_Flag.bit.Cops);

		case RECV_STAGE_M2MCLI_1:
            if (ch == '2')
                GSM_Recv_Sta = RECV_STAGE_M2MCLI_2;           
            else
                GSM_Recv_Sta = RECV_STAGE_DEF;
        break;
            
        GSM_SW_CASE_1_FIND( RECV_STAGE_M2MCLI_2, 'M');
        GSM_SW_CASE_1_FIND( RECV_STAGE_M2MCLI_3, 'C');
        GSM_SW_CASE_1_FIND( RECV_STAGE_M2MCLI_4, 'L');
        GSM_SW_CASE_1_FIND( RECV_STAGE_M2MCLI_5, 'I');
        GSM_SW_CASE_2_FIND( RECV_STAGE_M2MCLI_6, ':', RECV_STAGE_M2MCLI_7, 
                                                 'R', RECV_STAGE_RECV_1);
        GSM_SW_CASE_1_FIND( RECV_STAGE_M2MCLI_7, ' ');
        case RECV_STAGE_M2MCLI_8:
            GSM_Recv_Sta ++;
            if (ch == '1')
            {
                Gsm_At_Flag.bit.m2mcli_1 = 1;
            }
            else if (ch == '2')
            {
                Gsm_At_Flag.bit.m2mcli_2 = 1;
            }
            else if (ch == '4')
            {
                Gsm_At_Flag.bit.m2mcli_4 = 1;
            }
            else if (ch == '5')
            {
                Gsm_At_Flag.bit.m2mcli_5 = 1;
            }
            else
                GSM_Recv_Sta = RECV_STAGE_DEF;
        break;
        GSM_SW_CASE_1_FIND( RECV_STAGE_M2MCLI_9, '\r');
        case RECV_STAGE_M2MCLI_10:
            GsmTaskPost ();
            GSM_Recv_Sta = RECV_STAGE_DEF;
        break;
            
        GSM_SW_CASE_1_FIND( RECV_STAGE_RECV_1, 'E');
        GSM_SW_CASE_1_FIND( RECV_STAGE_RECV_2, 'C');
        GSM_SW_CASE_1_FIND( RECV_STAGE_RECV_3, 'V');
        GSM_SW_CASE_1_FIND( RECV_STAGE_RECV_4, ':');
        GSM_SW_CASE_1_FIND( RECV_STAGE_RECV_5, ' ');
        
        GSM_SW_CASE_1_FIND( RECV_STAGE_ERR_1, 'R');
        GSM_SW_CASE_1_FIND( RECV_STAGE_ERR_2, 'R');
        GSM_SW_CASE_1_FIND( RECV_STAGE_ERR_3, 'O');
        GSM_SW_CASE_OVER  ( RECV_STAGE_ERR_4, 'R', Gsm_At_Flag.bit.err);
        
        /*
        AT+CCID
        89860404191790237777

        OK
        */
        GSM_SW_CASE_1_FIND( RECV_STAGE_CCID_1, 'I');
        GSM_SW_CASE_1_FIND( RECV_STAGE_CCID_2, 'D');
        GSM_SW_CASE_1_FIND( RECV_STAGE_CCID_3, '\r');
        GSM_SW_CASE_1_FIND( RECV_STAGE_CCID_4, '\r'); 
        case RECV_STAGE_CCID_5:
            if (ch == '\n')
            {
                memset (RecBufTemp, 0, SIM_STR_LENGTH);
                RecBufLen = 0;
                GSM_Recv_Sta ++;
            }                
            else
                GSM_Recv_Sta = RECV_STAGE_DEF; 
        break;        
        case RECV_STAGE_CCID_6:
        {         
            if (   ( (ch >= '0') && (ch <= '9') )
                || ( (ch >= 'A') && (ch <= 'Z') )
                || ( (ch >= 'a') && (ch <= 'z') ) )
            {
                RecBufTemp[RecBufLen++] = ch;
            }            
            else if ( (ch == '\r') || (ch == '\n') )
            {
                if ( RecBufLen == SIM_CCID_CHAR_LENGTH )
                {
                    RecBufTemp[RecBufLen++] = '\0';
                    memcpy( SimCcidCh, RecBufTemp, SIM_CCID_CHAR_LENGTH+1 );
                    GSM_DebugMsg ("\r\nCCID: \r\n%s\r\n", SimCcidCh);
                }
                memset( RecBufTemp, 0, SIM_STR_LENGTH );
                RecBufLen = 0;
                GSM_Recv_Sta = (Gsm_REC_Process_Stage)0;   
            }
            else if ( RecBufLen > SIM_CCID_CHAR_LENGTH )
            {
                memset( RecBufTemp, 0, SIM_STR_LENGTH );
                RecBufLen = 0;
                GSM_Recv_Sta = (Gsm_REC_Process_Stage)0; 
            }
            else
            {
                memset( RecBufTemp, 0, SIM_STR_LENGTH );
                RecBufLen = 0;
                GSM_Recv_Sta = (Gsm_REC_Process_Stage)0; 
            }
        }
        break;

        /*
        AT+CIMI
        460042437101427

        OK
        */
        GSM_SW_CASE_1_FIND( RECV_STAGE_IMSI_1, 'M');
        GSM_SW_CASE_1_FIND( RECV_STAGE_IMSI_2, 'I');
        GSM_SW_CASE_1_FIND( RECV_STAGE_IMSI_3, '\r');
        GSM_SW_CASE_1_FIND( RECV_STAGE_IMSI_4, '\r');
        case RECV_STAGE_IMSI_5:
            if (ch == '\n')
            {
                memset (RecBufTemp, 0, SIM_STR_LENGTH);
                RecBufLen = 0;
                GSM_Recv_Sta ++;
            }                
            else
                GSM_Recv_Sta = RECV_STAGE_DEF; 
        break;        
        case RECV_STAGE_IMSI_6:
        {         
            if (   
                ( (ch >= '0') && (ch <= '9') )
                || ( (ch >= 'A') && (ch <= 'Z') )
                || ( (ch >= 'a') && (ch <= 'z') ) 
               )
            {
                RecBufTemp[RecBufLen++] = ch;
            }            
            else if ( (ch == '\r') || (ch == '\n') )
            {
                if ( RecBufLen <= SIM_IMSI_CHAR_LENGTH )
                {
                    RecBufTemp[RecBufLen++] = '\0';
                    memcpy( SimImsiCh, RecBufTemp, SIM_IMSI_CHAR_LENGTH+1 );
                    GSM_DebugMsg ("\r\nIMSI: \r\n%s\r\n", SimImsiCh);
                }
                memset( RecBufTemp, 0, SIM_STR_LENGTH );
                RecBufLen = 0;
                GSM_Recv_Sta = (Gsm_REC_Process_Stage)0;   
            }
            else if ( RecBufLen > SIM_IMSI_CHAR_LENGTH )
            {
                memset( RecBufTemp, 0, SIM_STR_LENGTH );
                RecBufLen = 0;
                GSM_Recv_Sta = (Gsm_REC_Process_Stage)0; 
            }
            else
            {
                memset( RecBufTemp, 0, SIM_STR_LENGTH );
                RecBufLen = 0;
                GSM_Recv_Sta = (Gsm_REC_Process_Stage)0; 
            }
        }
        break;
     
        /*
		AT+CGSN
		+CGSN: 868334030307205

		OK
        */
        GSM_SW_CASE_1_FIND( RECV_STAGE_IMEI_1, 'N');
        GSM_SW_CASE_1_FIND( RECV_STAGE_IMEI_2, ':');
        case RECV_STAGE_IMEI_3:
            if (ch == ' ')
            {
                memset (RecBufTemp, 0, SIM_STR_LENGTH);
                RecBufLen = 0;
                GSM_Recv_Sta ++;
            }                
            else
                GSM_Recv_Sta = RECV_STAGE_DEF; 
        break;        
        case RECV_STAGE_IMEI_4:
        {         
            if (   ( (ch >= '0') && (ch <= '9') )
//                || ( (ch >= 'A') && (ch <= 'Z') )
//                || ( (ch >= 'a') && (ch <= 'z') ) 
               )
            {
                RecBufTemp[RecBufLen++] = ch;
            }            
            else if ( (ch == '\r') || (ch == '\n') )
            {
                if ( RecBufLen == SIM_IMEI_CHAR_LENGTH )
                {
                    RecBufTemp[RecBufLen++] = '\0';
                    memcpy( SimImeiCh, RecBufTemp, SIM_IMEI_CHAR_LENGTH+1 );
                    GSM_DebugMsg ("\r\nIMEI: \r\n%s\r\n", SimImeiCh);
                }
                memset( RecBufTemp, 0, SIM_STR_LENGTH );
                RecBufLen = 0;
                GSM_Recv_Sta = (Gsm_REC_Process_Stage)0;   
            }
            else if ( RecBufLen > SIM_IMEI_CHAR_LENGTH )
            {
                memset( RecBufTemp, 0, SIM_STR_LENGTH );
                RecBufLen = 0;
                GSM_Recv_Sta = (Gsm_REC_Process_Stage)0; 
            }
            else
            {
                memset( RecBufTemp, 0, SIM_STR_LENGTH );
                RecBufLen = 0;
                GSM_Recv_Sta = (Gsm_REC_Process_Stage)0; 
            }
        }
        break;

#if  ONENET_PLATFORM_ENABLE > 0u  
        GSM_SW_CASE_1_FIND( RECV_STAGE_MIPLCREATE_0, 'I');
        GSM_SW_CASE_1_FIND( RECV_STAGE_MIPLCREATE_1, 'P');
        GSM_SW_CASE_1_FIND( RECV_STAGE_MIPLCREATE_2, 'L');
		case RECV_STAGE_MIPLCREATE_3:
            if (ch == 'C')
                GSM_Recv_Sta = RECV_STAGE_MIPLCREATE_4;      
            else if (ch == 'E')
                GSM_Recv_Sta = RECV_STAGE_MIPLEVENT_1;       
            else if (ch == 'O')
                GSM_Recv_Sta = RECV_STAGE_MIPLOBSERVE_1;
            else if (ch == 'D')
                GSM_Recv_Sta = RECV_STAGE_MIPLDISCOVER_1;            
            else
                GSM_Recv_Sta = RECV_STAGE_DEF;
        break;
        GSM_SW_CASE_1_FIND( RECV_STAGE_MIPLCREATE_4, 'R');
        GSM_SW_CASE_1_FIND( RECV_STAGE_MIPLCREATE_5, 'E');
        GSM_SW_CASE_1_FIND( RECV_STAGE_MIPLCREATE_6, 'A');
        GSM_SW_CASE_1_FIND( RECV_STAGE_MIPLCREATE_7, 'T');
        GSM_SW_CASE_1_FIND( RECV_STAGE_MIPLCREATE_8, 'E');
        GSM_SW_CASE_1_FIND( RECV_STAGE_MIPLCREATE_9, ':');
        GSM_SW_CASE_1_FIND( RECV_STAGE_MIPLCREATE_10, ' ');
		case RECV_STAGE_MIPLCREATE_11:
            if (ch >= '0' && ch <= '9')
            {
                IdCreateRsp = ch - '0'; //ch & 0x0F;
                Gsm_At_Flag.bit.IdCrtRsp = 1;
                GsmTaskPost ();
            }                
            GSM_Recv_Sta = RECV_STAGE_DEF;
        break;

        GSM_SW_CASE_1_FIND( RECV_STAGE_MIPLEVENT_1, 'V');
        GSM_SW_CASE_1_FIND( RECV_STAGE_MIPLEVENT_2, 'E');
        GSM_SW_CASE_1_FIND( RECV_STAGE_MIPLEVENT_3, 'N');
        GSM_SW_CASE_1_FIND( RECV_STAGE_MIPLEVENT_4, 'T');
        GSM_SW_CASE_1_FIND( RECV_STAGE_MIPLEVENT_5, ':');
        GSM_SW_CASE_1_FIND( RECV_STAGE_MIPLEVENT_6, ' ');
        GSM_SW_CASE_1_FIND( RECV_STAGE_MIPLEVENT_7, (TestObj.IdCreateRsp | 0x30));
		case RECV_STAGE_MIPLEVENT_8:
            if (ch == ',')
            {
                RecBufTemp[0] = 0;
                RecBufLen = 0;
                GSM_Recv_Sta ++;
            }
            else            
                GSM_Recv_Sta = RECV_STAGE_DEF;
        break;
        case RECV_STAGE_MIPLEVENT_9:
        {         
            if ( ( (ch >= '0') && (ch <= '9') ) )
            {
                RecBufLen++;
                RecBufTemp[0] = RecBufTemp[0]*10 + (ch & 0x0F);
            }            
            else if ( (ch == '\r') || (ch == '\n') )
            {
                switch (RecBufTemp[0])
                {
                    case 4: Gsm_At_Flag.bit.miplEvt_4 = 1;   break;
                    case 6: Gsm_At_Flag.bit.miplEvt_6 = 1;   break;
                    case 11: Gsm_At_Flag.bit.miplEvt_11 = 1; break;
                    case 14: Gsm_At_Flag.bit.miplEvt_14 = 1; break;
                    case 25: Gsm_At_Flag.bit.miplEvt_25 = 1; break;
                    default: break;
                }
                GsmTaskPost ();
                GSM_Recv_Sta = (Gsm_REC_Process_Stage)0;   
            }
            else if ( RecBufLen > 2 )
            {
                RecBufLen = 0;
                GSM_Recv_Sta = (Gsm_REC_Process_Stage)0; 
            }
            else
            {
                RecBufLen = 0;
                GSM_Recv_Sta = (Gsm_REC_Process_Stage)0; 
            }
        }
        break;

        GSM_SW_CASE_1_FIND( RECV_STAGE_MIPLOBSERVE_1, 'B');
        GSM_SW_CASE_1_FIND( RECV_STAGE_MIPLOBSERVE_2, 'S');
        GSM_SW_CASE_1_FIND( RECV_STAGE_MIPLOBSERVE_3, 'E');
        GSM_SW_CASE_1_FIND( RECV_STAGE_MIPLOBSERVE_4, 'R');
        GSM_SW_CASE_1_FIND( RECV_STAGE_MIPLOBSERVE_5, 'V');
        GSM_SW_CASE_1_FIND( RECV_STAGE_MIPLOBSERVE_6, 'E');
        GSM_SW_CASE_1_FIND( RECV_STAGE_MIPLOBSERVE_6_1, ':');
        GSM_SW_CASE_1_FIND( RECV_STAGE_MIPLOBSERVE_6_2, ' ');
        GSM_SW_CASE_1_FIND( RECV_STAGE_MIPLOBSERVE_6_3, (TestObj.IdCreateRsp | 0x30));
		case RECV_STAGE_MIPLOBSERVE_7:
            if (ch == ',')
            {
                *(u32 *)&RecBufTemp[0] = 0;
                RecBufLen = 0;
                GSM_Recv_Sta ++;
            }
            else            
                GSM_Recv_Sta = RECV_STAGE_DEF;
        break;
        case RECV_STAGE_MIPLOBSERVE_8:
        {         
            if ( ( (ch >= '0') && (ch <= '9') ) )
            {
                RecBufLen++;
                *(u32 *)&RecBufTemp[0] = *(u32 *)&RecBufTemp[0]*10 + (ch & 0x0F);
            }            
            else if (ch == ',' && RecBufLen > 0)
            {
                TestObj.IdObserveMsg = *(u32 *)&RecBufTemp[0];
                GSM_Recv_Sta ++;   
            }
            else if ( RecBufLen > 8 )
            {
                RecBufLen = 0;
                GSM_Recv_Sta = (Gsm_REC_Process_Stage)0; 
            }
            else
            {
                RecBufLen = 0;
                GSM_Recv_Sta = (Gsm_REC_Process_Stage)0; 
            }
        }
        break;  
        GSM_SW_CASE_1_FIND( RECV_STAGE_MIPLOBSERVE_9, '1');
        GSM_SW_CASE_1_FIND( RECV_STAGE_MIPLOBSERVE_10, ',');
        GSM_SW_CASE_1_FIND( RECV_STAGE_MIPLOBSERVE_11, '3');
        GSM_SW_CASE_1_FIND( RECV_STAGE_MIPLOBSERVE_12, '3');
        GSM_SW_CASE_1_FIND( RECV_STAGE_MIPLOBSERVE_13, '0');
        GSM_SW_CASE_1_FIND( RECV_STAGE_MIPLOBSERVE_14, '3');
        GSM_SW_CASE_1_FIND( RECV_STAGE_MIPLOBSERVE_15, ',');
        GSM_SW_CASE_1_FIND( RECV_STAGE_MIPLOBSERVE_16, '0');
        GSM_SW_CASE_1_FIND( RECV_STAGE_MIPLOBSERVE_17, ',');
        GSM_SW_CASE_1_FIND( RECV_STAGE_MIPLOBSERVE_18, '-');
        GSM_SW_CASE_OVER(   RECV_STAGE_MIPLOBSERVE_19, '1', Gsm_At_Flag.bit.miplObserve);

        // +MIPLDISCOVER: 0,61342,3303
		case RECV_STAGE_MIPLDISCOVER_1:
            if (ch == 'I')
            {
                GSM_Recv_Sta = RECV_STAGE_JOY;
                AtuoCompareString ((u8 *)"SCOVER: 0", strlen("SCOVER: 0"), RECV_STAGE_MIPLDISCOVER_2);
            }
            else            
                GSM_Recv_Sta = RECV_STAGE_DEF;
        break;
		case RECV_STAGE_MIPLDISCOVER_2:
            if (ch == ',')
            {
                *(u32 *)&RecBufTemp[0] = 0;
                RecBufLen = 0;
                GSM_Recv_Sta ++;
            }
            else            
                GSM_Recv_Sta = RECV_STAGE_DEF;
        break;
		case RECV_STAGE_MIPLDISCOVER_3:
        {         
            if ( ( (ch >= '0') && (ch <= '9') ) )
            {
                RecBufLen++;
                *(u32 *)&RecBufTemp[0] = *(u32 *)&RecBufTemp[0]*10 + (ch & 0x0F);
            }            
            else if (ch == ',' && RecBufLen > 0)
            {
                IdDiscoverMsg = *(u32 *)&RecBufTemp[0];
                GSM_Recv_Sta = RECV_STAGE_JOY;
                AtuoCompareString ((u8 *)"3303", strlen("3303"), RECV_STAGE_MIPLDISCOVER_4);
            }
            else if ( RecBufLen > 8 )
            {
                RecBufLen = 0;
                GSM_Recv_Sta = (Gsm_REC_Process_Stage)0; 
            }
            else
            {
                RecBufLen = 0;
                GSM_Recv_Sta = (Gsm_REC_Process_Stage)0; 
            }
        }
        break; 
        case RECV_STAGE_MIPLDISCOVER_4:
        {
            GSM_Recv_Sta = RECV_STAGE_DEF;
            Gsm_At_Flag.bit.miplDiscover = 1;
            GsmTaskPost ();
        }
        break;
#endif


		case RECV_STAGE_CSOC_1:
            if (ch == 'C')
                GSM_Recv_Sta = RECV_STAGE_CSOC_2;
            else if (ch == 'N')
                GSM_Recv_Sta = RECV_STAGE_CSORCV_1;
//            else if (ch == 'E')
//                GSM_Recv_Sta = RECV_STAGE_CSOERR_1;
            else
                GSM_Recv_Sta = RECV_STAGE_DEF;
        break;
            
        GSM_SW_CASE_1_FIND( RECV_STAGE_CSOC_2, ':');
        GSM_SW_CASE_1_FIND( RECV_STAGE_CSOC_3, ' ');
        case RECV_STAGE_CSOC_4:
            if (ch >= '0' && ch <= '9')
            {
                NewestSocketId = ch - '0'; 
                GSM_Recv_Sta = RECV_STAGE_CSOC_5;
            }                
            else
                GSM_Recv_Sta = RECV_STAGE_DEF; 
        break;
        GSM_SW_CASE_1_FIND( RECV_STAGE_CSOC_5, '\r');
        GSM_SW_CASE_OVER(   RECV_STAGE_CSOC_6, '\n', Gsm_At_Flag.bit.SoCreate);

        GSM_SW_CASE_1_FIND( RECV_STAGE_CSORCV_1, 'M');
        GSM_SW_CASE_1_FIND( RECV_STAGE_CSORCV_2, 'I');
        GSM_SW_CASE_1_FIND( RECV_STAGE_CSORCV_3, ':');
        GSM_SW_CASE_1_FIND( RECV_STAGE_CSORCV_4, ' ');
        case RECV_STAGE_CSORCV_5:
            if (ch >= '0' && ch <= '9')
            {
                if (NewestSocketId == ch - '0') 
                {
                    GSM_Recv_Sta = RECV_STAGE_CSOC_6;
                    SO_RCV_LEN = 0;
                    break;
                }
            }                
            GSM_Recv_Sta = RECV_STAGE_DEF; 
        break;
        GSM_SW_CASE_1_FIND( RECV_STAGE_CSORCV_6, ',');
        case RECV_STAGE_CSORCV_7:
            if (ch >= '0' && ch <= '9')
            {
                SO_RCV_LEN = SO_RCV_LEN*10 + ch - '0';
                break;
            }
        case RECV_STAGE_CSORCV_8:
            if (ch == ',')
                GSM_Recv_Sta = RECV_STAGE_RECV_1;
            else
            {
                GSM_Recv_Sta = RECV_STAGE_DEF;
            }
        break;

        // SEND: 0,12\r\n
        GSM_SW_CASE_1_FIND( RECV_STAGE_CSOSNDM_1, 'E');
        GSM_SW_CASE_1_FIND( RECV_STAGE_CSOSNDM_2, 'N');
        GSM_SW_CASE_1_FIND( RECV_STAGE_CSOSNDM_3, 'D');
        GSM_SW_CASE_1_FIND( RECV_STAGE_CSOSNDM_4, ':');
        GSM_SW_CASE_1_FIND( RECV_STAGE_CSOSNDM_5, ' ');
        case RECV_STAGE_CSOSNDM_6:
            if (ch >= '0' && ch <= '9')
            {
                if (NewestSocketId == ch - '0') 
                {
                    GSM_Recv_Sta = RECV_STAGE_CSOSNDM_7;
                    SO_RCV_LEN = 0;
                    break;
                }
            }                
            GSM_Recv_Sta = RECV_STAGE_DEF; 
        break;
        GSM_SW_CASE_1_FIND( RECV_STAGE_CSOSNDM_7, ',');
        case RECV_STAGE_CSOSNDM_8:
            if (ch >= '0' && ch <= '9')
            {
                SO_RCV_LEN = SO_RCV_LEN*10 + ch - '0';
                break;
            }
        GSM_SW_CASE_1_FIND( RECV_STAGE_CSOSNDM_9, '\r');
        GSM_SW_CASE_OVER(   RECV_STAGE_CSOSNDM_10, '\n', Gsm_At_Flag.bit.SoSend);
#if RECV_STAGE_JOY_ENABLE > 0u
        case RECV_STAGE_JOY:
        {
            if (pDest == NULL || DestStringLen == 0)
            {
                GSM_Recv_Sta = RECV_STAGE_DEF;
            }
            else
            {
                if (ch == pDest[CurDoLen])
                {
                    CurDoLen ++;
                    if (CurDoLen == DestStringLen)
                        GSM_Recv_Sta = NextStep;
                }
                else            
                    GSM_Recv_Sta = RECV_STAGE_DEF;
            }
        }
        break;
#endif
            
        default: break;
    }
    
}


