#ifndef  __NB_IOT_H__
#define  __NB_IOT_H__

#include <stdint.h>

#define ONENET_PLATFORM_ENABLE    0u

typedef	enum
{
    AT_GSM_NONE = 0,			//NOTHIG TO DO
    AT_GSM_PWROFF,
    AT_GSM_PWROFF_WAIT,
    AT_GSM_PWRON,
    AT_GSM_PWRON_WAIT,
    AT_GSM_RESTART,
    AT_GSM_RESTART_WAIT,

    AT_GSM_READY,
    AT_GSM_READY_WAIT,    
    AT_GSM_PIN,
    AT_GSM_PIN_WAIT,
    AT_GSM_CSQ,
    AT_GSM_CSQ_WAIT,
    AT_GSM_REG,
    AT_GSM_REG_WAIT,
    AT_GSM_GACT,
    AT_GSM_GACT_WAIT,
    AT_GSM_COPS,
    AT_GSM_COPS_WAIT,
    AT_GSM_GCONTRDP,
    AT_GSM_GCONTRDP_WAIT,
    AT_GSM_CLINNEW,
    AT_GSM_CLINNEW_WAIT,
    AT_GSM_CLISEND,
    AT_GSM_CLISEND_WAIT,  
    AT_GSM_STATUS,
    AT_GSM_STATUS_WAIT,
#if  ONENET_PLATFORM_ENABLE > 0u   
    AT_GSM_SETBOOTSTRAP,
    AT_GSM_SETBOOTSTRAP_WAIT,
    AT_GSM_MIPLCREATE,
    AT_GSM_MIPLCREATE_WAIT,
    AT_GSM_MIPLADDOBJ,
    AT_GSM_MIPLADDOBJ_WAIT,
    AT_GSM_MIPLOPEN,
    AT_GSM_MIPLOPEN_WAIT,
    AT_GSM_MIPLOBSERVERSP,
    AT_GSM_MIPLOBSERVERSP_WAIT,
    AT_GSM_MIPLDISCOVERRSP,
    AT_GSM_MIPLDISCOVERRSP_WAIT, 
    AT_GSM_MIPLNOTIFY,
    AT_GSM_MIPLNOTIFY_WAIT,
#endif
    AT_GSM_AUTO_SEND_CMD,
    AT_GSM_AUTO_SEND_CMD_WAIT,

}Gsm_Stage;

