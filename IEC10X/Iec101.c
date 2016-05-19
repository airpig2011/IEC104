

#include "Iec101.h"
#include "sys.h"
#ifdef  IEC101_STM32
 /*
 * GLOABLE VARIALBLE
 */
uint8_t                         Iec101_Respon_Confirm = 0;
uint8_t                         Iec101_Sendbuf[IEC101_MAX_BUF_LEN];


uint8_t             IEC10X_Call_AllQoi = 0;
uint8_t             IEC10X_Call_GroupQoi = 0;
uint16_t            IEC101_Pulse_Cnt = 0;

/*
 * STATE
 * */
uint8_t             IEC101_STATE_FLAG_INIT = IEC101_FLAG_LINK_CLOSED;
uint8_t             IEC101_STATE_FLAG_CALLALL = IEC101_FLAG_LINK_CLOSED;
uint8_t             IEC101_STATE_FLAG_GROUP = IEC101_FLAG_LINK_CLOSED;
uint8_t             IEC101_STATE_FLAG_CLOCK = IEC101_FLAG_LINK_CLOSED;
uint8_t             IEC101_STATE_FLAG_DELAY = IEC101_FLAG_LINK_CLOSED;
uint8_t             IEC101_STATE_FLAG_PULSE = IEC101_FLAG_LINK_CLOSED;

uint8_t IEC10X_RetStatusOk(uint16_t addr){

    uint16_t len;
    PIEC101_10_T Iec10x = (PIEC101_10_T)Iec101_Sendbuf;

    len = IEC101_STABLE_LEN;

    Iec10x->_begin = IEC101_STABLE_BEGING;

    Iec10x->_ctrl.up._dir = IEC101_CTRL_DIR_UP;
    Iec10x->_ctrl.up._prm = IEC101_CTRL_PRM_SLAVE;
    Iec10x->_ctrl.up._acd = IEC101_CTRL_ACD_NONE_DATA;
    Iec10x->_ctrl.up._dfc = IEC101_CTRL_DFC_CAN_REC;

    Iec10x->_ctrl.up._func = IEC101_CTRL_RES_LINK_STATUS;

    Iec10x->_addr = addr;
    Iec10x->_cs = Iec101_Sendbuf[1]+Iec101_Sendbuf[2]+Iec101_Sendbuf[3];
    Iec10x->_end = IEC101_STABLE_END;

    /* enqueue to the transmisson queue */
    IEC10X_Enqueue(Iec101_Sendbuf, len, IEC10X_PRIO_INITLINK, NULL,NULL);

    return RET_SUCESS;
}
static uint8_t IEC10X_ResConfirm(uint8_t Prio){

    uint16_t len;

    PIEC101_10_T Iec10x = (PIEC101_10_T)Iec101_Sendbuf;

    len = IEC101_STABLE_LEN;

    Iec10x->_begin = IEC101_STABLE_BEGING;

    Iec10x->_ctrl.up._dir = IEC101_CTRL_DIR_UP;
    Iec10x->_ctrl.up._prm = IEC101_CTRL_PRM_SLAVE;
    Iec10x->_ctrl.up._acd = IEC101_CTRL_ACD_NONE_DATA;
    Iec10x->_ctrl.up._dfc = IEC101_CTRL_DFC_CAN_REC;

    Iec10x->_ctrl.up._func = IEC101_CTRL_RES_CONFIRM;

    Iec10x->_addr = Iec10x_Sta_Addr;
    Iec10x->_cs = Iec101_Sendbuf[1]+Iec101_Sendbuf[2]+Iec101_Sendbuf[3];
    Iec10x->_end = IEC101_STABLE_END;

    /* enqueue to the transmisson queue */
    IEC10X_Enqueue(Iec101_Sendbuf, len, Prio, NULL,NULL);

    return RET_SUCESS;
}
uint8_t IEC10X_ReqLinkStatus(){

    uint16_t len;

    PIEC101_10_T Iec10x = (PIEC101_10_T)Iec101_Sendbuf;

    LOG("%s \n",__FUNCTION__);

    len = IEC101_STABLE_LEN;

    Iec10x->_begin = IEC101_STABLE_BEGING;

    /*Ctrol*/
    Iec10x->_ctrl.down._dir = IEC101_CTRL_DIR_UP;
    Iec10x->_ctrl.down._prm = IEC101_CTRL_PRM_MASTER;
    Iec10x->_ctrl.down._fcb = IEC101_CTRL_FCB_OPPO_NONE;
    Iec10x->_ctrl.down._fcv = IEC101_CTRL_FCV_DISABLE;

    Iec10x->_ctrl.up._func = IEC101_CTRL_REQ_LINK_STATUS;

    Iec10x->_addr = Iec10x_Sta_Addr;
    Iec10x->_cs = Iec101_Sendbuf[1]+Iec101_Sendbuf[2]+Iec101_Sendbuf[3];
    Iec10x->_end = IEC101_STABLE_END;

    /* enqueue to the transmisson queue */
    IEC10X_Enqueue(Iec101_Sendbuf, len, IEC10X_PRIO_INITLINK, NULL,NULL);

    return RET_SUCESS;
}


uint8_t IEC10X_ResetLink(void){

    uint16_t len;

    PIEC101_10_T Iec10x = (PIEC101_10_T)Iec101_Sendbuf;

    len = IEC101_STABLE_LEN;

    Iec10x->_begin = IEC101_STABLE_BEGING;

    /*Ctrol*/
    Iec10x->_ctrl.down._dir = IEC101_CTRL_DIR_UP;
    Iec10x->_ctrl.down._prm = IEC101_CTRL_PRM_MASTER;
    Iec10x->_ctrl.down._fcb = IEC101_CTRL_FCB_OPPO_NONE;
    Iec10x->_ctrl.down._fcv = IEC101_CTRL_FCV_DISABLE;

    Iec10x->_ctrl.up._func = IEC101_CTRL_RESET_LINK;

    Iec10x->_addr = Iec10x_Sta_Addr;
    Iec10x->_cs = Iec101_Sendbuf[1]+Iec101_Sendbuf[2]+Iec101_Sendbuf[3];
    Iec10x->_end = IEC101_STABLE_END;

    /* enqueue to the transmisson queue */
    IEC10X_Enqueue(Iec101_Sendbuf, len, IEC10X_PRIO_INITLINK, NULL,NULL);

    return RET_SUCESS;
}


