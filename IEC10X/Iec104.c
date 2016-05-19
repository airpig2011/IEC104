/*******************************************************************
Copyright (C):    
File name    :    Iec104.c
DESCRIPTION  :
AUTHOR       :
Version      :    1.0
Date         :    2014/07/31
Others       :
History      :
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
1) Date: 2014/07/31         Author: ChenDajie
   content:

*******************************************************************/
#include "Iec104.h"
#include "sys.h"

 /*
 * GLOABLE VARIALBLE
 */

uint8_t             Iec104_Sendbuf[IEC104_MAX_BUF_LEN];
/*
 * STATE
 * */
uint8_t             IEC104_STATE_FLAG_INIT = IEC104_FLAG_CLOSED;
uint8_t             IEC104_STATE_FLAG_CALLALL = IEC104_FLAG_CLOSED;
uint8_t             IEC104_STATE_FLAG_GROUP = IEC104_FLAG_CLOSED;
uint8_t             IEC104_STATE_FLAG_CLOCK = IEC104_FLAG_CLOSED;
uint8_t             IEC104_STATE_FLAG_TESTER = IEC104_FLAG_IDLE;
uint8_t             IEC104_STATE_FLAG_S_ACK = IEC104_FLAG_CLOSED;
/*
 * receive and send serial number
 * */
int32_t             Iec104_BuildSendSn = 0;
int32_t             Iec104_BuildRecvSn = 0;
int32_t             Iec104_DealSendSn = -1;
int32_t             Iec104_DealRecvSn = 0;

uint8_t             IEC104_Call_AllQoi = 0;
uint8_t             IEC104_Call_GroupQoi = 0;


#define IEC104_CYCLE_TIME_MS             100                   /*100ms*/
#define IEC104_RESEND_TIME_MS            (30*1000)             /*30s*/
#define IEC104_S_ACK_TIMEOUT             (5*1000)              /*5s*/
#define IEC104_TESTER_IDLE_TIMEOUT       (1*30*1000)           /*2min*/
uint32_t            Iec104_TimeCount = 0;
uint32_t            Iec104_TimeCycle = IEC104_RESEND_TIME_MS;
uint32_t            Iec104_TimeCycle_S = 0;
uint32_t            Iec104_TimeCycle_TesterIdle = 0;
uint8_t             Iec104_TesterCount = 0;

uint32_t            Iec10x_Update_SeekAddr = 0;
uint16_t            FirmFlagCount = 0;

uint8_t IEC104_UploadAddr(void){

    uint8_t len = 0;
    uint8_t *ptr = NULL;
    uint32_t Temp32 = 0;

    PIEC104_DATA_T Iec104Data = (PIEC104_DATA_T)Iec104_Sendbuf;
    PIEC10X_ASDU_T asdu = (PIEC10X_ASDU_T)(Iec104Data->Asdu);
    PASDU_INFO_T info = (PASDU_INFO_T)(asdu->_info);

    /*build head*/
    Iec104Data->Head = IEC104_HEAD;

    /*build control code*/
    Iec104Data->Ctrl.I.Type = 0;
    Iec104Data->Ctrl.I.SendSn = Iec104_BuildSendSn++;
    Iec104Data->Ctrl.I.RecvSn = Iec104_BuildRecvSn;

    /*build ASDU , COT ,Addr*/
    asdu->_type = IEC104_ASDU_TYPE_M_DTU_INF_1;
    asdu->_num._sq = 0;
    asdu->_num._num = 1;
    asdu->_reason._reason = IEC10X_ASDU_COT_UNKNOW;
    asdu->_addr = Iec10x_Sta_Addr;

    /*build info*/
    ptr = info->_addr;
    Temp32 = 0;
    memcpy(ptr, &Temp32, 3);

    ptr = info->_element;
    Temp32 = IEC104_INFO_SIGNATURE;
    memcpy(ptr, &Temp32, 4);

    ptr+=4;
    Temp32 = Iec10x_Sta_Addr;
    memcpy(ptr, &Temp32, 2);

    ptr+=2;

    /*len*/
    len = ptr - Iec104_Sendbuf;
    Iec104Data->Len = len - 2;

    //DumpHEX(Iec101_Sendbuf,len);
    /* enqueue to the transmisson queue */
    IEC10X_Enqueue(Iec104_Sendbuf, len ,IEC10X_PRIO_INITLINK, NULL,NULL);

    return RET_SUCESS;
}

uint8_t IEC104_Build_InitFin(void){

    uint8_t len = 0;
    uint8_t *ptr = NULL;
    uint32_t Temp32 = 0;

    PIEC104_DATA_T Iec104Data = (PIEC104_DATA_T)Iec104_Sendbuf;
    PIEC10X_ASDU_T asdu = (PIEC10X_ASDU_T)(Iec104Data->Asdu);
    PASDU_INFO_T info = (PASDU_INFO_T)(asdu->_info);

    /*build head*/
    Iec104Data->Head = IEC104_HEAD;

    /*build control code*/
    Iec104Data->Ctrl.I.Type = 0;
    Iec104Data->Ctrl.I.SendSn = Iec104_BuildSendSn++;
    Iec104Data->Ctrl.I.RecvSn = Iec104_BuildRecvSn;

    /*build ASDU , COT ,Addr*/
    asdu->_type = Iec10x_M_EI_NA_1;
    asdu->_num._sq = 0;
    asdu->_num._num = 1;
    asdu->_reason._reason = IEC10X_ASDU_REASON_INIT;
    asdu->_addr = Iec10x_Sta_Addr;

    /*build info*/
    ptr = info->_addr;
    Temp32 = 0;
    memcpy(ptr, &Temp32, 3);

    ptr = info->_element;
    //Temp32 = 0;
    //memcpy(ptr, &Temp32, 4);
    info->_element[0] = 0;

    ptr++;

    /*len*/
    len = ptr - Iec104_Sendbuf;
    Iec104Data->Len = len - 2;

    //DumpHEX(Iec101_Sendbuf,len);
    /* enqueue to the transmisson queue */
    IEC10X_Enqueue(Iec104_Sendbuf, len ,IEC10X_PRIO_INITLINK, NULL,NULL);

    return RET_SUCESS;
}

uint8_t IEC104_BuildCallACK(uint8_t qoi,uint8_t Prio){

    uint8_t len = 0;
    uint8_t *ptr = NULL;
    uint32_t Temp32 = 0;

    PIEC104_DATA_T Iec104Data = (PIEC104_DATA_T)Iec104_Sendbuf;
    PIEC10X_ASDU_T asdu = (PIEC10X_ASDU_T)(Iec104Data->Asdu);
    PASDU_INFO_T info = (PASDU_INFO_T)(asdu->_info);

    /*build head*/
    Iec104Data->Head = IEC104_HEAD;

    /*build control code*/
    Iec104Data->Ctrl.I.Type = 0;
    Iec104Data->Ctrl.I.SendSn = Iec104_BuildSendSn++;
    Iec104Data->Ctrl.I.RecvSn = Iec104_BuildRecvSn;

    /*build ASDU , COT ,Addr*/
    asdu->_type = IEC10X_C_IC_NA_1;
    asdu->_num._sq = 0;
    asdu->_num._num = 1;
    asdu->_reason._reason = IEC10X_ASDU_REASON_ACTCON;
    asdu->_addr = Iec10x_Sta_Addr;

    /*build info*/
    ptr = info->_addr;
    Temp32 = 0;
    memcpy(ptr, &Temp32, 3);

    ptr = info->_element;
    ptr[0] = qoi;

    ptr+=1;
    /*len*/
    len = ptr - Iec104_Sendbuf;
    Iec104Data->Len = len - 2;
    
    DumpHEX(Iec104_Sendbuf,len);
    /* enqueue to the transmisson queue */
    IEC10X_Enqueue(Iec104_Sendbuf, len ,Prio, NULL,NULL);

    return RET_SUCESS;
}

