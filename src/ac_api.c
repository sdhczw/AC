/**
******************************************************************************
* @file     ZC_api.c
* @authors  cxy
* @version  V1.0.0
* @date     10-Sep-2014
* @brief   
******************************************************************************
*/

#include <zc_common.h>
#include <zc_protocol_interface.h>
#include <zc_protocol_controller.h>
#include <ac_hal.h>
#include <zc_module_interface.h>
/*************************************************
* Function: AC_BuildOption
* Description: 
* Author: cxy 
* Returns: 
* Parameter: 
* History:
*************************************************/
void AC_BuildOption(AC_OptList *pstruOptList, u8 *pu8OptNum, u8 *pu8Buffer, u16 *pu16Len)
{
    //��ѡ�ֶ��������
    ZC_MessageOptHead *pstruOpt;
    u8 u8OptNum = 0;
    u8 u16OptLen = 0;
    
    *pu16Len = u16OptLen;
    *pu8OptNum = u8OptNum;
    
    
    if (NULL == pstruOptList)
    {
        return;
    }
    
    pstruOpt = (ZC_MessageOptHead *)pu8Buffer;

    /*add opt, if it exist*/
    if (NULL != pstruOptList->pstruTransportInfo)
    {
        pstruOpt->OptCode = ZC_HTONS(ZC_OPT_TRANSPORT);
        pstruOpt->OptLen = ZC_HTONS(sizeof(ZC_TransportInfo));
        memcpy((u8*)(pstruOpt + 1), (u8*)pstruOptList->pstruTransportInfo, sizeof(ZC_TransportInfo));

        u8OptNum++;
        u16OptLen += sizeof(ZC_MessageOptHead) + sizeof(ZC_TransportInfo);
        pstruOpt = (ZC_MessageOptHead *)(pu8Buffer + u16OptLen);        
    }
    

    if (NULL != pstruOptList->pstruSsession)
    {
        pstruOpt = (ZC_MessageOptHead *)pu8Buffer;
        pstruOpt->OptCode = ZC_HTONS(ZC_OPT_SSESSION);
        pstruOpt->OptLen = ZC_HTONS(sizeof(ZC_SsessionInfo));
        memcpy((u8*)(pstruOpt + 1), (u8*)pstruOptList->pstruSsession, sizeof(ZC_SsessionInfo));

        u8OptNum++;
        u16OptLen += sizeof(ZC_MessageOptHead) + sizeof(ZC_SsessionInfo);
        pstruOpt = (ZC_MessageOptHead *)(pu8Buffer + u16OptLen);        
    }    

    
    *pu16Len = u16OptLen;
    *pu8OptNum = u8OptNum;
    return;
}

/*************************************************
* Function: AC_BuildMessage
* Description: 
* Author: cxy 
* Returns: 
* Parameter: 
* History:
*************************************************/
void AC_BuildMessage(u8 u8MsgCode, u8 u8MsgId, 
    u8 *pu8Payload, u16 u16PayloadLen,
    AC_OptList *pstruOptList,
    u8 *pu8Msg, u16 *pu16Len)
{
    //Э���������
    ZC_MessageHead *pstruMsg = (ZC_MessageHead *)pu8Msg;
    u16 u16OptLen = 0;
    u16 crc = 0;
    


    AC_BuildOption(pstruOptList, &pstruMsg->OptNum, (pu8Msg + sizeof(ZC_MessageHead)), &u16OptLen);
    memmove((pu8Msg + sizeof(ZC_MessageHead) + u16OptLen), pu8Payload, u16PayloadLen);   
    pstruMsg->MsgCode = u8MsgCode;
    pstruMsg->MsgId = u8MsgId;  
    pstruMsg->Version = 0;

    /*updata len*/
    pstruMsg->Payloadlen = ZC_HTONS(u16PayloadLen + u16OptLen);

    /*calc crc*/
    crc = crc16_ccitt((pu8Msg + sizeof(ZC_MessageHead)), (u16PayloadLen + u16OptLen));
    pstruMsg->TotalMsgCrc[0]=(crc&0xff00)>>8;
    pstruMsg->TotalMsgCrc[1]=(crc&0xff);


    *pu16Len = (u16)sizeof(ZC_MessageHead) + u16PayloadLen + u16OptLen;
}