uint8_t IEC101_BuildFinInit(void){

    uint16_t len = 0;
    uint8_t cs_temp = 0,i;

    PIEC101_68_T Iec10x = (PIEC101_68_T)Iec101_Sendbuf;
    PIEC10X_ASDU_T asdu = (PIEC10X_ASDU_T)(Iec10x->_asdu);
    PASDU_INFO_T info = (PASDU_INFO_T)(asdu->_info);

    len = IEC101_STABLE_LEN;

    /*head*/
    Iec10x->_begin = Iec10x->_begin_cfm = IEC101_VARIABLE_BEGING;

    /*Ctrol*/
    Iec10x->_ctrl.up._dir = IEC101_CTRL_DIR_UP;
    Iec10x->_ctrl.up._prm = IEC101_CTRL_PRM_MASTER;
    Iec10x->_ctrl.up._acd = IEC101_CTRL_ACD_WITH_DATA;
    Iec10x->_ctrl.up._dfc = IEC101_CTRL_DFC_CANNOT_REC;

    Iec10x->_ctrl.up._func = IEC101_CTRL_SEND_USR_DATA_ACK;

    Iec10x->_addr = Iec10x_Sta_Addr;
    /*asdu*/
    asdu->_type = Iec10x_M_EI_NA_1;
    asdu->_num._num = 1;
    asdu->_reason._reason = IEC10X_ASDU_REASON_INIT;
    asdu->_addr = Iec10x_Sta_Addr;
    /*info*/
    info->_addr = IEC10X_INFO_ADDR_NONE;
    info->_element[0] = 0;

    /*len*/
    len = IEC101_VARIABLE_LEN + asdu->_num._num;
    Iec10x->_len = Iec10x->_len_cfm = len-4-2;          /*-start-len-len-start   -cs-end*/

    /*end*/
    for(i=4; i<len-2; i++){
        cs_temp += Iec101_Sendbuf[i];
    }
    Iec101_Sendbuf[len-2] = cs_temp;
    Iec101_Sendbuf[len-1] = IEC101_VARIABLE_END;

    //DumpHEX(Iec101_Sendbuf,len);
    /* enqueue to the transmisson queue */
    IEC10X_Enqueue(Iec101_Sendbuf, len ,IEC10X_PRIO_INITLINK, NULL,NULL);

    return RET_SUCESS;

}
uint8_t IEC101_BuildActConfirm(uint8_t qoi,uint8_t Prio){

    uint16_t len = 0;
    uint8_t cs_temp = 0,i;

    PIEC101_68_T Iec10x = (PIEC101_68_T)Iec101_Sendbuf;
    PIEC10X_ASDU_T asdu = (PIEC10X_ASDU_T)(Iec10x->_asdu);
    PASDU_INFO_T info = (PASDU_INFO_T)(asdu->_info);

    len = IEC101_STABLE_LEN;

    /*head*/
    Iec10x->_begin = Iec10x->_begin_cfm = IEC101_VARIABLE_BEGING;

    /*Ctrol*/
    Iec10x->_ctrl.up._dir = IEC101_CTRL_DIR_UP;
    Iec10x->_ctrl.up._prm = IEC101_CTRL_PRM_MASTER;
    Iec10x->_ctrl.up._acd = IEC101_CTRL_ACD_NONE_DATA;
    Iec10x->_ctrl.up._dfc = IEC101_CTRL_DFC_CANNOT_REC;

    Iec10x->_ctrl.up._func = IEC101_CTRL_SEND_USR_DATA_ACK;

    Iec10x->_addr = Iec10x_Sta_Addr;
    /*asdu*/
    asdu->_type = IEC10X_C_IC_NA_1;
    asdu->_num._sq = 0;
    asdu->_num._num = 1;
    asdu->_reason._pn = 0;
    asdu->_reason._test = 0;
    asdu->_reason._reason = IEC10X_ASDU_REASON_ACTCON;
    asdu->_addr = Iec10x_Sta_Addr;
    /*info*/
    info->_addr = IEC10X_INFO_ADDR_NONE;
    info->_element[0] = qoi;
    /*len*/
    len = IEC101_VARIABLE_LEN + asdu->_num._num;
    Iec10x->_len = Iec10x->_len_cfm = len-4-2;          /*-start-len-len-start   -cs-end*/

    /*end*/
    for(i=4; i<len-2; i++){
        cs_temp += Iec101_Sendbuf[i];
    }
    Iec101_Sendbuf[len-2] = cs_temp;
    Iec101_Sendbuf[len-1] = IEC101_VARIABLE_END;

    //DumpHEX(Iec101_Sendbuf,len);
    /* enqueue to the transmisson queue */
    IEC10X_Enqueue(Iec101_Sendbuf, len, Prio, NULL,NULL);

    return RET_SUCESS;
}

uint8_t IEC101_BuildActFinish(uint8_t qoi, uint8_t Prio){

    uint16_t len = 0;
    uint8_t cs_temp = 0,i;

    PIEC101_68_T Iec10x = (PIEC101_68_T)Iec101_Sendbuf;
    PIEC10X_ASDU_T asdu = (PIEC10X_ASDU_T)(Iec10x->_asdu);
    PASDU_INFO_T info = (PASDU_INFO_T)(asdu->_info);

    len = IEC101_STABLE_LEN;

    /*head*/
    Iec10x->_begin = Iec10x->_begin_cfm = IEC101_VARIABLE_BEGING;

    /*Ctrol*/
    Iec10x->_ctrl.up._dir = IEC101_CTRL_DIR_UP;
    Iec10x->_ctrl.up._prm = IEC101_CTRL_PRM_MASTER;
    Iec10x->_ctrl.up._acd = IEC101_CTRL_ACD_WITH_DATA;
    Iec10x->_ctrl.up._dfc = IEC101_CTRL_DFC_CANNOT_REC;

    Iec10x->_ctrl.up._func = IEC101_CTRL_SEND_USR_DATA_ACK;

    Iec10x->_addr = Iec10x_Sta_Addr;
    /*asdu*/
    asdu->_type = IEC10X_C_IC_NA_1;
    asdu->_num._sq = 0;
    asdu->_num._num = 1;
    asdu->_reason._pn = 0;
    asdu->_reason._test = 0;
    asdu->_reason._reason = IEC10X_ASDU_REASON_ACTTERM;
    asdu->_addr = Iec10x_Sta_Addr;
    /*info*/
    info->_addr = IEC10X_INFO_ADDR_NONE;
    info->_element[0] = qoi;

    /*len*/
    len = IEC101_VARIABLE_LEN + asdu->_num._num;
    Iec10x->_len = Iec10x->_len_cfm = len-4-2;          /*-start-len-len-start   -cs-end*/

    /*end*/
    for(i=4; i<len-2; i++){
        cs_temp += Iec101_Sendbuf[i];
    }
    Iec101_Sendbuf[len-2] = cs_temp;
    Iec101_Sendbuf[len-1] = IEC101_VARIABLE_END;

    //DumpHEX(Iec101_Sendbuf,len);
    /* enqueue to the transmisson queue */
    IEC10X_Enqueue(Iec101_Sendbuf, len, Prio, NULL,NULL);

    return RET_SUCESS;
}