uint8_t IEC104_BuildSignal_Spon(uint8_t TimeFlag, uint8_t signalV, uint16_t addrV){

    uint8_t len = 0, asdu_num = 0, i;
    uint8_t *ptr = NULL;
    uint32_t Temp32 = 0;
    PCP56Time2a_T time = NULL;

    PIEC104_DATA_T Iec104Data = (PIEC104_DATA_T)Iec104_Sendbuf;
    PIEC10X_ASDU_T asdu = (PIEC10X_ASDU_T)(Iec104Data->Asdu);
    PASDU_INFO_T info = (PASDU_INFO_T)(asdu->_info);

    /* check Time flag */
    if(TimeFlag != 1 && TimeFlag != 0){
        LOG("-%s-, error time flag(%d) \n",__FUNCTION__,TimeFlag);
        return RET_ERROR;
    }
    LOG("-%s-, time flag(%d) signalV(%d) \n",__FUNCTION__,TimeFlag,signalV);
    /*get value*/
    asdu_num = 1;

    /*build head*/
    Iec104Data->Head = IEC104_HEAD;

    /*build control code*/
    Iec104Data->Ctrl.I.Type = 0;
    Iec104Data->Ctrl.I.SendSn = Iec104_BuildSendSn++;
    Iec104Data->Ctrl.I.RecvSn = Iec104_BuildRecvSn;

    /*build ASDU , COT ,Addr*/
    if(TimeFlag == 0)
        asdu->_type = IEC10X_M_SP_NA_1;
    else
        asdu->_type = IEC10X_M_SP_TB_1;
    asdu->_num._sq = 1;
    asdu->_num._num = asdu_num;
    asdu->_reason._reason = IEC10X_COT_SPONT;
    asdu->_addr = Iec10x_Sta_Addr;

    /*build info addr*/
    ptr = info->_addr;
    Temp32 = addrV;
    memcpy(ptr, &Temp32, 3);

    /*build info value*/
    ptr = info->_element;
    for(i=0; i<asdu_num; i++){
        *ptr = signalV;
        ptr++;
    }
    if(TimeFlag == 1){
        time = (PCP56Time2a_T)ptr;
        IEC10X->GetTime(time);
        ptr += sizeof(CP56Time2a_T);
    }

    /*len*/
    len = ptr - Iec104_Sendbuf;
    Iec104Data->Len = len - 2;

    //DumpHEX(Iec101_Sendbuf,len);
    /* enqueue to the transmisson queue */
    IEC10X_Enqueue(Iec104_Sendbuf, len ,IEC10X_PRIO_SPON, NULL,NULL);

    return RET_SUCESS;
}
uint8_t IEC104_BuildDetect_Spont(uint8_t TimeFlag, PIEC10X_DETECT_T detectV, uint16_t addrV){

    uint8_t len = 0, asdu_num = 0;
    uint8_t *ptr = NULL;
    uint32_t Temp32 = 0;
    PIEC10X_DETECT_T detect = NULL;
    PCP56Time2a_T time = NULL;

    PIEC104_DATA_T Iec104Data = (PIEC104_DATA_T)Iec104_Sendbuf;
    PIEC10X_ASDU_T asdu = (PIEC10X_ASDU_T)(Iec104Data->Asdu);
    PASDU_INFO_T info = (PASDU_INFO_T)(asdu->_info);

    /* check Time flag */
    if(TimeFlag != 1 && TimeFlag != 0){
        LOG("-%s-, error time flag(%d) \n",__FUNCTION__,TimeFlag);
        return RET_ERROR;
    }
    /*get value*/
    asdu_num = 1;

    /*build head*/
    Iec104Data->Head = IEC104_HEAD;

    /*build control code*/
    Iec104Data->Ctrl.I.Type = 0;
    Iec104Data->Ctrl.I.SendSn = Iec104_BuildSendSn++;
    Iec104Data->Ctrl.I.RecvSn = Iec104_BuildRecvSn;

    /*build ASDU , COT ,Addr*/
    if(TimeFlag == 0)
        asdu->_type = IEC10X_M_ME_NA_1;
    else
        asdu->_type = IEC10X_M_ME_TD_1;
    asdu->_num._sq = 1;
    asdu->_num._num = asdu_num;
    asdu->_reason._reason = IEC10X_COT_SPONT;
    asdu->_addr = Iec10x_Sta_Addr;

    /*build info addr*/
    ptr = info->_addr;
    Temp32 = addrV;
    memcpy(ptr, &Temp32, 3);

    /*build info value*/
    ptr = info->_element;
    detect = (PIEC10X_DETECT_T)ptr;
    detect->_detect = detectV->_detect;
    detect->_qds = detectV->_qds;
    ptr += sizeof(IEC10X_DETECT_T);
    if(TimeFlag == 1){
        time = (PCP56Time2a_T)ptr;
        IEC10X->GetTime(time);
        ptr += sizeof(CP56Time2a_T);
    }

    /*len*/
    len = ptr - Iec104_Sendbuf;
    Iec104Data->Len = len - 2;

    //DumpHEX(Iec101_Sendbuf,len);
    /* enqueue to the transmisson queue */
    IEC10X_Enqueue(Iec104_Sendbuf, len ,IEC10X_PRIO_SPON, NULL,NULL);

    return RET_SUCESS;
}

