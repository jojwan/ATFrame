#include "ATHandleFrame.h"
#include "bsp_usart.h"

#include "bsp_usart.h"
#define  AT_FRAME_DEBUG_MSG           DebugMsg

enum
{
AT_SEND_CMD,
AT_SEND_CMD_WAIT
};

void InitATStageFrame ( ATFrameStageMem * pVar,    
                        u32  *          FlagBit,
                        JustDo *        TaskSleep,
                        RevRsp *        IsTimeout,
                        ATFrameSndStr * SendCmdBase)
{
    if (pVar != NULL)
    {
        pVar->NextATCmd = 0;
        pVar->CurStep = AT_SEND_CMD;
        pVar->CurRetryTime = 0;
        if ((pVar->FlagBit = FlagBit) != NULL) 
            *(pVar->FlagBit) = 0;
        pVar->TaskSleep = TaskSleep;
        pVar->IsTimeout = IsTimeout;
        pVar->SendCmdBase = SendCmdBase;
    }
}

void AT_Cmd_Task (ATFrameStageMem * pAtVar, AtFrameStage * pFmSt)
{
    switch (pAtVar->CurStep)
    {
        case AT_SEND_CMD: 
            if (pFmSt[pAtVar->NextATCmd].ShowMsg != NULL)
                AT_FRAME_DEBUG_MSG ("[ST]:%s\r\n", pFmSt[pAtVar->NextATCmd].ShowMsg);
            if (pFmSt[pAtVar->NextATCmd].AtCmdStr != NULL)
            {
				pAtVar->CurStep = AT_SEND_CMD_WAIT;
				*(pAtVar->FlagBit) = 0;
                if (pFmSt[pAtVar->NextATCmd].SetRequestFun != NULL)
                    pFmSt[pAtVar->NextATCmd].SetRequestFun ();
                if (pFmSt[pAtVar->NextATCmd].SendCmdFun != NULL)
                    pFmSt[pAtVar->NextATCmd].SendCmdFun (&pAtVar->NextATCmd);
                else
                {
                    if (pAtVar->SendCmdBase != NULL)
                        pAtVar->SendCmdBase (pFmSt[pAtVar->NextATCmd].AtCmdStr, 
                                             pFmSt[pAtVar->NextATCmd].Timeout);  
                }
            }
            else
            {
				pAtVar->CurStep = AT_SEND_CMD;
                pAtVar->NextATCmd = 0;
                pAtVar->CurRetryTime = 0;
                *(pAtVar->FlagBit) = 0;
            }
            pAtVar->ShowMsgWFlag = 1;
        break;
        case AT_SEND_CMD_WAIT: 
            if (pAtVar->ShowMsgWFlag && pFmSt[pAtVar->NextATCmd].ShowMsg != NULL)
            {
                pAtVar->ShowMsgWFlag = 0;
                AT_FRAME_DEBUG_MSG ("[ST]:%s_Wait\r\n", 
                                    pFmSt[pAtVar->NextATCmd].ShowMsg);
            }
            if (pFmSt[pAtVar->NextATCmd].IsTrueFun != NULL
                && pFmSt[pAtVar->NextATCmd].IsTrueFun() )
            {
                u8 stage = pAtVar->NextATCmd;
                *(pAtVar->FlagBit) = 0;
                pAtVar->CurStep = AT_SEND_CMD;
                pAtVar->CurRetryTime = 0;
                pAtVar->NextATCmd ++;
                if (pFmSt[stage].DoTrueFun != NULL)
                {
                    // You can modify pAtVar->NextATCmd in function DoTrueFun
                    pFmSt[stage].DoTrueFun (&pAtVar->NextATCmd);
                }    
            }
            else if (pFmSt[pAtVar->NextATCmd].IsFalseFun != NULL
                && pFmSt[pAtVar->NextATCmd].IsFalseFun() )
            {
                *(pAtVar->FlagBit) = 0;
                pAtVar->CurStep = AT_SEND_CMD;
                pAtVar->CurRetryTime ++;
                if (pFmSt[pAtVar->NextATCmd].DoFalseFun != NULL)
                    pFmSt[pAtVar->NextATCmd].DoFalseFun (&pAtVar->NextATCmd);
            }
            else if (pAtVar->IsTimeout != NULL && pAtVar->IsTimeout())
            {
                *(pAtVar->FlagBit) = 0;
                pAtVar->CurRetryTime ++;
                pAtVar->CurStep = AT_SEND_CMD;
                if (pAtVar->CurRetryTime >= pFmSt[pAtVar->NextATCmd].RetryTime)
                {
                    pAtVar->CurRetryTime = 0;
					pAtVar->NextATCmd = 0;
                }
                if (pFmSt[pAtVar->NextATCmd].DoTimeoutFun != NULL)
                    pFmSt[pAtVar->NextATCmd].DoTimeoutFun (&pAtVar->NextATCmd);
            }
            else
            {
                if (pAtVar->TaskSleep != NULL)
                    pAtVar->TaskSleep ();
            }
        break;
        default: break;
    }
}