uint8_t IEC101_BuildSignal(uint8_t reason,uint8_t Prio){

    uint16_t len = 0,addr;
    uint8_t cs_temp = 0, i, asdu_num = 0, *ptr = NULL, signal,sum;

    /*init struct*/
    PIEC101_68_T Iec10x = (PIEC101_68_T)Iec101_Sendbuf;
    PIEC10X_ASDU_T asdu = (PIEC10X_ASDU_T)(Iec10x->_asdu);
    PASDU_INFO_T info = (PASDU_INFO_T)(asdu->_info);

    /*get value*/
    IEC10X->GetStationInfo(&addr,0,&asdu_num);
    len = IEC101_STABLE_LEN;
    LOG("-%s- total info (%d) \n\n",__FUNCTION__, asdu_num);

    /*head*/
    Iec10x->_begin = Iec10x->_begin_cfm = IEC101_VARIABLE_BEGING;

    /*Ctrol*/
    Iec10x->_ctrl.up._dir = IEC101_CTRL_DIR_UP;
    Iec10x->_ctrl.up._prm = IEC101_CTRL_PRM_MASTER;
    Iec10x->_ctrl.up._acd = IEC101_CTRL_ACD_WITH_DATA;
    Iec10x->_ctrl.up._dfc = IEC101_CTRL_DFC_CANNOT_REC;

    Iec10x->_ctrl.up._func = IEC101_CTRL_SEND_USR_DATA_ACK;

    Iec10x->_addr = Iec10x_Sta_Addr;
    /*asdu*/
    asdu->_type = IEC10X_M_SP_NA_1;
    asdu->_num._sq = 1;
    asdu->_num._num = asdu_num;
    asdu->_reason._pn = 0;
    asdu->_reason._test = 0;
    asdu->_reason._reason = reason;
    asdu->_addr = Iec10x_Sta_Addr;
    /*info*/
    info->_addr = IEC10X_INFO_ADDR_SIG_BASE+IEC10X_INFO_ADDR_SIG_TEMP_HX_OFF;

    /*signal value*/
    ptr = info->_element;
    for(i=0; i<asdu_num; i++){
        if(IEC10X->GetStationInfo(&addr,i,&asdu_num) == RET_SUCESS)
            *ptr = 1;
        else
            *ptr = 0;
        ptr++;
    }
    /*len*/
    len = ptr + 2 - Iec101_Sendbuf;
    Iec10x->_len = Iec10x->_len_cfm = len-4-2;          /*-start-len-len-start   -cs-end*/

    /*end*/
    for(i=4; i<len-2; i++){
        cs_temp += Iec101_Sendbuf[i];
    }
    Iec101_Sendbuf[len-2] = cs_temp;
    Iec101_Sendbuf[len-1] = IEC101_VARIABLE_END;

    //DumpHEX(Iec101_Sendbuf,len);
    /* enqueue to the transmisson queue */
    IEC10X_Enqueue(Iec101_Sendbuf, len, Prio, NULL,NULL);

    return RET_SUCESS;
}

uint8_t IEC101_BuildDetect(uint8_t reason,uint8_t type, uint8_t Prio){

    uint16_t len = 0,addr;
    uint8_t cs_temp = 0, i, asdu_num = 0, *ptr = NULL;
    PIEC10X_DETECT_T detect = NULL;
    PIEC10X_DETECT_F_T detect_f = NULL;

    /*init struct*/
    PIEC101_68_T Iec10x = (PIEC101_68_T)Iec101_Sendbuf;
    PIEC10X_ASDU_T asdu = (PIEC10X_ASDU_T)(Iec10x->_asdu);
    PASDU_INFO_T info = (PASDU_INFO_T)(asdu->_info);

    /*get value*/
    IEC10X->GetStationInfo(&addr,0,&asdu_num);
    LOG("-%s- total info (%d) \n\n",__FUNCTION__, asdu_num);

    len = IEC101_STABLE_LEN;

    /*head*/
    Iec10x->_begin = Iec10x->_begin_cfm = IEC101_VARIABLE_BEGING;

    /*Ctrol*/
    Iec10x->_ctrl.up._dir = IEC101_CTRL_DIR_UP;
    Iec10x->_ctrl.up._prm = IEC101_CTRL_PRM_MASTER;
    Iec10x->_ctrl.up._acd = IEC101_CTRL_ACD_NONE_DATA;
    Iec10x->_ctrl.up._dfc = IEC101_CTRL_DFC_CANNOT_REC;

    Iec10x->_ctrl.up._func = IEC101_CTRL_SEND_USR_DATA_ACK;

    Iec10x->_addr = Iec10x_Sta_Addr;
    /*asdu*/
    asdu->_type = type;
    asdu->_num._sq = 1;
    asdu->_num._num = asdu_num;
    asdu->_reason._pn = 0;
    asdu->_reason._test = 0;
    asdu->_reason._reason = reason;
    asdu->_addr = Iec10x_Sta_Addr;

    /*info*/
    info->_addr = IEC10X_INFO_ADDR_DET+IEC10X_INFO_ADDR_DET_TEMP_HX_OFF;

    /*Detect value*/
    ptr = info->_element;
    for(i=0; i<asdu_num; i++){
        /*short int*/
        if(type == IEC10X_M_ME_NA_1){
            detect = (PIEC10X_DETECT_T)ptr;
            detect->_detect = IEC10X->GetStaTemp(i);
            detect->_qds = 0;
            ptr += sizeof(IEC10X_DETECT_T);
        }
        /*float*/
        else if(type == IEC10X_M_ME_NC_1){
            detect_f = (PIEC10X_DETECT_F_T)ptr;
            detect_f->_detect = IEC10X->GetStaTemp(i);
            detect_f->_qds = 0;
            ptr += sizeof(IEC10X_DETECT_F_T);
        }
    }

    /*len*/
    len = ptr + 2 - Iec101_Sendbuf;                     /* add cs+end*/
    Iec10x->_len = Iec10x->_len_cfm = len-4-2;          /*-start-len-len-start   -cs-end*/

    /*end*/
    for(i=4; i<len-2; i++){
        cs_temp += Iec101_Sendbuf[i];
    }
    Iec101_Sendbuf[len-2] = cs_temp;
    Iec101_Sendbuf[len-1] = IEC101_VARIABLE_END;

    //DumpHEX(Iec101_Sendbuf,len);
    /* enqueue to the transmisson queue */
    IEC10X_Enqueue(Iec101_Sendbuf, len, Prio, NULL,NULL);

    return RET_SUCESS;
}