/*************************************************
* Function: AC_BuildMessage
* Description: 
* Author: cxy 
* Returns: 
* Parameter: 
* History:
*************************************************/
void AC_BuildEasyMessage(u8 u8MsgCode, u8 u8MsgId, 
                         u8 *pu8Payload, u16 u16PayloadLen,
                         AC_OptList *pstruOptList,
                         u8 *pu8Msg, u16 *pu16Len)
{
    if(NULL!=pstruOptList->pstruSsession) 
    {
        *pu16Len = 0xc+u16PayloadLen;
        pu8Msg[0] = 0x5a;
        pu8Msg[1] = (*pu16Len>>8)&0xff;
        pu8Msg[2] = *pu16Len;
        pu8Msg[3] = u8MsgId;   
        pu8Msg[4] = 0x01;  
        pu8Msg[5] = u8MsgCode;
        memcpy(pu8Msg + 6,pstruOptList->pstruSsession,4); 
        memcpy(pu8Msg+10,pu8Payload,u16PayloadLen);
        pu8Msg[*pu16Len -2]=AC_CalcSum(pu8Msg+1,*pu16Len-3);
        pu8Msg[*pu16Len -1]=0x5b;
        
    }
    else
    {
        *pu16Len = 0x8+u16PayloadLen;
        pu8Msg[0] = 0x5a;
        pu8Msg[1] = (*pu16Len>>8)&0xff;
        pu8Msg[2] = *pu16Len;
        pu8Msg[3] = u8MsgId;   
        pu8Msg[4] = 0x00;  
        pu8Msg[5] = u8MsgCode;
        memcpy(pu8Msg+6,pu8Payload,u16PayloadLen);
        pu8Msg[*pu16Len -2]=AC_CalcSum(pu8Msg+1,*pu16Len-3);
        pu8Msg[*pu16Len -1]=0x5b;
    }
}

/*************************************************
* Function: AC_SendDeviceStart
* Description: 
* Author: cxy 
* Returns: 
* Parameter: 
* History:
*************************************************/
void AC_SendDeviceStart()
{
    //�豸������Ϣ
    u16 u16DateLen;

    AC_BuildMessage(ZC_CODE_EQ_BEGIN, 0, 
        NULL, 0,        /*payload+payload len*/
        NULL,
        g_u8MsgBuildBuffer, &u16DateLen);
    
    AC_SendMessage(g_u8MsgBuildBuffer, u16DateLen);
}

/*************************************************
* Function: AC_SendMessage
* Description: 
* Author: cxy 
* Returns: 
* Parameter: 
* History:
*************************************************/

void AC_SendMessage(u8 *pu8Msg, u16 u16DataLen)
{
    ZC_RecvDataFromMoudle( pu8Msg,  u16DataLen);
}

/*************************************************
* Function: AC_SendRestMsg
* Description: 
* Author: cxy 
* Returns: 
* Parameter: 
* History:
*************************************************/
void AC_SendRestMsg()
{
    //wifi��������
    u16 u16DateLen;
    u8 u8MsgBuildBuffer[8];
    AC_BuildMessage(ZC_CODE_REST, 0, 
        NULL, 0,        /*payload+payload len*/
        NULL,
        u8MsgBuildBuffer, &u16DateLen);
    
    AC_SendMessage(u8MsgBuildBuffer, u16DateLen);
}