uint8_t IEC104_BuildDetectF_Spont(uint8_t TimeFlag, float detectV, uint16_t addrV){

    uint8_t len = 0, asdu_num = 0;
    uint8_t *ptr = NULL;
    uint32_t Temp32 = 0;
    PIEC10X_DETECT_F_T detect = NULL;
    PCP56Time2a_T time = NULL;

    PIEC104_DATA_T Iec104Data = (PIEC104_DATA_T)Iec104_Sendbuf;
    PIEC10X_ASDU_T asdu = (PIEC10X_ASDU_T)(Iec104Data->Asdu);
    PASDU_INFO_T info = (PASDU_INFO_T)(asdu->_info);

    /* check Time flag */
    if(TimeFlag != 1 && TimeFlag != 0){
        LOG("-%s-, error time flag(%d) \n",__FUNCTION__,TimeFlag);
        return RET_ERROR;
    }
    /*get value*/
    asdu_num = 1;

    /*build head*/
    Iec104Data->Head = IEC104_HEAD;

    /*build control code*/
    Iec104Data->Ctrl.I.Type = 0;
    Iec104Data->Ctrl.I.SendSn = Iec104_BuildSendSn++;
    Iec104Data->Ctrl.I.RecvSn = Iec104_BuildRecvSn;

    /*build ASDU , COT ,Addr*/
    if(TimeFlag == 0)
        asdu->_type = IEC10X_M_ME_NC_1;
    else
        asdu->_type = IEC10X_M_ME_TF_1;
    asdu->_num._sq = 1;
    asdu->_num._num = asdu_num;
    asdu->_reason._reason = IEC10X_COT_SPONT;
    asdu->_addr = Iec10x_Sta_Addr;

    /*build info addr*/
    ptr = info->_addr;
    Temp32 = addrV;
    memcpy(ptr, &Temp32, 3);

    /*build info value*/
    ptr = info->_element;
    detect = (PIEC10X_DETECT_F_T)ptr;
    detect->_detect = detectV;
    detect->_qds = 0;
    ptr += sizeof(IEC10X_DETECT_F_T);
    if(TimeFlag == 1){
        time = (PCP56Time2a_T)ptr;
        IEC10X->GetTime(time);
        ptr += sizeof(CP56Time2a_T);
    }

    /*len*/
    len = ptr - Iec104_Sendbuf;
    Iec104Data->Len = len - 2;

    //DumpHEX(Iec101_Sendbuf,len);
    /* enqueue to the transmisson queue */
    IEC10X_Enqueue(Iec104_Sendbuf, len ,IEC10X_PRIO_SPON, NULL,NULL);

    return RET_SUCESS;
}
uint8_t IEC104_BuildSignal(uint8_t reason,uint8_t Prio, uint8_t DevType){

    uint8_t len = 0, asdu_num = 0;
    uint16_t i = 0;
    uint8_t *ptr = NULL;
    uint32_t Temp32 = 0;

    PIEC104_DATA_T Iec104Data = (PIEC104_DATA_T)Iec104_Sendbuf;
    PIEC10X_ASDU_T asdu = (PIEC10X_ASDU_T)(Iec104Data->Asdu);
    PASDU_INFO_T info = (PASDU_INFO_T)(asdu->_info);

    /*get value*/
    IEC10X->GetInfoNum(&asdu_num, DevType);
    LOG("-%s- total info (%d) \n\n",__FUNCTION__, asdu_num);

    /*build head*/
    Iec104Data->Head = IEC104_HEAD;

    /*build control code*/
    Iec104Data->Ctrl.I.Type = 0;
    Iec104Data->Ctrl.I.SendSn = Iec104_BuildSendSn++;
    Iec104Data->Ctrl.I.RecvSn = Iec104_BuildRecvSn;

    /*build ASDU , COT ,Addr*/
    asdu->_type = IEC10X_M_SP_NA_1;
    asdu->_num._sq = 1;
    asdu->_num._num = asdu_num;
    asdu->_reason._reason = reason;
    asdu->_addr = Iec10x_Sta_Addr;

    /*build info addr*/
    ptr = info->_addr;
    if(DevType == ENDDEVICE_TYPE_HXTM){
        Temp32 = IEC104_INFOADDR_STATE_HXTM;
    }
    else if(DevType == ENDDEVICE_TYPE_HXGF){
        Temp32 = IEC104_INFOADDR_STATE_HXGF;
    }
    else{
        LOG("-%s-, error dev type:%d \n",__FUNCTION__,DevType);
        return RET_ERROR;
    }

    memcpy(ptr, &Temp32, 3);

    /*build info value*/
    ptr = info->_element;
    for(i=0; i<asdu_num; i++){
        *ptr = IEC10X->GetStationState(i, DevType);
        ptr++;
    }

    /*len*/
    len = ptr - Iec104_Sendbuf;
    Iec104Data->Len = len - 2;

    //DumpHEX(Iec101_Sendbuf,len);
    /* enqueue to the transmisson queue */
    IEC10X_Enqueue(Iec104_Sendbuf, len ,Prio, NULL,NULL);

    return RET_SUCESS;
}

uint8_t IEC104_BuildDetect(uint8_t reason,uint8_t ValueType, uint8_t Prio, uint8_t DevType){

    uint8_t len = 0, asdu_num = 0, i;
    uint8_t *ptr = NULL;
    uint32_t Temp32 = 0;
    PIEC10X_DETECT_T detect = NULL;
    PIEC10X_DETECT_F_T detect_f = NULL;

    PIEC104_DATA_T Iec104Data = (PIEC104_DATA_T)Iec104_Sendbuf;
    PIEC10X_ASDU_T asdu = (PIEC10X_ASDU_T)(Iec104Data->Asdu);
    PASDU_INFO_T info = (PASDU_INFO_T)(asdu->_info);

    /*get value*/
    IEC10X->GetInfoNum(&asdu_num, DevType);
    LOG("-%s- total info (%d) \n\n",__FUNCTION__, asdu_num);

    /*build head*/
    Iec104Data->Head = IEC104_HEAD;

    /*build control code*/
    Iec104Data->Ctrl.I.Type = 0;
    Iec104Data->Ctrl.I.SendSn = Iec104_BuildSendSn++;
    Iec104Data->Ctrl.I.RecvSn = Iec104_BuildRecvSn;

    /*build ASDU , COT ,Addr*/
    asdu->_type = ValueType;
    asdu->_num._sq = 1;
    asdu->_num._num = asdu_num;
    asdu->_reason._reason = reason;
    asdu->_addr = Iec10x_Sta_Addr;

    /*build info addr*/
    ptr = info->_addr;
    if(DevType == ENDDEVICE_TYPE_HXTM){
        Temp32 = IEC104_INFOADDR_VALUE_HXTM;
    }
    else if(DevType == ENDDEVICE_TYPE_HXGF){
        Temp32 = IEC104_INFOADDR_VALUE_HXGF;
    }
    else{
        LOG("-%s-, error dev type:%d \n",__FUNCTION__,DevType);
        return RET_ERROR;
    }
    memcpy(ptr, &Temp32, 3);

    /*Build Detect value*/
    ptr = info->_element;
    for(i=0; i<asdu_num; i++){
        /*short int*/
        if(ValueType == IEC10X_M_ME_NA_1){
            detect = (PIEC10X_DETECT_T)ptr;
            detect->_detect = IEC10X->GetStaValue(i, DevType);
            detect->_qds = 0;
            ptr += sizeof(IEC10X_DETECT_T);
        }
        /*float*/
        else if(ValueType == IEC10X_M_ME_NC_1){
            detect_f = (PIEC10X_DETECT_F_T)ptr;
            detect_f->_detect = IEC10X->GetStaValue(i, DevType);
            detect_f->_qds = 0;
            ptr += sizeof(IEC10X_DETECT_F_T);
        }
    }

    /*len*/
    len = ptr - Iec104_Sendbuf;
    Iec104Data->Len = len - 2;

    //DumpHEX(Iec101_Sendbuf,len);
    /* enqueue to the transmisson queue */
    IEC10X_Enqueue(Iec104_Sendbuf, len ,Prio, NULL,NULL);

    return RET_SUCESS;
}

uint8_t IEC104_BuildUpload(uint8_t ValueType, uint8_t Prio, uint8_t DevType){

    uint8_t len = 0, asdu_num = 0, i;
    uint8_t *ptr = NULL;
    uint32_t Temp32 = 0;
    PIEC10X_DETECT_T detect = NULL;

    PIEC104_DATA_T Iec104Data = (PIEC104_DATA_T)Iec104_Sendbuf;
    PIEC10X_ASDU_T asdu = (PIEC10X_ASDU_T)(Iec104Data->Asdu);
    PASDU_INFO_T info = (PASDU_INFO_T)(asdu->_info);

    /*get value*/
    IEC10X->GetInfoNum(&asdu_num, DevType);
    LOG("-%s- total info (%d) \n\n",__FUNCTION__, asdu_num);

    /*build head*/
    Iec104Data->Head = IEC104_HEAD;

    /*build control code*/
    Iec104Data->Ctrl.I.Type = 0;
    Iec104Data->Ctrl.I.SendSn = Iec104_BuildSendSn++;
    Iec104Data->Ctrl.I.RecvSn = Iec104_BuildRecvSn;

    /*build ASDU , COT ,Addr*/
    asdu->_type = ValueType;
    asdu->_num._sq = 1;
    asdu->_num._num = asdu_num;
    asdu->_reason._reason = AP_COT_BASE_INFO;
    asdu->_addr = Iec10x_Sta_Addr;

    /*build info addr*/
    ptr = info->_addr;
    
    Temp32 = IEC104_INFOADDR_BASE_DEVINFO;
    memcpy(ptr, &Temp32, 3);
    ptr += 3;
    /*Build Detect value*/
    ptr = info->_element;
    for(i=0; i<asdu_num; i++){

        detect = (PIEC10X_DETECT_T)ptr;
        detect->_detect = IEC10X->GetStaValue(i, DevType);
        detect->_qds = 0;
        ptr += sizeof(IEC10X_DETECT_T);
    }
    /*len*/
    len = ptr - Iec104_Sendbuf;
    Iec104Data->Len = len - 2;

    //DumpHEX(Iec101_Sendbuf,len);
    /* enqueue to the transmisson queue */
    IEC10X_Enqueue(Iec104_Sendbuf, len ,Prio, NULL,NULL);

    return RET_SUCESS;
}