uint8_t IEC101_BuildSignal_Spont(uint8_t TimeFlag, uint8_t signalV, uint16_t addrV){

    uint16_t len = 0;
    uint8_t cs_temp = 0, i, asdu_num = 0, *ptr = NULL;
    PCP56Time2a_T time = NULL;

    /*init struct*/
    PIEC101_68_T Iec10x = (PIEC101_68_T)Iec101_Sendbuf;
    PIEC10X_ASDU_T asdu = (PIEC10X_ASDU_T)(Iec10x->_asdu);
    PASDU_INFO_T info = (PASDU_INFO_T)(asdu->_info);

    if(TimeFlag != 1 && TimeFlag != 0){
        LOG("-%s-, error time flag(%d) \n",__FUNCTION__,TimeFlag);
        return RET_ERROR;
    }
    LOG("-%s-, time flag(%d) signalV(%d) \n",__FUNCTION__,TimeFlag,signalV);
    /*get value*/
    asdu_num = 1;

    len = IEC101_STABLE_LEN;

    /*head*/
    Iec10x->_begin = Iec10x->_begin_cfm = IEC101_VARIABLE_BEGING;

    /*Ctrol*/
    Iec10x->_ctrl.up._dir = IEC101_CTRL_DIR_UP;
    Iec10x->_ctrl.up._prm = IEC101_CTRL_PRM_MASTER;
    Iec10x->_ctrl.up._acd = IEC101_CTRL_ACD_WITH_DATA;
    Iec10x->_ctrl.up._dfc = IEC101_CTRL_DFC_CANNOT_REC;

    Iec10x->_ctrl.up._func = IEC101_CTRL_SEND_USR_DATA_ACK;

    Iec10x->_addr = Iec10x_Sta_Addr;
    /*asdu*/
    if(TimeFlag == 0)
        asdu->_type = IEC10X_M_SP_NA_1;
    else
        asdu->_type = IEC10X_M_SP_TB_1;
    asdu->_num._sq = 0;
    asdu->_num._num = asdu_num;
    asdu->_reason._pn = 0;
    asdu->_reason._test = 0;
    asdu->_reason._reason = IEC10X_COT_SPONT;
    asdu->_addr = Iec10x_Sta_Addr;
    /*info*/
    info->_addr = addrV;

    /*signal value*/
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
    len = ptr + 2 - Iec101_Sendbuf;
    Iec10x->_len = Iec10x->_len_cfm = len-4-2;          /*-start-len-len-start   -cs-end*/

    /*end*/
    for(i=4; i<len-2; i++){
        cs_temp += Iec101_Sendbuf[i];
    }
    Iec101_Sendbuf[len-2] = cs_temp;
    Iec101_Sendbuf[len-1] = IEC101_VARIABLE_END;

    //DumpHEX(Iec101_Sendbuf,len);
    /* enqueue to the transmisson queue */
    IEC10X_Enqueue(Iec101_Sendbuf, len, IEC10X_PRIO_SPON, NULL,NULL);

    return RET_SUCESS;
}
uint8_t IEC101_BuildDetect_Spont(uint8_t TimeFlag, PIEC10X_DETECT_T detectV, uint16_t addrV){

    uint16_t len = 0;
    uint8_t cs_temp = 0, i, asdu_num = 0, *ptr = NULL;
    PIEC10X_DETECT_T detect = NULL;
    PCP56Time2a_T time = NULL;

    /*init struct*/
    PIEC101_68_T Iec10x = (PIEC101_68_T)Iec101_Sendbuf;
    PIEC10X_ASDU_T asdu = (PIEC10X_ASDU_T)(Iec10x->_asdu);
    PASDU_INFO_T info = (PASDU_INFO_T)(asdu->_info);

    if(TimeFlag != 1 && TimeFlag != 0){
        LOG("-%s-, error time flag(%d) \n",__FUNCTION__,TimeFlag);
        return RET_ERROR;
    }
	
    /*get value*/
    asdu_num = 1;

    len = IEC101_STABLE_LEN;

    /*head*/
    Iec10x->_begin = Iec10x->_begin_cfm = IEC101_VARIABLE_BEGING;

    /*Ctrol*/
    Iec10x->_ctrl.up._dir = IEC101_CTRL_DIR_UP;
    Iec10x->_ctrl.up._prm = IEC101_CTRL_PRM_MASTER;
    Iec10x->_ctrl.up._acd = IEC101_CTRL_ACD_NONE_DATA;
    Iec10x->_ctrl.up._dfc = IEC101_CTRL_DFC_CANNOT_REC;

    Iec10x->_ctrl.up._func = IEC101_CTRL_SEND_USR_DATA_ACK;

    Iec10x->_addr = Iec10x_Sta_Addr;
    /*asdu*/
    if(TimeFlag == 0)
        asdu->_type = IEC10X_M_ME_NA_1;
    else
        asdu->_type = IEC10X_M_ME_TD_1;
    asdu->_num._sq = 0;
    asdu->_num._num = asdu_num;
    asdu->_reason._pn = 0;
    asdu->_reason._test = 0;
    asdu->_reason._reason = IEC10X_COT_SPONT;
    asdu->_addr = Iec10x_Sta_Addr;

    /*info*/
    info->_addr = addrV;

    /*Detect value*/
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
    len = ptr + 2 - Iec101_Sendbuf;                     /* add cs+end*/
    Iec10x->_len = Iec10x->_len_cfm = len-4-2;          /*-start-len-len-start   -cs-end*/

    /*end*/
    for(i=4; i<len-2; i++){
        cs_temp += Iec101_Sendbuf[i];
    }
    Iec101_Sendbuf[len-2] = cs_temp;
    Iec101_Sendbuf[len-1] = IEC101_VARIABLE_END;

    //DumpHEX(Iec101_Sendbuf,len);
    /* enqueue to the transmisson queue */
    IEC10X_Enqueue(Iec101_Sendbuf, len, IEC10X_PRIO_SPON, NULL,NULL);

    return RET_SUCESS;
}
uint8_t IEC101_BuildDetectF_Spont(uint8_t TimeFlag, float detectV, uint16_t addrV){

    uint16_t len = 0;
    uint8_t cs_temp = 0, i, asdu_num = 0, *ptr = NULL;
    PIEC10X_DETECT_F_T detect = NULL;
    PCP56Time2a_T time = NULL;

    /*init struct*/
    PIEC101_68_T Iec10x = (PIEC101_68_T)Iec101_Sendbuf;
    PIEC10X_ASDU_T asdu = (PIEC10X_ASDU_T)(Iec10x->_asdu);
    PASDU_INFO_T info = (PASDU_INFO_T)(asdu->_info);
	
    if(TimeFlag != 1 && TimeFlag != 0){
        LOG("-%s-, error time flag(%d) \n",__FUNCTION__,TimeFlag);
        return RET_ERROR;
    }

    /*get value*/
    asdu_num = 1;

    len = IEC101_STABLE_LEN;

    /*head*/
    Iec10x->_begin = Iec10x->_begin_cfm = IEC101_VARIABLE_BEGING;

    /*Ctrol*/
    Iec10x->_ctrl.up._dir = IEC101_CTRL_DIR_UP;
    Iec10x->_ctrl.up._prm = IEC101_CTRL_PRM_MASTER;
    Iec10x->_ctrl.up._acd = IEC101_CTRL_ACD_NONE_DATA;
    Iec10x->_ctrl.up._dfc = IEC101_CTRL_DFC_CANNOT_REC;

    Iec10x->_ctrl.up._func = IEC101_CTRL_SEND_USR_DATA_ACK;

    Iec10x->_addr = Iec10x_Sta_Addr;
    /*asdu*/
    if(TimeFlag == 0)
        asdu->_type = IEC10X_M_ME_NC_1;
    else
        asdu->_type = IEC10X_M_ME_TF_1;
    asdu->_num._sq = 0;
    asdu->_num._num = asdu_num;
    asdu->_reason._pn = 0;
    asdu->_reason._test = 0;
    asdu->_reason._reason = IEC10X_COT_SPONT;
    asdu->_addr = Iec10x_Sta_Addr;

    /*info*/
    info->_addr = addrV;

    /*Detect value*/
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
    len = ptr + 2 - Iec101_Sendbuf;                     /* add cs+end*/
    Iec10x->_len = Iec10x->_len_cfm = len-4-2;          /*-start-len-len-start   -cs-end*/

    /*end*/
    for(i=4; i<len-2; i++){
        cs_temp += Iec101_Sendbuf[i];
    }
    Iec101_Sendbuf[len-2] = cs_temp;
    Iec101_Sendbuf[len-1] = IEC101_VARIABLE_END;

    //DumpHEX(Iec101_Sendbuf,len);
    /* enqueue to the transmisson queue */
    IEC10X_Enqueue(Iec101_Sendbuf, len, IEC10X_PRIO_SPON, NULL,NULL);

    return RET_SUCESS;
}
uint8_t IEC101_BuildDelayAct(uint16_t delay_time){

    uint16_t len = 0;
    uint8_t cs_temp = 0, i, asdu_num = 0, *ptr = NULL;
    uint16_t cp16time2a = delay_time;

    /*init struct*/
    PIEC101_68_T Iec10x = (PIEC101_68_T)Iec101_Sendbuf;
    PIEC10X_ASDU_T asdu = (PIEC10X_ASDU_T)(Iec10x->_asdu);
    PASDU_INFO_T info = (PASDU_INFO_T)(asdu->_info);

    /*get value*/
    asdu_num = 1;

    len = IEC101_STABLE_LEN;

    /*head*/
    Iec10x->_begin = Iec10x->_begin_cfm = IEC101_VARIABLE_BEGING;

    /*Ctrol*/
    Iec10x->_ctrl.up._dir = IEC101_CTRL_DIR_UP;
    Iec10x->_ctrl.up._prm = IEC101_CTRL_PRM_MASTER;
    Iec10x->_ctrl.up._acd = IEC101_CTRL_ACD_NONE_DATA;
    Iec10x->_ctrl.up._dfc = IEC101_CTRL_DFC_CANNOT_REC;

    Iec10x->_ctrl.up._func = IEC101_CTRL_SEND_USR_DATA_ACK;

    Iec10x->_addr = Iec10x_Sta_Addr;
    /*asdu*/
    asdu->_type = IEC10X_C_CD_NA_1;
    asdu->_num._sq = 0;
    asdu->_num._num = asdu_num;
    asdu->_reason._pn = 0;
    asdu->_reason._test = 0;
    asdu->_reason._reason = IEC10X_COT_ACTCON;
    asdu->_addr = Iec10x_Sta_Addr;
    /*info*/
    info->_addr = IEC10X_INFO_ADDR_NONE;

    /*delay value*/
    ptr = info->_element;
    *(uint16_t *)ptr = IEC10X_Cp16time2a;
    ptr+=2;

    /*len*/
    len = ptr + 2 - Iec101_Sendbuf;
    Iec10x->_len = Iec10x->_len_cfm = len-4-2;          /*-start-len-len-start   -cs-end*/

    /*end*/
    for(i=4; i<len-2; i++){
        cs_temp += Iec101_Sendbuf[i];
    }
    Iec101_Sendbuf[len-2] = cs_temp;
    Iec101_Sendbuf[len-1] = IEC101_VARIABLE_END;

    //DumpHEX(Iec101_Sendbuf,len);
    /* enqueue to the transmisson queue */
    IEC10X_Enqueue(Iec101_Sendbuf, len, IEC10X_PRIO_DELAY, NULL,NULL);

    return RET_SUCESS;
}
uint8_t IEC101_BuildClockAct(){

    uint16_t len = 0;
    uint8_t cs_temp = 0, i, asdu_num = 0, *ptr = NULL;

    /*init struct*/
    PIEC101_68_T Iec10x = (PIEC101_68_T)Iec101_Sendbuf;
    PIEC10X_ASDU_T asdu = (PIEC10X_ASDU_T)(Iec10x->_asdu);
    PASDU_INFO_T info = (PASDU_INFO_T)(asdu->_info);
    PCP56Time2a_T time = (PCP56Time2a_T)(info->_element);

    /*get value*/
    asdu_num = 1;

    len = IEC101_STABLE_LEN;

    /*head*/
    Iec10x->_begin = Iec10x->_begin_cfm = IEC101_VARIABLE_BEGING;

    /*Ctrol*/
    Iec10x->_ctrl.up._dir = IEC101_CTRL_DIR_UP;
    Iec10x->_ctrl.up._prm = IEC101_CTRL_PRM_MASTER;
    Iec10x->_ctrl.up._acd = IEC101_CTRL_ACD_NONE_DATA;
    Iec10x->_ctrl.up._dfc = IEC101_CTRL_DFC_CANNOT_REC;

    Iec10x->_ctrl.up._func = IEC101_CTRL_SEND_USR_DATA_ACK;

    Iec10x->_addr = Iec10x_Sta_Addr;
    /*asdu*/
    asdu->_type = IEC10X_C_CS_NA_1;
    asdu->_num._sq = 0;
    asdu->_num._num = asdu_num;
    asdu->_reason._pn = 0;
    asdu->_reason._test = 0;
    asdu->_reason._reason = IEC10X_COT_ACTCON;
    asdu->_addr = Iec10x_Sta_Addr;
    /*info*/
    info->_addr = IEC10X_INFO_ADDR_NONE;

    /*clock value*/
    ptr = info->_element;
    //time;
    IEC10X->GetTime(time);
    ptr+=sizeof(CP56Time2a_T);


    /*len*/
    len = ptr + 2 - Iec101_Sendbuf;
    Iec10x->_len = Iec10x->_len_cfm = len-4-2;          /*-start-len-len-start   -cs-end*/

    /*end*/
    for(i=4; i<len-2; i++){
        cs_temp += Iec101_Sendbuf[i];
    }
    Iec101_Sendbuf[len-2] = cs_temp;
    Iec101_Sendbuf[len-1] = IEC101_VARIABLE_END;

    //DumpHEX(Iec101_Sendbuf,len);
    /* enqueue to the transmisson queue */
    IEC10X_Enqueue(Iec101_Sendbuf, len, IEC10X_PRIO_CLOCK, NULL,NULL);

    return RET_SUCESS;
}
IEC10X_ASDU_CALL_Qoi(uint8_t qoi){

    switch(qoi){
        case IEC10X_CALL_QOI_TOTAL:
            LOG("-%s- call cmd active \n", __FUNCTION__);
            IEC101_BuildSignal(IEC10X_COT_INTROGEN,IEC10X_PRIO_CALLALL);
            IEC101_STATE_FLAG_CALLALL = IEC101_FLAG_CALL_SIG_TOTAL;
            break;
        case IEC10X_CALL_QOI_GROUP1:
            LOG("-%s- call cmd call group1 \n" ,__FUNCTION__ );
            IEC101_STATE_FLAG_GROUP = IEC101_FLAG_CALL_GROUP;
            IEC101_BuildSignal(IEC10X_COT_INTRO1, IEC10X_PRIO_CALLGROUP);
            break;
        case IEC10X_CALL_QOI_GROUP2:
            LOG("-%s- call cmd call group2 \n" ,__FUNCTION__ );
            IEC101_STATE_FLAG_GROUP = IEC101_FLAG_CALL_GROUP;
            IEC101_BuildSignal(IEC10X_COT_INTRO2, IEC10X_PRIO_CALLGROUP);
            break;
        case IEC10X_CALL_QOI_GROUP9:
            LOG("-%s- call cmd call group9 \n" ,__FUNCTION__ );
            IEC101_STATE_FLAG_GROUP = IEC101_FLAG_CALL_GROUP;
            IEC101_BuildDetect(IEC10X_COT_INTRO9,IEC10X_M_ME_NA_1, IEC10X_PRIO_CALLGROUP);
            break;
        case IEC10X_CALL_QOI_GROUP10:
            LOG("-%s- call cmd call group10 \n" ,__FUNCTION__ );
            IEC101_STATE_FLAG_GROUP = IEC101_FLAG_CALL_GROUP;
            IEC101_BuildDetect(IEC10X_COT_INTRO10,IEC10X_M_ME_NA_1, IEC10X_PRIO_CALLGROUP);
            break;
        default:
            LOG("-%s- call cmd active error(%d) \n" ,__FUNCTION__ ,qoi);
            break;
    }
}