/*************************************************
* Function: AC_SendRestMsg
* Description: 
* Author: cxy 
* Returns: 
* Parameter: 
* History:
*************************************************/
void AC_SendUbindMsg()
{
    //ǿ�ƽ��
    u16 u16DateLen;
    u32 u32ResetFlag = 0;
    u8 u8MsgBuildBuffer[12];
    AC_BuildMessage(ZC_CODE_UNBIND, 0, 
        (u8 *)&u32ResetFlag, sizeof(u32ResetFlag),        /*payload+payload len*/
        NULL,
        u8MsgBuildBuffer, &u16DateLen);
    AC_SendMessage(u8MsgBuildBuffer, u16DateLen);

}

/*************************************************
* Function: AC_ParseOption
* Description: 
* Author: cxy 
* Returns: 
* Parameter: 
* History:
*************************************************/
void AC_ParseOption(ZC_MessageHead *pstruMsg, AC_OptList *pstruOptList, u16 *pu16OptLen)
{
    //����Option
    u8 u8OptNum;
    ZC_MessageOptHead *pstruOptHead;
    u16 u16Offset;

    u16Offset = sizeof(ZC_MessageHead);
    pstruOptHead = (ZC_MessageOptHead *)((u8*)pstruMsg + u16Offset);
    *pu16OptLen = 0;

    for (u8OptNum = 0; u8OptNum < pstruMsg->OptNum; u8OptNum++)
    {
        switch (ZC_HTONS(pstruOptHead->OptCode))
        {
            case ZC_OPT_TRANSPORT:
                pstruOptList->pstruTransportInfo = (ZC_TransportInfo *)(pstruOptHead + 1);
                break;
            case ZC_OPT_SSESSION:
                pstruOptList->pstruSsession = (ZC_SsessionInfo *)(pstruOptHead + 1);            
                break;
        }
        *pu16OptLen += sizeof(ZC_MessageOptHead) + ZC_HTONS(pstruOptHead->OptLen);
        pstruOptHead = (ZC_MessageOptHead *)((u8*)pstruOptHead + sizeof(ZC_MessageOptHead) + ZC_HTONS(pstruOptHead->OptLen));

    }
}

/*************************************************
* Function: AC_SendDeviceRegsiterWithMac
* Description: 
* Author: cxy 
* Returns: 
* Parameter: 
* History:
*************************************************/
void AC_SendDeviceRegsiterWithMac(u8 *pu8EqVersion, u8 *pu8ModuleKey, u8 *pu8Domain)
{
    //ͳһ��⣬�豸ע�������豸id��Ч��ʹ��wifi��mac��ַ��Ϊ�豸id�����й̼�ʹ��һ��˽Կ
    ZC_ExtRegisterReq struExtReg;
    u16 u16DateLen;

    struExtReg.struExtMessageHead.ExtMsgCode = ZC_CODE_EXT_REGSITER;
    memcpy(struExtReg.struRegReq.u8EqVersion, pu8EqVersion, ZC_EQVERSION_LEN);
    memcpy(struExtReg.struRegReq.u8ModuleKey, pu8ModuleKey, ZC_MODULE_KEY_LEN); 
    memcpy(struExtReg.struRegReq.u8Domain, pu8Domain, ZC_DOMAIN_LEN); 

    AC_BuildMessage(ZC_CODE_EXT, 0, 
        (u8*)&struExtReg, sizeof(ZC_ExtRegisterReq),   /*payload+payload len*/
        NULL,
        g_u8MsgBuildBuffer, &u16DateLen);
    
    AC_SendMessage(g_u8MsgBuildBuffer, u16DateLen);
}