uint8_t IEC104_BuildActFinish(uint8_t qoi, uint8_t Prio){

    uint8_t len = 0, asdu_num = 0;
    uint8_t *ptr = NULL;
    uint32_t Temp32 = 0;

    PIEC104_DATA_T Iec104Data = (PIEC104_DATA_T)Iec104_Sendbuf;
    PIEC10X_ASDU_T asdu = (PIEC10X_ASDU_T)(Iec104Data->Asdu);
    PASDU_INFO_T info = (PASDU_INFO_T)(asdu->_info);

    /*build head*/
    Iec104Data->Head = IEC104_HEAD;

    /*build control code*/
    Iec104Data->Ctrl.I.Type = 0;
    Iec104Data->Ctrl.I.SendSn = Iec104_BuildSendSn++;
    Iec104Data->Ctrl.I.RecvSn = Iec104_BuildRecvSn;

    /*build ASDU , COT ,Addr*/
    asdu->_type = IEC10X_C_IC_NA_1;
    asdu->_num._sq = 1;
    asdu->_num._num = asdu_num;
    asdu->_reason._reason = IEC10X_ASDU_REASON_ACTTERM;
    asdu->_addr = Iec10x_Sta_Addr;

    /*build info addr*/
    ptr = info->_addr;
    Temp32 = IEC10X_INFO_ADDR_SIG_BASE+IEC10X_INFO_ADDR_SIG_TEMP_HX_OFF;
    memcpy(ptr, &Temp32, 3);

    /*Build Detect value*/
    ptr = info->_element;
    ptr[0] = qoi;

    ptr+=1;
    /*len*/
    len = ptr - Iec104_Sendbuf;
    Iec104Data->Len = len - 2;

    //DumpHEX(Iec101_Sendbuf,len);
    /* enqueue to the transmisson queue */
    IEC10X_Enqueue(Iec104_Sendbuf, len ,Prio, NULL,NULL);

    return RET_SUCESS;
}

uint8_t IEC104_Build_U(uint8_t UType, uint8_t Ack){

    uint8_t len = 0, Tester, Start, Stop;
    uint8_t *ptr = NULL;

    PIEC104_DATA_T Iec104Data = (PIEC104_DATA_T)Iec104_Sendbuf;


    /*build head*/
    Iec104Data->Head = IEC104_HEAD;

    /*build control code*/
    if(Ack){
        Tester = IEC104_U_FUNC_TESTER_ACK;
        Start = IEC104_U_FUNC_STARTDT_ACK;
        Stop = IEC104_U_FUNC_STOPDT_ACK;
    }else{
        Tester = IEC104_U_FUNC_TESTER;
        Start = IEC104_U_FUNC_STARTDT;
        Stop = IEC104_U_FUNC_STOPDT;
    }

    switch(UType){

        case IEC104_U_FUNC_STARTDT:
            Iec104Data->Ctrl.Func.Func = Start;
            break;
        case IEC104_U_FUNC_STOPDT:
            Iec104Data->Ctrl.Func.Func = Stop;
            break;
        case IEC104_U_FUNC_TESTER:
            Iec104Data->Ctrl.Func.Func = Tester;
            break;
        default:
            LOG(">%s<, U Type Error(%d) !\n",__FUNCTION__,UType);
            return RET_ERROR;
    }


    /*build ASDU , COT ,Addr*/
    ptr=Iec104Data->Asdu;

    /*build info*/

    /*len*/
    len = ptr - Iec104_Sendbuf;
    Iec104Data->Len = len - 2;
    //DumpHEX(Iec101_Sendbuf,len);
    /* enqueue to the transmisson queue */
    IEC10X_Enqueue(Iec104_Sendbuf, len ,IEC10X_PRIO_INITLINK, NULL,NULL);

    return RET_SUCESS;
}


uint8_t IEC104_Build_S_Ack(void){

    uint8_t len = 0;
    uint8_t *ptr = NULL;

    PIEC104_DATA_T Iec104Data = (PIEC104_DATA_T)Iec104_Sendbuf;

    /*build head*/
    Iec104Data->Head = IEC104_HEAD;

    /*build control code*/
    Iec104Data->Ctrl.S.Type1 = 1;
    Iec104Data->Ctrl.S.Type2 = 0;

    Iec104Data->Ctrl.S.Reserve = 0;
    Iec104Data->Ctrl.S.RecvSn = Iec104_BuildRecvSn;

    /*build ASDU , COT ,Addr*/
    ptr=Iec104Data->Asdu;

    /*build info*/

    /*len*/
    len = ptr - Iec104_Sendbuf;
    Iec104Data->Len = len - 2;
    //DumpHEX(Iec101_Sendbuf,len);
    /* enqueue to the transmisson queue */
    IEC10X_Enqueue(Iec104_Sendbuf, len ,IEC10X_PRIO_INITLINK, NULL,NULL);

    return RET_SUCESS;
}
uint8_t IEC104_ASDU_Call(PIEC10X_ASDU_T Iec10x_Asdu){

    PASDU_INFO_T asdu_info = (PASDU_INFO_T)(Iec10x_Asdu->_info);
    uint8_t qoi = asdu_info->_element[0];
    uint8_t Prio = 0;

    uint32_t InfoAddr = 0;

    /* check info addrest */
    memcpy(&InfoAddr, asdu_info->_addr, 3);
    if(InfoAddr != 0){
        LOG("-%s- call cmd active error addr(%x) \n" ,__FUNCTION__ ,InfoAddr);
        return RET_ERROR;
    }

    switch(Iec10x_Asdu->_reason._reason){

        case IEC10X_ASDU_REASON_ACT:
            switch(qoi){
                case IEC10X_CALL_QOI_TOTAL:
                    Prio = IEC10X_PRIO_CALLALL;
                    IEC104_STATE_FLAG_CALLALL = IEC104_FLAG_CALL_ALLDATA;
                    IEC104_Call_AllQoi = qoi;
                    break;
                case IEC10X_CALL_QOI_GROUP1:
                case IEC10X_CALL_QOI_GROUP2:
                case IEC10X_CALL_QOI_GROUP9:
                case IEC10X_CALL_QOI_GROUP10:
                    Prio = IEC10X_PRIO_CALLGROUP;
                    IEC104_STATE_FLAG_GROUP = IEC101_FLAG_CALL_GROURPDATA;
                    IEC104_Call_GroupQoi = qoi;
                    break;
                default:
                    LOG("-%s- call cmd error qoi(%d) \n", __FUNCTION__,qoi);
                    return RET_ERROR;
            }
            IEC104_BuildCallACK(qoi,Prio);
            /**/
            IEC104_BuildSignal(qoi,Prio, ENDDEVICE_TYPE_HXTM);
            IEC104_BuildDetect(qoi ,IEC10X_M_ME_NA_1,Prio, ENDDEVICE_TYPE_HXTM);
            /**/
            IEC104_BuildSignal(qoi,Prio, ENDDEVICE_TYPE_HXGF);
            IEC104_BuildDetect(qoi ,IEC10X_M_ME_NA_1,Prio, ENDDEVICE_TYPE_HXGF);
            IEC104_BuildActFinish(qoi, Prio);
            break;

        default:
            LOG("-%s- call cmd error reason(%d) \n", __FUNCTION__,Iec10x_Asdu->_reason._reason);
            break;
    }
    return RET_SUCESS;
}
uint8_t Iec104_Aadu_Clock(PIEC10X_ASDU_T Iec10x_Asdu){

    PASDU_INFO_T asdu_info = (PASDU_INFO_T)(Iec10x_Asdu->_info);

    memcpy(&IEC10X_Cp56time2a,asdu_info->_element, sizeof(CP56Time2a_T));

    if(asdu_info->_addr[0] != 0 || asdu_info->_addr[1] != 0 || asdu_info->_addr[2] != 0){
        LOG("-%s- Clock cmd error addr(0x%02x:%02x:%02x) \n", __FUNCTION__,asdu_info->_addr[0],asdu_info->_addr[2],asdu_info->_addr[2]);
        return RET_ERROR;
    }

    switch(Iec10x_Asdu->_reason._reason){

        case IEC10X_COT_ACT:
            LOG("-%s- Clock cmd (20%d-%d-%d %d %d:%d:%d) \n", __FUNCTION__,IEC10X_Cp56time2a._year._year,IEC10X_Cp56time2a._month._month,IEC10X_Cp56time2a._day._dayofmonth,
                    IEC10X_Cp56time2a._day._dayofweek,IEC10X_Cp56time2a._hour._hours,IEC10X_Cp56time2a._min._minutes,IEC10X_Cp56time2a._milliseconds);
            /*get time*/
            /*...*/
            IEC10X->SetTime(&IEC10X_Cp56time2a);
            //IEC104_Build_S_Ack();

            break;
        case IEC10X_COT_SPONT:
            LOG("-%s- Clock cmd spont \n", __FUNCTION__);
            break;
        default:
            LOG("-%s- Clock cmd error reason(%d) \n", __FUNCTION__,Iec10x_Asdu->_reason._reason);
            break;
    }
    return RET_SUCESS;
}