uint8_t IEC10X_ASDU_Call(PIEC10X_ASDU_T Iec10x_Asdu){

    PASDU_INFO_T asdu_info = (PASDU_INFO_T)(Iec10x_Asdu->_info);
    uint8_t qoi = asdu_info->_element[0];
    uint8_t Prio = 0;

    if(asdu_info->_addr != 0){
        LOG("-%s- call cmd active error addr(%x) \n" ,__FUNCTION__ ,asdu_info->_addr);
        return RET_ERROR;
    }

    switch(Iec10x_Asdu->_reason._reason){

        case IEC10X_ASDU_REASON_ACT:
            switch(qoi){
                case IEC10X_CALL_QOI_TOTAL:
                    Prio = IEC10X_PRIO_CALLALL;
                    IEC101_STATE_FLAG_CALLALL = IEC101_FLAG_CALL_ACT_RET;
                    IEC10X_Call_AllQoi = qoi;
                    break;
                case IEC10X_CALL_QOI_GROUP1:
                case IEC10X_CALL_QOI_GROUP2:
                case IEC10X_CALL_QOI_GROUP9:
                case IEC10X_CALL_QOI_GROUP10:
                    Prio = IEC10X_PRIO_CALLGROUP;
                    IEC101_STATE_FLAG_GROUP = IEC101_FLAG_CALL_ACT_RET;
                    IEC10X_Call_GroupQoi = qoi;
                    break;
                default:
                    LOG("-%s- call cmd error qoi(%d) \n", __FUNCTION__,qoi);
                    return RET_ERROR;
            }
            IEC10X_ResConfirm(Prio);
            IEC101_BuildActConfirm(qoi,Prio);
            break;

        default:
            LOG("-%s- call cmd error reason(%d) \n", __FUNCTION__,Iec10x_Asdu->_reason._reason);
            break;
    }
    return RET_SUCESS;
}

