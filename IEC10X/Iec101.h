/*******************************************************************
Copyright (C):    
File name    :    Iec101.h
DESCRIPTION  :
AUTHOR       :
Version      :    1.0
Date         :    2014/07/23
Others       :
History      :
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
1) Date: 2014/07/23         Author: ChenDajie
   content:

*******************************************************************/

#ifndef _IEC101_H
#define _IEC101_H


#include <stdio.h>
#include <string.h>
#include "sys.h"
#include "common_datatype.h"
#include "PRIO_QUEUE_Iec10x.h"
#include "Iec10x.h"
#ifdef IEC101_STM32
#define ARM_PACK __packed
#ifdef __cplusplus
 extern "C" {
#endif

 /*
 *********************************************************************************************************
 *                           FUNCTION CODE
 *********************************************************************************************************
 */
 /*
  * down
  * */
#define     IEC101_CTRL_RESET_LINK          0
#define     IEC101_CTRL_PULSE               2
#define     IEC101_CTRL_SEND_USR_DATA_ACK   3
#define     IEC101_CTRL_SEND_DATA           4
#define     IEC101_CTRL_REQ_LINK_STATUS     9


/*
 * up
 * */
#define     IEC101_CTRL_RES_CONFIRM         0x00
#define     IEC101_CTRL_RES_LINK_STATUS     0x0B
/*
 * DIR, PRM, FCB, FCV, ACD, DFC
 * */
#define     IEC101_CTRL_DIR_DOWN            0x01
#define     IEC101_CTRL_DIR_UP              0x00
#define     IEC101_CTRL_FCB_OPPO_BIT        0x01
#define     IEC101_CTRL_FCB_OPPO_NONE       0x00
#define     IEC101_CTRL_PRM_MASTER          0x01
#define     IEC101_CTRL_PRM_SLAVE           0x00
#define     IEC101_CTRL_FCV_ENABLE          0x01
#define     IEC101_CTRL_FCV_DISABLE         0x00
#define     IEC101_CTRL_ACD_WITH_DATA       0x01
#define     IEC101_CTRL_ACD_NONE_DATA       0x00
#define     IEC101_CTRL_DFC_CAN_REC         0x00
#define     IEC101_CTRL_DFC_CANNOT_REC      0x01

 /*
 *********************************************************************************************************
 *                           FRAME  VALUE
 *********************************************************************************************************
 */
#define     IEC101_STABLE_BEGING             0X10
#define     IEC101_STABLE_END                0X16
#define     IEC101_STABLE_LEN                0X06
/*
 * variable
 * */
#define     IEC101_VARIABLE_BEGING           0X68
#define     IEC101_VARIABLE_END              0X16
 /*
  * length of variable
  * */
#define     IEC101_VARIABLE_HEAD_LEN          (sizeof(IEC101_68_T)-1)
#define     IEC101_VARIABLE_ASDU_LEN          (sizeof(IEC10X_ASDU_T)-1)
#define     IEC101_VARIABLE_INFO_LEN          (sizeof(ASDU_INFO_T)-1)
#define     IEC101_VARIABLE_LEN               (IEC101_VARIABLE_HEAD_LEN+IEC101_VARIABLE_ASDU_LEN+IEC101_VARIABLE_INFO_LEN+2)        /*add cs+end*/

#pragma pack(1)


#define IEC101_MAX_BUF_LEN  256

/*
 * Control Code
 * */
typedef __packed struct{

    uint8_t _func:4;               /* function */
    uint8_t _fcv:1;
    uint8_t _fcb:1;                /* Frame calculate bit */
    uint8_t _prm:1;                /* 1:from start station, 0:from end station */
    uint8_t _dir:1;

}CTRL_DOWN_T;

typedef __packed struct {

    uint8_t _func:4;               /*function*/
    uint8_t _dfc:1;
    uint8_t _acd:1;
    uint8_t _prm:1;
    uint8_t _dir:1;

}CTRL_UP_T;

typedef __packed union
{
    CTRL_UP_T up;
    CTRL_DOWN_T down;
    uint8_t val;
} CTRL_T,*PCTRL_T;

/*
 *  regular frame   head:10
 * */
typedef __packed struct
{
    uint8_t _begin;
    CTRL_T _ctrl;
    uint16_t _addr;
    uint8_t _cs;
    uint8_t _end;
}IEC101_10_T,*PIEC101_10_T;

/*
 *  variable fram   head:68
 * */
typedef __packed struct
{
    uint8_t _begin;
    uint8_t _len;
    uint8_t _len_cfm;
    uint8_t _begin_cfm;
    CTRL_T _ctrl;
    uint16_t _addr;
    uint8_t _asdu[1];
}IEC101_68_T,*PIEC101_68_T;

#pragma pack()


/* IEC101 STATE-MACHINE */
enum {
    IEC101_FLAG_LINK_CLOSED,
    IEC101_FLAG_LINK_IDLE,
    IEC101_FLAG_INIT_LINK,
    IEC101_FLAG_RESET_LINK,
    IEC101_FLAG_REQ_LINK,
    IEC101_FLAG_RESET_REMOTE_LINK,
    IEC101_FLAG_INIT_FIN,
    IEC101_FLAG_CALL_ACT,
    IEC101_FLAG_CALL_ACT_FIN,
    IEC101_FLAG_CALL_ACT_RET,
    IEC101_FLAG_CALL_SIG_TOTAL,
    IEC101_FLAG_CALL_DET_TOTAL,
    IEC101_FLAG_DELAY_ACT,
    IEC101_FLAG_CLOCK_SYS,
    IEC101_FLAG_PULSE,
    IEC101_FLAG_CALL_GROUP,
    IEC101_FLAG_CONNECT_OK,
};

/*
*********************************************************************************************************
*                                           GLOABLE VARIABLE
*********************************************************************************************************
*/
extern        uint16_t            IEC101_Pulse_Cnt;
extern        uint8_t             IEC101_STATE_FLAG_INIT;
/*
*********************************************************************************************************
*                                          FUNCTION PROTOTYPES
*********************************************************************************************************
*/

void                Iex101_main(uint8_t *buf, uint16_t len);
uint8_t             Iec101_StateMachine(void);

uint8_t             IEC101_BuildSignal_Spont(uint8_t TimeFlag, uint8_t signalV, uint16_t addrV);
uint8_t             IEC101_BuildDetect_Spont(uint8_t TimeFlag, PIEC10X_DETECT_T detectV, uint16_t addrV);
uint8_t             IEC101_BuildDetectF_Spont(uint8_t TimeFlag, float detectV, uint16_t addrV);

#ifdef __cplusplus
}
#endif  /*IEC101_STM32*/
#endif
#endif /*_IEC101_H*/