uint8_t IEC104_ASDU_SetAct(PIEC10X_ASDU_T Iec10x_Asdu, uint8_t Type){

    PASDU_INFO_T asdu_info = (PASDU_INFO_T)(Iec10x_Asdu->_info);
    uint8_t *ptr = NULL;
    uint8_t n = Iec10x_Asdu->_num._num, Sq = Iec10x_Asdu->_num._sq, i;
    float Value = 0.0;
    uint32_t InfoAddr = 0;
            
    /* if sq == 1 */
    PIEC10X_DETECT_T detect = NULL;
    PIEC10X_DETECT_F_T detect_f = NULL;
  
    /* if sq == 0 */  
    PIEC10X_DETECT_SQ0_T detect_Sq0 = NULL;
    PIEC10X_DETECT_SQ0_F_T detect_Sq0_f = NULL;

    /* check info addrest */
    memcpy(&InfoAddr, asdu_info->_addr, 3);

    switch(Iec10x_Asdu->_reason._reason){

        case IEC10X_COT_ACT:
        
            switch(Type){
                case IEC10X_C_SE_NA_1:
                
                    if(Sq == 1){
                        ptr = asdu_info->_element;
                        for(i=0; i<n; i++){

                            detect = (PIEC10X_DETECT_T)ptr;
                            Value = (float)(detect->_detect);
                            ptr += sizeof(IEC10X_DETECT_T);
                            IEC10X->SetConfig(Value, InfoAddr+i);
                        }
                    }else if(Sq == 0){
                        ptr = Iec10x_Asdu->_info;
                        for(i=0; i<n; i++){

                            detect_Sq0 = (PIEC10X_DETECT_SQ0_T)ptr;
                            Value = (float)(detect_Sq0->_detect);
                            InfoAddr = 0;
                            memcpy(&InfoAddr, detect_Sq0->_addr, 3);
                            IEC10X->SetConfig(Value, InfoAddr);
                            ptr += sizeof(IEC10X_DETECT_SQ0_T);   
                        }
                        
                    }
                    break;
                case IEC10X_C_SE_NC_1:
                    if(Sq == 1){
                        ptr = asdu_info->_element;
                        for(i=0; i<n; i++){
                   
                            detect_f = (PIEC10X_DETECT_F_T)ptr;
                            Value = detect_f->_detect;
                            ptr += sizeof(IEC10X_DETECT_F_T);
                            IEC10X->SetConfig(Value, InfoAddr+i);
                        }
                    }else if(Sq == 0){
                        ptr = Iec10x_Asdu->_info;
                        for(i=0; i<n; i++){
                            detect_Sq0_f = (PIEC10X_DETECT_SQ0_F_T)ptr;
                            Value = (float)(detect_Sq0_f->_detect);
                            memcpy(&InfoAddr, detect_Sq0_f->_addr, 3);
                            IEC10X->SetConfig(Value, InfoAddr);
                            ptr += sizeof(IEC10X_DETECT_SQ0_F_T); 
                        }
                    }
                    break;
                default:
                    LOG("-%s-, Type error !",__FUNCTION__);
                    return RET_ERROR;
            }
            break;

        default:
            LOG("-%s- , error reason(%d) \n", __FUNCTION__,Iec10x_Asdu->_reason._reason);
            return RET_ERROR;
    }
    return RET_SUCESS;
}

uint8_t IEC104_Build_SetAck(uint8_t Prio, uint8_t Type){

    uint8_t len = 0;
    uint8_t *ptr = NULL;

    PIEC104_DATA_T Iec104Data = (PIEC104_DATA_T)Iec104_Sendbuf;
    PIEC10X_ASDU_T asdu = (PIEC10X_ASDU_T)(Iec104Data->Asdu);
    PASDU_INFO_T info = (PASDU_INFO_T)(asdu->_info);

    /*build head*/
    Iec104Data->Head = IEC104_HEAD;

    /*build control code*/
    Iec104Data->Ctrl.I.Type = 0;
    Iec104Data->Ctrl.I.SendSn = Iec104_BuildSendSn++;
    Iec104Data->Ctrl.I.RecvSn = Iec104_BuildRecvSn;

    /*build ASDU , COT ,Addr*/
    asdu->_type = Type;
    asdu->_num._sq = 0;
    asdu->_num._num = 0;
    asdu->_reason._reason = IEC10X_ASDU_REASON_ACTFIN;
    asdu->_addr = Iec10x_Sta_Addr;

    /*build info*/
    ptr = info->_addr;

    /*len*/
    len = ptr - Iec104_Sendbuf;
    Iec104Data->Len = len - 2;
    
    //DumpHEX(Iec104_Sendbuf,len);
    /* enqueue to the transmisson queue */
    IEC10X_Enqueue(Iec104_Sendbuf, len ,Prio, NULL,NULL);

    return RET_SUCESS;
}