uint8_t IEC10X_ASDU_Delay(PIEC10X_ASDU_T Iec10x_Asdu){

    PASDU_INFO_T asdu_info = (PASDU_INFO_T)(Iec10x_Asdu->_info);

    if(asdu_info->_addr != 0){
        LOG("-%s- delay cmd error addr(%d) \n", __FUNCTION__,asdu_info->_addr);
        return RET_ERROR;
    }

    switch(Iec10x_Asdu->_reason._reason){

        case IEC10X_COT_ACT:
            IEC10X_Cp16time2a = *(uint16_t *)(asdu_info->_element);
            LOG("-%s- delay cmd (0x%x%x)(%d)ms \n", __FUNCTION__,asdu_info->_element[0],asdu_info->_element[1],IEC10X_Cp16time2a);
            IEC101_STATE_FLAG_DELAY = IEC101_FLAG_DELAY_ACT;
            IEC10X_ResConfirm(IEC10X_PRIO_DELAY);
            IEC101_BuildDelayAct(IEC10X_Cp16time2a);
            break;
        case IEC10X_COT_SPONT:
            IEC10X_Cp16time2a_V = *(uint16_t *)(asdu_info->_element);
            LOG("-%s- delay cmd delay value(%d)ms \n", __FUNCTION__,IEC10X_Cp16time2a_V);
            IEC10X_ResConfirm(IEC10X_PRIO_DELAY);
            break;
        default:
            LOG("-%s- delay cmd error reason(%d) \n", __FUNCTION__,Iec10x_Asdu->_reason._reason);
            break;
    }
    return RET_SUCESS;
}