/*************************************************
* Function: AC_SendDeviceRegsiter
* Description: 
* Author: cxy 
* Returns: 
* Parameter: 
* History:
*************************************************/
void AC_SendDeviceRegsiter(u8 *pu8EqVersion, u8 *pu8ModuleKey, u8 *pu8Domain, u8 *pu8DeviceId)
{
    //�豸ע������
    ZC_RegisterReq *pstruReg= (ZC_RegisterReq *)g_u8MsgBuildBuffer;
    u16 u16DateLen;
    memcpy(pstruReg->u8EqVersion, pu8EqVersion, ZC_EQVERSION_LEN);
    memcpy(pstruReg->u8ModuleKey, pu8ModuleKey, ZC_MODULE_KEY_LEN); 
    memcpy(pstruReg->u8Domain, pu8Domain, ZC_DOMAIN_LEN); 
    strcpy((char *)pstruReg->u8DeviceId, (const char *)pu8DeviceId); 

    AC_BuildMessage(ZC_CODE_REGSITER, 0, 
        (u8*)pstruReg, sizeof(ZC_RegisterReq)+strlen((const char *)pu8DeviceId),   /*payload+payload len*/
        NULL,
        g_u8MsgBuildBuffer, &u16DateLen);
    
    AC_SendMessage(g_u8MsgBuildBuffer, u16DateLen);
}

/*************************************************
* Function: AC_RecvMessage
* Description: 
* Author: cxy 
* Returns: 
* Parameter: 
* History:
*************************************************/
void AC_RecvMessage(ZC_MessageHead *pstruMsg)
{
    //�����յ���Ϣ����Ҫ���øýӿڴ�����Ϣ��
    AC_OptList struOptList;
    u16 u16OptLen = 0;
    u8 *pu8Playload = NULL;

    struOptList.pstruSsession = NULL;
    struOptList.pstruTransportInfo = NULL;
    
    /*Parser Option*/
    AC_ParseOption(pstruMsg, &struOptList, &u16OptLen);
    pu8Playload = (u8*)pstruMsg + u16OptLen + sizeof(ZC_MessageHead);
    switch(pstruMsg->MsgCode)
    {
        //�¼�֪ͨ����Ϣ
        case ZC_CODE_EQ_DONE:
        case ZC_CODE_WIFI_CONNECTED:
        case ZC_CODE_WIFI_DISCONNECTED:
        case ZC_CODE_CLOUD_CONNECTED:
        case ZC_CODE_CLOUD_DISCONNECTED:
            AC_DealNotifyMessage(pstruMsg, &struOptList, pu8Playload);
            break;
        //�豸�¼�����Ϣ    
        default:
        {
            AC_DealEvent(pstruMsg, &struOptList, pu8Playload);
        }
            break;            
    }
}

/*************************************************
* Function: AC_SendDevStatus2Server
* Description: 
* Author: cxy 
* Returns: 
* Parameter: 
* History:
*************************************************/
u8 AC_CalcSum(u8 *pu8Src, u16 u16Len)
{
	u8 u8Sum = 0;
	u32 u32i;
	for(u32i = 0; u32i < u16Len;u32i++)
	{
		u8Sum += pu8Src[u32i];
	}
	return u8Sum;
}
/*************************************************
* Function: AC_SendDevStatus2Server
* Description: 
              ============================================
             |  1B  |1B|1B|1B| 1B | 1B |  nB  | 1B |  1B |
             ============================================
            | 0x5A | len |id|resv|code|payload|sum|0x5B |
            ============================================
            sum = len + id + resv + code + payload 
* Author: zhangwen 
* Returns:
* Parameter: 
* History:
*************************************************/
u32 AC_CheckSum(u8 * Buffer,u32 len) 
{ 
    u8 u8Sum =AC_CalcSum(Buffer+1,len-3);
    if (Buffer[len-2]==AC_CalcSum(Buffer+1,len-3)) 
    {
        return(ZC_RET_OK);//У�����ȷ 
    }
    else 
    {
//        u32 i;
//        for(i = 0; i < len; i++)
//        {
//            AC_Printf("0x%x ",Buffer[i]); 
//        }
//        AC_Printf("\n");  
//        AC_Printf("checksum error,recv = 0x%x,calc = 0x%x!!!\n",Buffer[len-2],u8Sum); 
        return(ZC_RET_ERROR);  //У��ʹ��� 
    }
} 