uint8_t Iec104_Deal_SN(uint16_t SendSn, uint16_t RecvSn){

    LOG("Receive Pakage I(%d,%d), Send(%d,%d)\n",SendSn,RecvSn, Iec104_BuildSendSn,Iec104_BuildRecvSn);
    
#if 0
    if(SendSn > Iec104_DealSendSn+1){
        LOG("-%s-, error,send last(%d),now(%d) \n",__FUNCTION__,Iec104_DealSendSn,SendSn);
        IEC104_STATE_FLAG_INIT = IEC104_FLAG_SEND_CLOSED;
        return RET_ERROR;
    }else if(SendSn < Iec104_DealSendSn+1){
        LOG("-%s-, Retransmit,send last(%d),now(%d) \n",__FUNCTION__,Iec104_DealSendSn,SendSn);
        return RET_ERROR;
    }
    if(RecvSn != Iec104_BuildSendSn){
        LOG("-%s-, error,receive last(%d),now(%d) \n",__FUNCTION__,Iec104_BuildSendSn,RecvSn);
        IEC104_STATE_FLAG_INIT = IEC104_FLAG_SEND_CLOSED;
        return RET_ERROR;
    }
    if(RecvSn < Iec104_DealRecvSn){
        LOG("-%s-, error,receive2 last(%d),now(%d) \n",__FUNCTION__,Iec104_DealRecvSn,RecvSn);
        return RET_ERROR;
    }
#endif

    if(SendSn < Iec104_DealSendSn || RecvSn < Iec104_DealRecvSn){

        LOG("-%s-, error,send last(%d),now(%d). recv last(%d),now(%d) \n",__FUNCTION__,
                        Iec104_DealSendSn,SendSn, Iec104_DealRecvSn, RecvSn);
        return RET_ERROR;
    }
    Iec104_BuildRecvSn = SendSn+1;

    Iec104_DealSendSn = SendSn;
    Iec104_DealRecvSn = RecvSn;

    //Iec104_BuildRecvSn++;

    /* return S ACK */
    IEC104_STATE_FLAG_S_ACK = IEC104_FLAG_S_ACK;
    Iec104_TimeCycle_S = 0;

    return RET_SUCESS;
}
int8_t Iec104_BuildDataAck(uint8_t TI, uint16_t COT, uint32_t InfoAddr, uint16_t Info, uint8_t Prio){

    uint8_t len = 0;
    uint8_t *ptr = NULL;
    uint32_t Temp32 = 0;

    /* build head */
    PIEC104_DATA_T Iec104Data = (PIEC104_DATA_T)Iec104_Sendbuf;
    PIEC10X_ASDU_T asdu = (PIEC10X_ASDU_T)(Iec104Data->Asdu);
    PASDU_INFO_T info = (PASDU_INFO_T)(asdu->_info);

    /*build head*/
    Iec104Data->Head = IEC104_HEAD;

    /*build control code*/
    Iec104Data->Ctrl.I.Type = 0;
    Iec104Data->Ctrl.I.SendSn = Iec104_BuildSendSn++;
    Iec104Data->Ctrl.I.RecvSn = Iec104_BuildRecvSn;

    /*build ASDU , COT ,Addr*/
    asdu->_type = TI;
    asdu->_num._sq = 0;
    asdu->_num._num = 1;
    asdu->_reason._reason = COT;
    asdu->_addr = Iec10x_Sta_Addr;

    /*build info*/
    ptr = info->_addr;
    Temp32 = InfoAddr;
    memcpy(ptr, &Temp32, 3);

    ptr = info->_element;
    if(COT == IEC10X_COT_ACT_TERMINAL || COT == IEC10X_COT_ACTCON ||COT == IEC10X_COT_ACT_TERMINAL_ACK ||COT == IEC10X_TI_AP_FIRM_BACKOFF){
        ptr[0] = 0;
        ptr[1] = 0;
        ptr+=2;
    }else if(COT == IEC10X_COT_DATA_ACK || COT == IEC10X_COT_DATA_FIN_ACK){
        memcpy(ptr, &Info, 2);
        ptr+=2;
    }

    /*len*/
    len = ptr - Iec104_Sendbuf;
    Iec104Data->Len = len - 2;
    
    //DumpHEX(Iec104_Sendbuf,len);
    /* enqueue to the transmisson queue */
    IEC10X_Enqueue(Iec104_Sendbuf, len ,Prio, NULL,NULL);

    return RET_SUCESS;
}

int8_t Iec104_Deal_FirmUpdate(PIEC10X_ASDU_T asdu, uint8_t Len){
    
    uint16_t cot = asdu->_reason._reason;
    uint16_t FlagNum = 0, i;
    uint8_t DataLen = Len - IEC104_DATA_LEN - 3;        /* flag num 2, check sum 1 byte*/
    PASDU_INFO_T element = (PASDU_INFO_T)(asdu->_info);
    uint8_t *DataPtr = (uint8_t *)(element->_element) +3;
    uint8_t TI = asdu->_type, csum = 0, CsumTemp = 0;
    uint32_t FirmwareType = 0;
    int8_t ret = 0;
    
    PASDU_INFO_T info = (PASDU_INFO_T)(asdu->_info);

    FlagNum = *(uint16_t *)(info->_element);
    memcpy(&FirmwareType, asdu->_info,3);

    if(Len == 0){
        LOG("-%s-,data:%d,Len:%d error!\n",__FUNCTION__,FlagNum,DataLen);
        return RET_ERROR;
    }
    
    /* reset the flag counter */
    if(FlagNum == 1){
        FirmFlagCount = 0;
        Iec10x_Update_SeekAddr = 0;
    }

    switch(cot){
    
        case IEC10X_COT_DATA:
            
            if(FlagNum == FirmFlagCount+1){
            
                /* check sum */
                csum = info->_element[2];
                
                for(i=0; i<DataLen; i++){
                    CsumTemp += DataPtr[i];
                }
                if(CsumTemp == csum){
                    LOG("-%s-,data:%d,Len:%d,seek:%d \n",__FUNCTION__,FlagNum,DataLen,Iec10x_Update_SeekAddr);
                    for(i=0; i<3; i++){
                        ret = IEC10X->SaveFirmware(DataLen,DataPtr,FirmwareType, Iec10x_Update_SeekAddr);
                        if(ret == RET_SUCESS)
                            break;
                    }
                    if(ret == RET_ERROR){
                        LOG("save firmware error \n");
                        break;
                    }

                    FirmFlagCount = FlagNum;
                    Iec10x_Update_SeekAddr+=DataLen;
                }else{
                    LOG("%s, check sum error:%d,need:%d,num:%d\n",__FUNCTION__,CsumTemp,csum,FlagNum);
                }
            }else if(FlagNum < FirmFlagCount+1){
                LOG("update flag resend,need:%d,flag:%d\n",FirmFlagCount+1,FlagNum);
            }else{
                LOG("update flag error! need:%d,flag:%d\n",FirmFlagCount+1,FlagNum);
                //Iec104_BuildDataAck(TI, IEC10X_COT_ACT_TERMINAL, FirmwareType, FlagNum,1);
            }
            break;
            
        case IEC10X_COT_DATA_NEEDACK:
            if(FlagNum == FirmFlagCount+1){

                /* check sum */
                csum = info->_element[2];
                for(i=0; i<DataLen; i++){
                    CsumTemp += DataPtr[i];
                }
                if(CsumTemp == csum){
                    LOG("-%s-,data need ack:%d,Len:%d,seek:%d \n",__FUNCTION__,FlagNum,DataLen,Iec10x_Update_SeekAddr);
                    
                    for(i=0; i<3; i++){
                        ret = IEC10X->SaveFirmware(DataLen,DataPtr,FirmwareType,Iec10x_Update_SeekAddr);
                        if(ret == RET_SUCESS)
                            break;
                    }
                    
                    if(ret == RET_ERROR){
                        LOG("save firmware error \n");
                        break;
                    }
                    Iec104_BuildDataAck(TI, IEC10X_COT_DATA_ACK, FirmwareType, FlagNum,1);
                    
                    FirmFlagCount = FlagNum;
                    Iec10x_Update_SeekAddr+=DataLen;
                }else{
                    LOG("%s,need ack check sum error:%d,need:%d,num...:%d\n",__FUNCTION__,CsumTemp,csum,FlagNum);
                    //Iec104_BuildDataAck(TI, IEC10X_COT_ACT_TERMINAL, FirmwareType, FlagNum,1);
                }
            }else if(FlagNum < FirmFlagCount+1){
                LOG("update flag resend,need:%d,flag:%d\n",FirmFlagCount+1,FlagNum);
                Iec104_BuildDataAck(TI, IEC10X_COT_DATA_ACK, FirmwareType, FlagNum,1);
            }else{
                LOG("update flag error! need:%d,flag:%d\n",FirmFlagCount+1,FlagNum);
                //Iec104_BuildDataAck(TI, IEC10X_COT_ACT_TERMINAL, FirmwareType, FlagNum,1);
            }
            break;

        case IEC10X_COT_DATA_FIN:
            if(FirmFlagCount == FlagNum){
                LOG("-%s-,data finish:%d,Len:%d,Total Len:%d \n",__FUNCTION__,FlagNum,DataLen,Iec10x_Update_SeekAddr);
                ret = IEC10X->CheckFirmware(FirmwareType, Iec10x_Update_SeekAddr);
                if(ret == RET_SUCESS){
                    Iec104_BuildDataAck(TI, IEC10X_COT_DATA_FIN_ACK, FirmwareType, FlagNum,1);
                    IEC10X->UpdateFirmware(FirmwareType);
                }else{
                    /* check firmware error ,terminal update */
                    Iec104_BuildDataAck(TI, IEC10X_COT_ACT_TERMINAL, FirmwareType, 0,1);
                }
            }else{
                LOG("-%s-,data finish error:%d,Len:%d,Total Len:%d,FirmFlagCount:%d,FlagNum:%d, \n",__FUNCTION__,FlagNum,DataLen,Iec10x_Update_SeekAddr,FirmFlagCount,FlagNum);
                return RET_ERROR;
            }
            break;

        case IEC10X_COT_ACT_TERMINAL:
            LOG("-%s-, Terminal:%d,Len:%d \n",__FUNCTION__,FlagNum,DataLen);
            Iec104_BuildDataAck(TI, IEC10X_COT_ACT_TERMINAL_ACK, FirmwareType, 0,1);
            break;
        case IEC10X_COT_ACT_TERMINAL_ACK:
            LOG("-%s-, Terminal Ack \n",__FUNCTION__);
            break;
        default :
            LOG("-%s-,data:%d,Len:%d error cot: \n",__FUNCTION__,FlagNum,Len);
            return RET_ERROR;
    }
    return RET_SUCESS;
}