uint8_t IEC10X_ASDU_CLOCK(PIEC10X_ASDU_T Iec10x_Asdu){

    PASDU_INFO_T asdu_info = (PASDU_INFO_T)(Iec10x_Asdu->_info);
    PCP56Time2a_T time = (PCP56Time2a_T)(asdu_info->_element);

    memcpy(&IEC10X_Cp56time2a,asdu_info->_element, sizeof(CP56Time2a_T));


    if(asdu_info->_addr != 0){
        LOG("-%s- Clock cmd error addr(%d) \n", __FUNCTION__,asdu_info->_addr);
        return RET_ERROR;
    }

    switch(Iec10x_Asdu->_reason._reason){

        case IEC10X_COT_ACT:
            LOG("-%s- Clock cmd (20%d-%d-%d %d %d:%d:%d) delay(%d) \n", __FUNCTION__,IEC10X_Cp56time2a._year._year,IEC10X_Cp56time2a._month._month,IEC10X_Cp56time2a._day._dayofmonth,
                    IEC10X_Cp56time2a._day._dayofweek,IEC10X_Cp56time2a._hour._hours,IEC10X_Cp56time2a._min._minutes,IEC10X_Cp56time2a._milliseconds,IEC10X_Cp16time2a_V);
            IEC10X_Cp56time2a._milliseconds += IEC10X_Cp16time2a_V;
            /*get time*/
            /*...*/
            IEC10X->SetTime(&IEC10X_Cp56time2a);
            IEC101_STATE_FLAG_DELAY = IEC101_FLAG_CLOCK_SYS;
            IEC10X_ResConfirm(IEC10X_PRIO_CLOCK);
            IEC101_BuildClockAct();
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


uint8_t Iec10x_Deal_10(uint8_t *buf, uint16_t len){

    uint8_t cfun, sta,i;
    uint8_t cs_temp = 0;
    PIEC101_10_T Iec10x_10 = NULL;

    Iec10x_10 = (PIEC101_10_T)buf;
    /* check check_sum*/
    for(i=1; i<len-2; i++){
        cs_temp += buf[i];
    }
    if(Iec10x_10->_cs != cs_temp){
        LOG("-%s-,check sum error(%x) \n",__FUNCTION__,cs_temp);
        return RET_ERROR;
    }
    /* check end of the frame*/
    if(Iec10x_10->_end != 0x16){
        LOG("-%s-,iec10x_10 end error(%d) \n",__FUNCTION__,Iec10x_10->_end);
        return RET_ERROR;
    }
    /*deal the function code*/
    cfun = Iec10x_10->_ctrl.down._func;
    sta = Iec10x_10->_ctrl.down._prm;
    //IEC10X->GetLinkAddr();
#ifdef IEC10XLOCK
    if(IEC10X->LOCK != NULL)
        IEC10X->LOCK();
#endif
    if(sta == IEC101_CTRL_PRM_MASTER){
        switch(cfun){

            case IEC101_CTRL_RESET_LINK:
                LOG("++++Reset link... \n");
                IEC101_STATE_FLAG_INIT = IEC101_FLAG_RESET_LINK;
                break;
            case IEC101_CTRL_PULSE:
                LOG("++++PULSE... \n");
                IEC101_STATE_FLAG_PULSE = IEC101_FLAG_PULSE;
                IEC10X_ResConfirm(IEC10X_PRIO_PULSE);
                break;
            case IEC101_CTRL_SEND_DATA:
                LOG("++++Send data... \n");
                break;
            case IEC101_CTRL_REQ_LINK_STATUS:
                LOG("++++Request link status... \n");
                IEC101_STATE_FLAG_INIT = IEC101_FLAG_INIT_LINK;
                //Iec10x_Sta_Addr = IEC10X->GetLinkAddr();

                //Iec10x_Sta_Addr = 0x138a;
                //IEC10X_RetStatusOk(Iec10x_Sta_Addr);
                break;
            default:
                LOG("++++error DOWN function code (%d)... \n",cfun);
                break;
        }
    }else if(sta == IEC101_CTRL_PRM_SLAVE){

        switch(cfun){
            case IEC101_CTRL_RES_LINK_STATUS:
                LOG("++++Respon link status... \n");
                LOG("IEC101_FLAG_RESET_REMOTE_LINK \n");
                IEC10X_ResetLink();
                IEC101_STATE_FLAG_INIT = IEC101_FLAG_RESET_REMOTE_LINK;
                break;
            case IEC101_CTRL_RES_CONFIRM:
                LOG("++++Respon confirm... \n");
                Iec101_Respon_Confirm = 1;
                break;
            default:
                LOG("++++error UP function code (%d)... \n",cfun);
                break;
        }
    }
#ifdef IEC10XLOCK
    if(IEC10X->UNLOCK != NULL)
        IEC10X->UNLOCK();
#endif
    return RET_SUCESS;
}

uint8_t Iec10x_Deal_68(uint8_t *buf, uint16_t len){

    uint8_t cfun, sta, i;
    uint8_t cs_temp = 0;
    PIEC101_68_T Iec10x_68 = NULL;
    PIEC10X_ASDU_T Iec10x_Asdu = NULL;

    Iec10x_68 = (PIEC101_68_T)buf;
    Iec10x_Asdu = (PIEC10X_ASDU_T)Iec10x_68->_asdu;

    /* check check_sum*/
    for(i=4; i<len-2; i++){
        cs_temp += buf[i];
    }
    if(buf[len-2] != cs_temp){
        LOG("-%s-,iec10x_68 check sum error (%d)(%d) \n",__FUNCTION__,buf[len-2],cs_temp);
        return RET_ERROR;
    }
    /* check end of the frame*/
    if(buf[len-1] != IEC101_VARIABLE_END){
        LOG("-%s-,iec10x_68 end error(%d) \n",__FUNCTION__,buf[len-1]);
        return RET_ERROR;
    }
    /* check len of the receive frame */
    if(Iec10x_68->_len+6 != len){
        LOG("-%s-,iec10x_68 rec len error(%d)(%d) \n",__FUNCTION__, Iec10x_68->_len, len);
        return RET_ERROR;
    }
    /* check len of the frame */
    if(Iec10x_68->_len != Iec10x_68->_len_cfm){
        LOG("-%s-,iec10x_68 len error(%d)(%d) \n",__FUNCTION__, Iec10x_68->_len, Iec10x_68->_len_cfm);
        return RET_ERROR;
    }
    /*deal the function code*/
    cfun = Iec10x_68->_ctrl.down._func;
    sta = Iec10x_68->_ctrl.down._prm;
#ifdef IEC10XLOCK
    if(IEC10X->LOCK != NULL)
        IEC10X->LOCK();
#endif
    switch(cfun){
        case IEC101_CTRL_SEND_USR_DATA_ACK:
            Iec101_Respon_Confirm = 0;
            LOG("++++Send user data need ack... \n");
            switch(Iec10x_Asdu->_type){
                case IEC10X_C_IC_NA_1:
                    LOG("++++asdu type call cmd... \n");
                    IEC10X_ASDU_Call(Iec10x_Asdu);
                    break;
                case IEC10X_C_CD_NA_1:
                    LOG("++++asdu type delay active... \n");
                    IEC10X_ASDU_Delay(Iec10x_Asdu);
                    break;
                case IEC10X_C_CS_NA_1:
                    LOG("++++asdu type clock active... \n");
                    IEC10X_ASDU_CLOCK(Iec10x_Asdu);
                    break;
                default:
                    break;
            }
            break;
        default:
            LOG("++++error function code (%d)... \n", cfun);
            break;
    }
#ifdef IEC10XLOCK
    if(IEC10X->UNLOCK != NULL)
        IEC10X->UNLOCK();
#endif
    return RET_SUCESS;
}

void Iex101_main(uint8_t *buf, uint16_t len){

    uint8_t *BufTemp = NULL;
    int16_t LenRemain,LenTmp;

    if(buf == NULL){
        LOG("-%s-,buffer (null)",__FUNCTION__);
        return;
    }
    if(len <= 0 || len>IEC101_MAX_BUF_LEN){
        LOG("-%s-,buffer len error(%d) \n",__FUNCTION__,len);
        return;
    }

    BufTemp = buf;
    LenRemain = len;
    while(BufTemp<buf+len){

        if(BufTemp[0] == IEC101_STABLE_BEGING){
            LenTmp = 6;
            IEC101_Pulse_Cnt = 0;
            if(LenRemain<6){
                LOG("_%s_,len error(%d) \n",__FUNCTION__,len);
                return;
            }
            Iec10x_Deal_10(BufTemp, LenTmp);
        }else if(BufTemp[0] == IEC101_VARIABLE_BEGING){
            LenTmp = BufTemp[1]+6;
            IEC101_Pulse_Cnt = 0;
            if(BufTemp[1]!=BufTemp[2]){
                LOG("_%s_,len error(%d)(%d) \n",__FUNCTION__,BufTemp[1],BufTemp[2]);
                return;
            }
            Iec10x_Deal_68(BufTemp, LenTmp);
        }else{
            LOG("-%s-,head type error(%d) \n",__FUNCTION__,BufTemp[0]);
            return;
        }
        BufTemp+=LenTmp;
        LenRemain-=LenTmp;
    }
    return;
}

uint8_t Iec101_StateMachine(void){

    /*Init link*/

    switch(IEC101_STATE_FLAG_INIT){

        case IEC101_FLAG_LINK_CLOSED:
            break;
        case IEC101_FLAG_INIT_LINK:
            LOG("IEC101_FLAG_INIT_LINK \n");
            Iec101_Respon_Confirm = 0;
            Iec10x_Sta_Addr = IEC10X->GetLinkAddr();
            IEC10X_RetStatusOk(Iec10x_Sta_Addr);
            IEC101_STATE_FLAG_INIT = IEC101_FLAG_LINK_IDLE;
            break;
        case IEC101_FLAG_RESET_LINK:
            LOG("IEC101_FLAG_RESET_LINK \n");
            IEC10X_ClearQ();
            IEC10X_ResConfirm(IEC10X_PRIO_INITLINK);

            IEC101_STATE_FLAG_CALLALL = IEC101_FLAG_LINK_CLOSED;
            IEC101_STATE_FLAG_GROUP   = IEC101_FLAG_LINK_CLOSED;
            IEC101_STATE_FLAG_DELAY   = IEC101_FLAG_LINK_CLOSED;
            IEC101_STATE_FLAG_PULSE   = IEC101_FLAG_LINK_CLOSED;

            IEC101_STATE_FLAG_INIT = IEC101_FLAG_REQ_LINK;
            break;
        case IEC101_FLAG_REQ_LINK:
            LOG("IEC101_FLAG_REQ_LINK \n");
            IEC10X_ReqLinkStatus();
            IEC101_STATE_FLAG_INIT = IEC101_FLAG_LINK_IDLE;
            break;
        case IEC101_FLAG_RESET_REMOTE_LINK:
            if(Iec101_Respon_Confirm){
                Iec101_Respon_Confirm = 0;
                IEC101_STATE_FLAG_INIT = IEC101_FLAG_INIT_FIN;
                LOG("IEC101_FLAG_INIT_FIN! \n");
                IEC101_BuildFinInit();
            }
            break;
        case IEC101_FLAG_INIT_FIN:
            if(Iec101_Respon_Confirm){
                Iec101_Respon_Confirm = 0;
                IEC101_STATE_FLAG_INIT = IEC101_FLAG_CONNECT_OK;
            }
            break;
        case IEC101_FLAG_CONNECT_OK:
        case IEC101_FLAG_LINK_IDLE:
        default:
            break;
    }

    /*total call*/
    switch(IEC101_STATE_FLAG_CALLALL){
        case IEC101_FLAG_CALL_ACT:
            break;
        case IEC101_FLAG_CALL_ACT_RET:
            if(Iec101_Respon_Confirm){
                Iec101_Respon_Confirm = 0;
                IEC10X_ASDU_CALL_Qoi(IEC10X_Call_AllQoi);
            }
            break;
        case IEC101_FLAG_CALL_SIG_TOTAL:
            if(Iec101_Respon_Confirm){
                Iec101_Respon_Confirm = 0;
                IEC101_STATE_FLAG_CALLALL = IEC101_FLAG_CALL_DET_TOTAL;
                IEC101_BuildDetect(IEC10X_COT_INTROGEN,IEC10X_M_ME_NA_1,IEC10X_PRIO_CALLALL);
            }
            break;
        case IEC101_FLAG_CALL_DET_TOTAL:
            if(Iec101_Respon_Confirm){
                Iec101_Respon_Confirm = 0;
                IEC101_BuildActFinish(IEC10X_Call_AllQoi,IEC10X_PRIO_CALLALL);
                IEC101_STATE_FLAG_CALLALL = IEC101_FLAG_CALL_ACT_FIN;
            }
            break;
        case IEC101_FLAG_CALL_ACT_FIN:
            if(Iec101_Respon_Confirm){
                Iec101_Respon_Confirm = 0;
                IEC101_STATE_FLAG_CALLALL = IEC101_FLAG_LINK_IDLE;
            }
            break;
        case IEC101_FLAG_LINK_IDLE:
        default:
            break;
    }

    /*group call*/
    switch(IEC101_STATE_FLAG_GROUP){
        case IEC101_FLAG_CALL_ACT:
            break;
        case IEC101_FLAG_CALL_ACT_RET:
            if(Iec101_Respon_Confirm){
                Iec101_Respon_Confirm = 0;
                IEC10X_ASDU_CALL_Qoi(IEC10X_Call_GroupQoi);
            }
            break;
        case IEC101_FLAG_CALL_GROUP:
            if(Iec101_Respon_Confirm){
                Iec101_Respon_Confirm = 0;
                IEC101_STATE_FLAG_GROUP = IEC101_FLAG_CALL_ACT_FIN;
                IEC101_BuildActFinish(IEC10X_Call_GroupQoi,IEC10X_PRIO_CALLGROUP);
            }
            break;
        case IEC101_FLAG_CALL_ACT_FIN:
            if(Iec101_Respon_Confirm){
                Iec101_Respon_Confirm = 0;
                IEC101_STATE_FLAG_GROUP = IEC101_FLAG_LINK_IDLE;
            }
            break;
        case IEC101_FLAG_LINK_IDLE:
        default:
            break;
    }

    /*delay and clock*/
    switch(IEC101_STATE_FLAG_DELAY){

        case IEC101_FLAG_DELAY_ACT:
            break;
        case IEC101_FLAG_CLOCK_SYS:
            IEC101_STATE_FLAG_DELAY = IEC101_FLAG_LINK_IDLE;
            break;
        case IEC101_FLAG_LINK_IDLE:
        default:
            break;
    }

    /*pulse*/
    switch(IEC101_STATE_FLAG_PULSE){
        case IEC101_FLAG_PULSE:
            break;
        case IEC101_FLAG_LINK_IDLE:
            break;
        default:
            break;
    }
    return RET_SUCESS;
}

#endif