typedef	enum
{
    RECV_STAGE_DEF,
    RECV_STAGE_OK_1,
    RECV_STAGE_OK_2,
    RECV_STAGE_OK_3,
    
    RECV_STAGE_PLUS_1,
    RECV_STAGE_PLUS_2,
    RECV_STAGE_PLUS_3,
    RECV_STAGE_PLUS_4,
    
    RECV_STAGE_CPIN_1,
    RECV_STAGE_CPIN_2,
    RECV_STAGE_CPIN_3,
    RECV_STAGE_CPIN_4,
    RECV_STAGE_CPIN_5,
    RECV_STAGE_CPIN_6,
    RECV_STAGE_CPIN_7,
    RECV_STAGE_CPIN_8,
    RECV_STAGE_CPIN_9,
    RECV_STAGE_CPIN_10,
    
    RECV_STAGE_CSQ_1,
    RECV_STAGE_CSQ_2,
    RECV_STAGE_CSQ_3,
    RECV_STAGE_CSQ_4,
    RECV_STAGE_CSQ_5,
    RECV_STAGE_CSQ_6,
    RECV_STAGE_CSQ_7,
    RECV_STAGE_CSQ_8,
    RECV_STAGE_CSQ_9,
    
    RECV_STAGE_GREG_1,
    RECV_STAGE_GREG_2,
    RECV_STAGE_GREG_3,
    RECV_STAGE_GREG_4,
    RECV_STAGE_GREG_5,
    RECV_STAGE_GREG_6,
    RECV_STAGE_GREG_7,
    RECV_STAGE_GREG_8,
    RECV_STAGE_GREG_9,
    
    RECV_STAGE_GACT_1,
    RECV_STAGE_GACT_2,
    RECV_STAGE_GACT_3,
    RECV_STAGE_GACT_4,
    RECV_STAGE_GACT_5,
    RECV_STAGE_GACT_6,
    RECV_STAGE_GACT_7,
    RECV_STAGE_GACT_8,
    RECV_STAGE_GACT_9,  

    RECV_STAGE_COPS_1,
    RECV_STAGE_COPS_2,
    RECV_STAGE_COPS_3,
    RECV_STAGE_COPS_4,
    RECV_STAGE_COPS_5,
    RECV_STAGE_COPS_6,
    RECV_STAGE_COPS_7,
    RECV_STAGE_COPS_8,
    RECV_STAGE_COPS_9, 
    RECV_STAGE_COPS_10,
    RECV_STAGE_COPS_11,
    RECV_STAGE_COPS_12,
    RECV_STAGE_COPS_13,
    RECV_STAGE_COPS_14,
    RECV_STAGE_COPS_15,
    RECV_STAGE_COPS_16,
    RECV_STAGE_COPS_17,
    RECV_STAGE_COPS_18,

    RECV_STAGE_M2MCLI_1,
    RECV_STAGE_M2MCLI_2,
    RECV_STAGE_M2MCLI_3,
    RECV_STAGE_M2MCLI_4,
    RECV_STAGE_M2MCLI_5,
    RECV_STAGE_M2MCLI_6,
    RECV_STAGE_M2MCLI_7,
    RECV_STAGE_M2MCLI_8,
    RECV_STAGE_M2MCLI_9, 
    RECV_STAGE_M2MCLI_10, 

    RECV_STAGE_CCID_1,
    RECV_STAGE_CCID_2,
    RECV_STAGE_CCID_3,
    RECV_STAGE_CCID_4,
    RECV_STAGE_CCID_5,
    RECV_STAGE_CCID_6,
    RECV_STAGE_CCID_7,
    RECV_STAGE_CCID_8,
    RECV_STAGE_CCID_9, 
    RECV_STAGE_CCID_10, 
   
    RECV_STAGE_IMSI_1,
    RECV_STAGE_IMSI_2,
    RECV_STAGE_IMSI_3,
    RECV_STAGE_IMSI_4,
    RECV_STAGE_IMSI_5,
    RECV_STAGE_IMSI_6,
    RECV_STAGE_IMSI_7,
    RECV_STAGE_IMSI_8,
    RECV_STAGE_IMSI_9, 
    RECV_STAGE_IMSI_10, 

    RECV_STAGE_IMEI_1,
    RECV_STAGE_IMEI_2,
    RECV_STAGE_IMEI_3,
    RECV_STAGE_IMEI_4,
    RECV_STAGE_IMEI_5,
    RECV_STAGE_IMEI_6,
    RECV_STAGE_IMEI_7,
    RECV_STAGE_IMEI_8,
    RECV_STAGE_IMEI_9, 
    RECV_STAGE_IMEI_10, 
    
    RECV_STAGE_RECV_1,
    RECV_STAGE_RECV_2,
    RECV_STAGE_RECV_3,
    RECV_STAGE_RECV_4,
    RECV_STAGE_RECV_5,
    RECV_STAGE_RECV_DATA_1, 
    RECV_STAGE_RECV_DATA_2, 
    
    /*--------------------------------------------------------------------------*/
    /* connect OneNET platform cmd*/
    RECV_STAGE_MIPLCREATE_0,
    RECV_STAGE_MIPLCREATE_1,
    RECV_STAGE_MIPLCREATE_2,
    RECV_STAGE_MIPLCREATE_3,
    RECV_STAGE_MIPLCREATE_4,
    RECV_STAGE_MIPLCREATE_5,
    RECV_STAGE_MIPLCREATE_6,
    RECV_STAGE_MIPLCREATE_7,
    RECV_STAGE_MIPLCREATE_8,
    RECV_STAGE_MIPLCREATE_9, 
    RECV_STAGE_MIPLCREATE_10,
    RECV_STAGE_MIPLCREATE_11,
    
    RECV_STAGE_MIPLEVENT_1,
    RECV_STAGE_MIPLEVENT_2,
    RECV_STAGE_MIPLEVENT_3,
    RECV_STAGE_MIPLEVENT_4,
    RECV_STAGE_MIPLEVENT_5,
    RECV_STAGE_MIPLEVENT_6,
    RECV_STAGE_MIPLEVENT_7,
    RECV_STAGE_MIPLEVENT_8,
    RECV_STAGE_MIPLEVENT_9, 
    RECV_STAGE_MIPLEVENT_10,
    
    RECV_STAGE_MIPLOBSERVE_1,
    RECV_STAGE_MIPLOBSERVE_2,
    RECV_STAGE_MIPLOBSERVE_3,
    RECV_STAGE_MIPLOBSERVE_4,
    RECV_STAGE_MIPLOBSERVE_5,
    RECV_STAGE_MIPLOBSERVE_6,
    RECV_STAGE_MIPLOBSERVE_6_1,
    RECV_STAGE_MIPLOBSERVE_6_2,
    RECV_STAGE_MIPLOBSERVE_6_3,
    RECV_STAGE_MIPLOBSERVE_7,
    RECV_STAGE_MIPLOBSERVE_8,
    RECV_STAGE_MIPLOBSERVE_9, 
    RECV_STAGE_MIPLOBSERVE_10,
    RECV_STAGE_MIPLOBSERVE_11,
    RECV_STAGE_MIPLOBSERVE_12,    
    RECV_STAGE_MIPLOBSERVE_13,    
    RECV_STAGE_MIPLOBSERVE_14,    
    RECV_STAGE_MIPLOBSERVE_15,    
    RECV_STAGE_MIPLOBSERVE_16,    
    RECV_STAGE_MIPLOBSERVE_17,    
    RECV_STAGE_MIPLOBSERVE_18,    
    RECV_STAGE_MIPLOBSERVE_19,    
    
    RECV_STAGE_MIPLDISCOVER_1,
    RECV_STAGE_MIPLDISCOVER_2,
    RECV_STAGE_MIPLDISCOVER_3,
    RECV_STAGE_MIPLDISCOVER_4,
    RECV_STAGE_MIPLDISCOVER_5,
    RECV_STAGE_MIPLDISCOVER_6,
    RECV_STAGE_MIPLDISCOVER_7,
    RECV_STAGE_MIPLDISCOVER_8,
    RECV_STAGE_MIPLDISCOVER_9, 
    RECV_STAGE_MIPLDISCOVER_10,

    RECV_STAGE_CSOC_1,
    RECV_STAGE_CSOC_2,
    RECV_STAGE_CSOC_3,
    RECV_STAGE_CSOC_4,
    RECV_STAGE_CSOC_5,
    RECV_STAGE_CSOC_6,
    RECV_STAGE_CSOC_7,
    RECV_STAGE_CSOC_8,
    RECV_STAGE_CSOC_9,

    RECV_STAGE_CSORCV_1,
    RECV_STAGE_CSORCV_2,
    RECV_STAGE_CSORCV_3,
    RECV_STAGE_CSORCV_4,
    RECV_STAGE_CSORCV_5,
    RECV_STAGE_CSORCV_6,
    RECV_STAGE_CSORCV_7,
    RECV_STAGE_CSORCV_8,
    RECV_STAGE_CSORCV_9,
    
    RECV_STAGE_CSOERR_1,
    RECV_STAGE_CSOERR_2,
    RECV_STAGE_CSOERR_3,
    RECV_STAGE_CSOERR_4,
    RECV_STAGE_CSOERR_5,
    RECV_STAGE_CSOERR_6,
    RECV_STAGE_CSOERR_7,
    RECV_STAGE_CSOERR_8,
    RECV_STAGE_CSOERR_9,
    
    RECV_STAGE_CSOSNDM_1,  // send mask
    RECV_STAGE_CSOSNDM_2,
    RECV_STAGE_CSOSNDM_3,
    RECV_STAGE_CSOSNDM_4,
    RECV_STAGE_CSOSNDM_5,
    RECV_STAGE_CSOSNDM_6,
    RECV_STAGE_CSOSNDM_7,
    RECV_STAGE_CSOSNDM_8,
    RECV_STAGE_CSOSNDM_9,
    RECV_STAGE_CSOSNDM_10,
    
    RECV_STAGE_JOY,
    
    /*--------------------------------------------------------------------------*/

    RECV_STAGE_ERR_1,
    RECV_STAGE_ERR_2,
    RECV_STAGE_ERR_3,
    RECV_STAGE_ERR_4,
    RECV_STAGE_ERR_5,
    
}Gsm_REC_Process_Stage;