void Iec104_Tester_Timer(void){

    IEC104_STATE_FLAG_TESTER = IEC104_FLAG_IDLE;
    Iec104_TimeCycle_TesterIdle = 0;
}

uint8_t Iec104_Deal_I(PIEC104_DATA_T Iec104Data, uint16_t len){

    uint8_t Type;
    uint16_t RecvSn,SendSn;
    uint32_t FirmwareType = 0;

    PIEC10X_ASDU_T asdu = (PIEC10X_ASDU_T)(Iec104Data->Asdu);

    SendSn = Iec104Data->Ctrl.I.SendSn;
    RecvSn = Iec104Data->Ctrl.I.RecvSn;

    /* check asdu addrest */
    if(Iec10x_Sta_Addr != asdu->_addr){
        LOG("-%s-, error asdu addr(%x)(%x) \n" ,__FUNCTION__ ,Iec10x_Sta_Addr,asdu->_addr);
        return RET_ERROR;
    }
    
    /* deal the receive and send serial number */
    if(Iec104_Deal_SN(SendSn, RecvSn) == RET_ERROR){
        return RET_ERROR;
    }
    /* Start tester timer */
    Iec104_Tester_Timer();

    Type = asdu->_type;
    switch(Type){

        case IEC10X_C_IC_NA_1:
            LOG("++++Asdu Type Call cmd... \n");
            IEC104_ASDU_Call(asdu);
            IEC104_STATE_FLAG_S_ACK = IEC104_FLAG_IDLE;
            break;
        case IEC10X_C_CS_NA_1:
            LOG("++++Asdu Type Clock syc cmd... \n");
            Iec104_Aadu_Clock(asdu);
            break;
            
        case IEC10X_C_SE_NA_1:
        case IEC10X_C_SE_NC_1:
            LOG("++++Asdu Type Set type(%d)... \n",Type);
            IEC104_ASDU_SetAct(asdu, Type);
            IEC104_Build_SetAck(1, Type);
            break;
        case IEC10X_TI_FIRM_UPDATE:
            LOG("++++Asdu Type Firmware Update... \n");
            Iec104_Deal_FirmUpdate(asdu, len);
            break;
        case IEC10X_TI_AP_FIRM_BACKOFF:
            LOG("++++Asdu Type Firmware Backoff... \n");
            memcpy(&FirmwareType, asdu->_info,3);
            Iec104_BuildDataAck(IEC10X_TI_AP_FIRM_BACKOFF, IEC10X_COT_ACTCON, FirmwareType, 0,1);
            IEC10X->BackoffFirmware(FirmwareType);
            break;
        default:
            LOG("-%s-, error Type(%d) \n", __FUNCTION__,Type);
            return RET_ERROR;
    }
    return RET_SUCESS;
}


uint8_t Iec104_Deal_S(PIEC104_DATA_T Iec104Data, uint16_t len){


    return RET_SUCESS;
}
uint8_t Iec104_Deal_U(PIEC104_DATA_T Iec104Data, uint16_t len){

    switch(Iec104Data->Ctrl.Func.Func){

        case IEC104_U_FUNC_STARTDT:
            LOG(">%s<, function STARTDT \n",__FUNCTION__);
            IEC104_STATE_FLAG_INIT = IEC104_FLAG_LINK_INIT;
            Iec104_TimeCycle = 0;
            Iec104_TimeCount = 0;
            //IEC104_Build_U(IEC104_U_FUNC_STARTDT,1);
            //IEC104_Build_InitFin();
            break;
        case IEC104_U_FUNC_STOPDT:
            LOG(">%s<, function STOPDT \n",__FUNCTION__);
            IEC10X->CloseLink();
            IEC104_Build_U(IEC104_U_FUNC_STOPDT,1);
            IEC104_STATE_FLAG_INIT = IEC104_FLAG_RECV_CLOSED;
            break;
        case IEC104_U_FUNC_TESTER:
            LOG(">%s<, function TESTER \n",__FUNCTION__);
            IEC104_Build_U(IEC104_U_FUNC_TESTER,1);
            break;

        /* U ACK */
        case IEC104_U_FUNC_STARTDT_ACK:
            LOG(">%s<, function STARTDT ACK\n",__FUNCTION__);
            break;
        case IEC104_U_FUNC_STOPDT_ACK:
            LOG(">%s<, function STOPDT ACK\n",__FUNCTION__);
            IEC104_STATE_FLAG_INIT = IEC104_FLAG_RECV_CLOSED;
            break;
        case IEC104_U_FUNC_TESTER_ACK:
            LOG(">%s<, function TESTER ACK\n",__FUNCTION__);
            Iec104_TesterCount = 0;
            break;
        default:
            LOG(">%s<, function ERROR \n",__FUNCTION__);
            break;
    }
    return RET_SUCESS;
}

