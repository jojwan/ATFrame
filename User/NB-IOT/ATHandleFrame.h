#ifndef  __ATHANDLEFRAME_H__
#define  __ATHANDLEFRAME_H__

#include "stm32f10x.h"
//#include <stdint.h>
#include <stdio.h>

typedef void JustDo (void);
typedef void Dofun (u8 *p);
typedef void DofunP (void *p);
typedef u8   RevRsp (void);
typedef void ATFrameSndStr (const char *cmd, uint16_t tm);
typedef u8 CharProcHandle (u8 data, u8 isOver, u8 cnt);

typedef struct _AtFrameStage
{
    char *       ShowMsg;
    const char * AtCmdStr;
    Dofun *      SendCmdFun;
    JustDo *     SetRequestFun;
    RevRsp *     IsTrueFun;
    Dofun *      DoTrueFun;
    RevRsp *     IsFalseFun;
    Dofun *      DoFalseFun;
    Dofun *      DoTimeoutFun;
    u16          Timeout;
    u16          RetryTime;
}AtFrameStage;

typedef struct _ATFrameVarMem
{
    u8 NextATCmd;
    u8 CurStep;
    u8 CurRetryTime;
    u8 ShowMsgWFlag;
    u32  *    FlagBit;
    JustDo *  TaskSleep;
    RevRsp *  IsTimeout;
    ATFrameSndStr * SendCmdBase;
}ATFrameStageMem;

typedef struct _AtFrameStringProcess
{
    struct _AtFrameStringProcess *pNext;
    const char * AtRspTarget;
    DofunP *      ConfirmRspFun;
    CharProcHandle * ProcessDataFun;
    u8  DontRemoveMe;   // 匹配上当前节点字符串，置位标志，就把当前节点移除
    u16 AtRspTargetLen;
    u8 MatchFlag;
}AtFrameStringProcess;

typedef struct _ATFrameStrMem
{
    AtFrameStringProcess * pHead;
    JustDo * TaskPostSem;
//    AtFrameStringProcess * CmdRspStr;
    uint8_t ATProcLen;
    uint8_t ATProcStarCnt;  // 记录当前在处理第几个星号
    uint8_t LastStartPosition;
}ATFrameStrMem;

void InitATStageFrame ( ATFrameStageMem * pVar,    
                        u32  *          FlagBit,
                        JustDo *        TaskSleep,
                        RevRsp *        IsTimeout,
                        ATFrameSndStr * SendCmdBase);
void AT_Cmd_Task (ATFrameStageMem * pAtVar, AtFrameStage * pFmSt);
void AT_Cmd_String_Process (ATFrameStrMem * pATFrameStr, uint8_t ch);
void ResetATFrameStrParam(ATFrameStrMem * pATFrameStr);
void RemoveProcNode(AtFrameStringProcess ** pHead, AtFrameStringProcess * pNode);
void InsertProcNode(AtFrameStringProcess ** pHead, AtFrameStringProcess * pNode);


/*******************  Bit definition  *******************/
#define  	AT_FILTER_FB0                        ((uint32_t)0x00000001)        /*!< Filter bit 0 */
#define  	AT_FILTER_FB1                        ((uint32_t)0x00000002)        /*!< Filter bit 1 */
#define  	AT_FILTER_FB2                        ((uint32_t)0x00000004)        /*!< Filter bit 2 */
#define  	AT_FILTER_FB3                        ((uint32_t)0x00000008)        /*!< Filter bit 3 */
#define  	AT_FILTER_FB4                        ((uint32_t)0x00000010)        /*!< Filter bit 4 */
#define  	AT_FILTER_FB5                        ((uint32_t)0x00000020)        /*!< Filter bit 5 */
#define  	AT_FILTER_FB6                        ((uint32_t)0x00000040)        /*!< Filter bit 6 */
#define  	AT_FILTER_FB7                        ((uint32_t)0x00000080)        /*!< Filter bit 7 */
#define  	AT_FILTER_FB8                        ((uint32_t)0x00000100)        /*!< Filter bit 8 */
#define  	AT_FILTER_FB9                        ((uint32_t)0x00000200)        /*!< Filter bit 9 */
#define  	AT_FILTER_FB10                       ((uint32_t)0x00000400)        /*!< Filter bit 10 */
#define  	AT_FILTER_FB11                       ((uint32_t)0x00000800)        /*!< Filter bit 11 */
#define  	AT_FILTER_FB12                       ((uint32_t)0x00001000)        /*!< Filter bit 12 */
#define  	AT_FILTER_FB13                       ((uint32_t)0x00002000)        /*!< Filter bit 13 */
#define  	AT_FILTER_FB14                       ((uint32_t)0x00004000)        /*!< Filter bit 14 */
#define  	AT_FILTER_FB15                       ((uint32_t)0x00008000)        /*!< Filter bit 15 */
#define  	AT_FILTER_FB16                       ((uint32_t)0x00010000)        /*!< Filter bit 16 */
#define  	AT_FILTER_FB17                       ((uint32_t)0x00020000)        /*!< Filter bit 17 */
#define  	AT_FILTER_FB18                       ((uint32_t)0x00040000)        /*!< Filter bit 18 */
#define  	AT_FILTER_FB19                       ((uint32_t)0x00080000)        /*!< Filter bit 19 */
#define  	AT_FILTER_FB20                       ((uint32_t)0x00100000)        /*!< Filter bit 20 */
#define  	AT_FILTER_FB21                       ((uint32_t)0x00200000)        /*!< Filter bit 21 */
#define  	AT_FILTER_FB22                       ((uint32_t)0x00400000)        /*!< Filter bit 22 */
#define  	AT_FILTER_FB23                       ((uint32_t)0x00800000)        /*!< Filter bit 23 */
#define  	AT_FILTER_FB24                       ((uint32_t)0x01000000)        /*!< Filter bit 24 */
#define  	AT_FILTER_FB25                       ((uint32_t)0x02000000)        /*!< Filter bit 25 */
#define  	AT_FILTER_FB26                       ((uint32_t)0x04000000)        /*!< Filter bit 26 */
#define  	AT_FILTER_FB27                       ((uint32_t)0x08000000)        /*!< Filter bit 27 */
#define  	AT_FILTER_FB28                       ((uint32_t)0x10000000)        /*!< Filter bit 28 */
#define  	AT_FILTER_FB29                       ((uint32_t)0x20000000)        /*!< Filter bit 29 */
#define  	AT_FILTER_FB30                       ((uint32_t)0x40000000)        /*!< Filter bit 30 */
#define  	AT_FILTER_FB31                       ((uint32_t)0x80000000)        /*!< Filter bit 31 */

#endif