#define  GSM_SW_CASE_OVER( case_tp, j_ch, BitFlag)                              \
		case case_tp:                                                           \
			if ( ch == j_ch )                                                   \
            {                                                                   \
                BitFlag = 1;                                                    \
                GsmTaskPost ();                                                 \
            }                                                                   \
            GSM_Recv_Sta = RECV_STAGE_DEF;                                      \
			break;                                                              \
            
#define  GSM_SW_CASE_1_FIND( case_tp, j_ch)                                     \
		case case_tp:                                                           \
			if ( ch == j_ch )                                                   \
				GSM_Recv_Sta = (Gsm_REC_Process_Stage)(case_tp+1);              \
			else                                                                \
				GSM_Recv_Sta = RECV_STAGE_DEF;                                  \
			break;                                                              \

#define  GSM_SW_CASE_2_FIND( case_tp, j_ch_1, case_tp_1, j_ch_2, case_tp_2)     \
		case case_tp:                                                           \
			if ( ch == j_ch_1 )                                                 \
				GSM_Recv_Sta = case_tp_1;                                       \
			else if ( ch == j_ch_2 )                                            \
				GSM_Recv_Sta = case_tp_2;                                       \
			else                                                                \
				GSM_Recv_Sta = RECV_STAGE_DEF;                                  \
			break;                                                              \
            