void Iex104_Receive(uint8_t *buf, uint16_t len){

    uint8_t *BufTemp = NULL;
    int16_t LenRemain,LenTmp;
    PIEC104_DATA_T Iec104Data = NULL;

    if(buf == NULL){
        LOG("-%s-,buffer (null)",__FUNCTION__);
        return;
    }
#if 0
    if(len <= 0 || len>IEC104_MAX_BUF_LEN || len<BufTemp[0]+2){
        LOG("-%s-,buffer len error(%d) \n",__FUNCTION__,len);
        return;
    }
#endif

    BufTemp = buf;
    LenRemain = len;

    while(BufTemp<buf+len){

        Iec104Data = (PIEC104_DATA_T)BufTemp;
        Iec10x_Lock();
        if(Iec104Data->Head == IEC104_HEAD){

            LenTmp = Iec104Data->Len + 2;
            if(LenRemain < IEC104_HEAD_LEN){
                LOG("_%s_,len error(%d) \n",__FUNCTION__,len);
                Iec10x_UnLock();
                return;
            }
            if(Iec104Data->Ctrl.Type.Type1 == 0){
                LOG("-%s-,Frame Type I \n",__FUNCTION__);
                Iec104_Deal_I(Iec104Data, LenTmp);

            }else if(Iec104Data->Ctrl.Type.Type1 == 1 && Iec104Data->Ctrl.Type.Type2 == 0){
                LOG("-%s-,Frame Type S \n",__FUNCTION__);
                Iec104_Deal_S(Iec104Data, LenTmp);

            }else if(Iec104Data->Ctrl.Type.Type1 == 1 && Iec104Data->Ctrl.Type.Type2 == 1){
                LOG("-%s-,Frame Type U \n",__FUNCTION__);
                Iec104_Deal_U(Iec104Data, LenTmp);
            }
        }else{
            LOG("-%s-,head type error(%d) \n",__FUNCTION__,BufTemp[0]);
            Iec10x_UnLock();
            return;
        }
        Iec10x_UnLock();
        BufTemp+=LenTmp;
        LenRemain-=LenTmp;
    }
    return;
}

void Iec104_ResetFlag(void){

    IEC104_STATE_FLAG_CALLALL = IEC104_FLAG_IDLE;
    IEC104_STATE_FLAG_GROUP = IEC104_FLAG_IDLE;
    IEC104_STATE_FLAG_CLOCK = IEC104_FLAG_IDLE;
    IEC104_STATE_FLAG_TESTER = IEC104_FLAG_IDLE;

    Iec104_BuildSendSn = 0;
    Iec104_BuildRecvSn = 0;
    Iec104_DealSendSn = -1;
    Iec104_DealRecvSn = 0;

    Iec104_TesterCount = 0;

}

uint32_t TestCount_Temp = 0;
uint8_t Iec104_StateMachine(void){

    /*Init link*/
    switch(IEC104_STATE_FLAG_INIT){

        case IEC104_FLAG_CLOSED:
#if defined(IEC104_STM32_FUJIAN_HX)
            IEC104_STATE_FLAG_INIT = IEC104_FLAG_UPLOAD_ADDR;
#else
            IEC104_STATE_FLAG_INIT = IEC104_FLAG_START_LINK;
            IEC10X_ClearQ();
            Iec104_ResetFlag();
#endif  /* IEC104_STM32_FUJIAN_HX */
            break;
        case IEC104_FLAG_SEND_CLOSED:
            Iec104_TimeCycle += IEC104_CYCLE_TIME_MS;
            if(Iec104_TimeCycle > IEC104_RESEND_TIME_MS){
                Iec104_TimeCycle = 0;
                Iec104_TimeCount++;
                IEC104_Build_U(IEC104_U_FUNC_STOPDT,0);
            }
            if(Iec104_TimeCount>=3){
                Iec104_TimeCount = 0;
                IEC104_STATE_FLAG_INIT = IEC104_FLAG_IDLE;
                IEC10X->CloseLink();
            }
            break;
        case IEC104_FLAG_RECV_CLOSED:
            Iec104_ResetFlag();
            IEC10X_ClearQ();
            IEC104_STATE_FLAG_INIT = IEC104_FLAG_CONNECT_SUCESS;
            break;
        case IEC104_FLAG_LINK_INIT:
            LOG("Iec104 machine state :IEC104_FLAG_LINK_INIT \n");
            Iec104_ResetFlag();
            Iec10x_Sta_Addr = IEC10X->GetLinkAddr();
            IEC104_Build_U(IEC104_U_FUNC_STARTDT,1);
            IEC104_Build_InitFin();
            IEC104_BuildUpload(IEC10X_TI_AP_BASE_INFO, IEC10X_PRIO_INITLINK, AP_TYPE_BASE_INFO);
            IEC104_STATE_FLAG_INIT = IEC104_FLAG_CONNECT_SUCESS;
            break;
        case IEC104_FLAG_UPLOAD_ADDR:
            Iec104_TimeCycle += IEC104_CYCLE_TIME_MS;
            if(Iec104_TimeCycle > IEC104_RESEND_TIME_MS){
                Iec10x_Sta_Addr = IEC10X->GetLinkAddr();
                Iec104_TimeCycle = 0;
                Iec104_TimeCount++;
                IEC104_UploadAddr();
            }
            if(Iec104_TimeCount>=3){
                Iec104_TimeCount = 0;
                IEC104_STATE_FLAG_INIT = IEC104_FLAG_IDLE;
                IEC10X->CloseLink();
            }
            break;

        case IEC104_FLAG_CONNECT_SUCESS:
        case IEC104_FLAG_IDLE:
        default:
            break;
    }
    if(IEC104_STATE_FLAG_INIT == IEC104_FLAG_CONNECT_SUCESS){

        /* Return s ack */
        switch(IEC104_STATE_FLAG_S_ACK){

            case IEC104_FLAG_S_ACK:
                Iec104_TimeCycle_S += IEC104_CYCLE_TIME_MS;
                if(Iec104_TimeCycle_S > IEC104_S_ACK_TIMEOUT){
                    Iec104_TimeCycle_S = 0;
                    IEC104_Build_S_Ack();
                    IEC104_STATE_FLAG_S_ACK = IEC104_FLAG_IDLE;
                }
                break;
            case IEC104_FLAG_IDLE:
                break;
            default:
                break;
        }
        /* test spon */
#if 0
        if(TestCount_Temp++>(10*60*20)){
            TestCount_Temp = 0;
            IEC104_BuildSignal_Spon(1, 1, IEC104_INFOADDR_STATE_HXGF+2);
            IEC104_BuildDetectF_Spont(1, 60.2, IEC104_INFOADDR_VALUE_HXTM+2);
            IEC104_BuildDetectF_Spont(1, 61.1, IEC104_INFOADDR_VALUE_HXGF+2);
        }
#endif
    }
    /* Tester */
    switch(IEC104_STATE_FLAG_TESTER){
        case IEC104_FLAG_TESTER:
            IEC104_Build_U(IEC104_U_FUNC_TESTER,0);
            IEC104_STATE_FLAG_TESTER = IEC104_FLAG_IDLE;
            Iec104_TesterCount++;
            LOG("Tester Count(%d)... \n",Iec104_TesterCount);
            if(Iec104_TesterCount>3){
                Iec104_TesterCount = 0;
                LOG("Tester error(%d)... \n",Iec104_TesterCount);
                IEC104_STATE_FLAG_INIT = IEC104_FLAG_CLOSED;
                IEC10X->CloseLink();
            }
            break;
        case IEC104_FLAG_TESTER_STOP:
            break;
        case IEC104_FLAG_IDLE:
            Iec104_TimeCycle_TesterIdle += IEC104_CYCLE_TIME_MS;
            if(Iec104_TimeCycle_TesterIdle > IEC104_TESTER_IDLE_TIMEOUT){
                Iec104_TimeCycle_TesterIdle = 0;
                IEC104_STATE_FLAG_TESTER = IEC104_FLAG_TESTER;
            }
            break;
        default:
            break;
    }
    return RET_SUCESS;
}