void ResetATFrameStrParam(ATFrameStrMem * pATFrameStr)
{
    AtFrameStringProcess * pCur = pATFrameStr->pHead;
    pATFrameStr->ATProcLen = 0;
    pATFrameStr->ATProcStarCnt = 0;
    pATFrameStr->LastStartPosition = 0;
    
    while (pCur != NULL)
    {
        pCur->MatchFlag = 0;
        pCur = pCur->pNext;
    } 
}

void RemoveProcNode(AtFrameStringProcess ** pHead, AtFrameStringProcess * pNode)
{
    if (*pHead == pNode)
    {
        *pHead = (*pHead)->pNext;
    }
    else
    {
        AtFrameStringProcess * pCur = (*pHead);
        while (pCur != NULL)
        {
            if (pCur->pNext == pNode)
            {
                pCur->pNext = pCur->pNext->pNext;
                break;
            }
            pCur = pCur->pNext;
        }
    }
}

void InsertProcNode(AtFrameStringProcess ** pHead, AtFrameStringProcess * pNode)
{
    if ((*pHead) == NULL)
    {
        (*pHead) = pNode;
        pNode->pNext = NULL;
    }
    else
    {
        AtFrameStringProcess * pCur = (*pHead);
        while (pCur != NULL)
        {
            if (pCur->pNext == NULL)
            {
                pCur->pNext = pNode;
                pNode->pNext = NULL;
                break;
            }
            pCur = pCur->pNext;
        }
    }
}

/*
AAA+CSQRN+CSQ: 17,0\r\nBBBOK\r\nCCC+CPIN: READY\r\nDDD
         +CSQ: *,0\r\n
         +CPIN:READY\r\n
         OK\r\n
*/
void AT_Cmd_String_Process (ATFrameStrMem * pATFrameStr, uint8_t ch)
{
    AtFrameStringProcess * pCur = pATFrameStr->pHead;
    u8  isMatch = 0;
    u8  isResetCnt = 0;
    while (pCur != NULL)
    {
        // 在都不匹配的时候，列表中的成员都依次判断
        if (pATFrameStr->ATProcLen == 0 || pCur->MatchFlag)
        {
            isResetCnt ++;
            if (pATFrameStr->ATProcLen >0 && pCur->AtRspTarget[pATFrameStr->ATProcLen] == '*')
            {
                u8 IsFinish = 0;
                // 记录上一次星号位置
                if (pATFrameStr->LastStartPosition != pATFrameStr->ATProcLen)
                {
                    pATFrameStr->LastStartPosition = pATFrameStr->ATProcLen;
                    pATFrameStr->ATProcStarCnt ++;
                }
                if (ch == pCur->AtRspTarget[pATFrameStr->ATProcLen+1])
                {//链表不能出现这种情况 +CSQ: *,0\r\n  +CS*,1\r\n
                    pATFrameStr->ATProcLen += 2;
                    IsFinish = 1;
                }
                if (pCur->ProcessDataFun != NULL)
                    // 在这里面需要做错误处理，比如出现错误字符，超出长度
                    if (pCur->ProcessDataFun (ch, IsFinish, pATFrameStr->ATProcStarCnt))
                    {//出现错误，复位参数
                        ResetATFrameStrParam(pATFrameStr);
                    }
            }
            else if (ch == pCur->AtRspTarget[pATFrameStr->ATProcLen])
            {
                if (pATFrameStr->ATProcLen + 1 == pCur->AtRspTargetLen)
                {
#if 1
                    if (! pCur->DontRemoveMe)
                        RemoveProcNode(&pATFrameStr->pHead, pCur);
#endif
                    // 和目标字符串匹配
                    if (pCur->ConfirmRspFun != NULL)
                        pCur->ConfirmRspFun (pATFrameStr);
                    ResetATFrameStrParam(pATFrameStr);
                    if (pATFrameStr->TaskPostSem != NULL)
                        pATFrameStr->TaskPostSem ();
                    return ;
                }
                isMatch = 1;
                pCur->MatchFlag = 1;
            }
            else if (pATFrameStr->ATProcLen > 0)
            { // 排除前面匹配后面不匹配这种情况  +CSQQ 
                pCur->MatchFlag = 0;
                isResetCnt --; 
            }
        }
        pCur = pCur->pNext;
    }
    if (isMatch)
        pATFrameStr->ATProcLen ++;
    // 在都不匹配的情况下 重设参数。
    if (isResetCnt == 0)
        ResetATFrameStrParam(pATFrameStr);
}