#define  GSM_SW_CASE_3_FIND( case_tp, j_ch_1, case_tp_1, j_ch_2, case_tp_2,     \
                                                         j_ch_3, case_tp_3)     \
		case case_tp:                                                           \
			if ( ch == j_ch_1 )                                                 \
				GSM_Recv_Sta = case_tp_1;                                       \
			else if ( ch == j_ch_2 )                                            \
				GSM_Recv_Sta = case_tp_2;                                       \
			else if ( ch == j_ch_3 )                                            \
				GSM_Recv_Sta = case_tp_3;                                       \
			else                                                                \
				GSM_Recv_Sta = RECV_STAGE_DEF;                                  \
			break;                                                              \
            

typedef union {
	uint32_t value;
	struct 
	{
		uint32_t OK:1,	  // 
		PinReady:1,				// 
        CSQ_En:1,
		Greg:1,			// 
		Gact:1,
		Cops:1,
        Gcontrdp:1,
        timeout:1,
        m2mcli_1:1,
        m2mcli_2:1,
        m2mcli_4:1,
        m2mcli_5:1,
        err:1,
        
#if  ONENET_PLATFORM_ENABLE > 0u  
        IdCrtRsp:1,
        miplEvt_4:1,
        miplEvt_6:1,
        miplEvt_11:1,
        miplEvt_14:1,
        miplEvt_25:1,
        miplObserve:1,
        miplDiscover:1,
#endif    
        SoCreate:1,
        SoConnect:1,
        SoSend:1,
        SoRcvDat:1,
        SoERR:1,
        
        end: 1;
        
	}bit;
}Gsm_Control_Union;

typedef union {
	uint32_t value;
	struct 
	{
		uint32_t connect:3,  // 连接上服务器
		gapQ:1,	             // 间隔查询 CSQ 和 GREG
        SendFree: 1,         // 发送空闲，没有数据需要发送
   	    WaitSend: 1,		 // 有数据需要发送
        
        end: 1;
        
	}bit;
}Gsm_Status_Type;

typedef void (*RecvHandle)( unsigned char dat);

void  Gsm_Init (void);
void  Gsm_Timer (uint32_t seconds);
void  Gsm_OneNET_Task (void);
void  GsmUartProcess (uint8_t ch, RecvHandle handle);
void  SetGsmTime(uint32_t timeout);
uint8_t IsChinaMobileCard(void);
uint8_t GetGsmSendStatus (void);
uint8_t GetGsmConnectStatus (void);
uint8_t GsmCNCSendData (uint8_t *buf,uint16_t length);
uint8_t GsmOneNETSendData (uint8_t tmp);
uint8_t GsmTcpSendData (uint8_t *buf,uint16_t length);

    
#endif

