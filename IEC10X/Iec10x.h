/*******************************************************************
Copyright (C):    
File name    :    Iec10x.h
DESCRIPTION  :
AUTHOR       :
Version      :    1.0
Date         :    2014/07/23
Others       :
History      :
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
1) Date: 2014/07/23         Author: ChenDajie
   content:
           add iec104

*******************************************************************/

#ifndef _IEC10X_H
#define _IEC10X_H


#include <stdio.h>
#include <string.h>
#include "sys.h"
#include "common_datatype.h"
#include "PRIO_QUEUE_Iec10x.h"

/*
 * CONFIGURE
 * */
#define             IEC10XLOCK
#define             PRIO_QUEUE
#ifdef  IEC101_STM32
#define             IEC10X_PRIO_MAX                 7
#elif defined(IEC104_STM32)
#define             IEC10X_PRIO_MAX                 7
#endif
#define             IEC10X_HEADER_LENGTH            1

/*
 * PRIO
 * */
#define             IEC10X_PRIO_INITLINK            0
#define             IEC10X_PRIO_CALLALL             1
#define             IEC10X_PRIO_CALLGROUP           2
#define             IEC10X_PRIO_CLOCK               3
#define             IEC10X_PRIO_DELAY               3
#define             IEC10X_PRIO_PULSE               0
#define             IEC10X_PRIO_SPON                0


/*
 *  Transmission  Priority Queue.
 */
typedef struct IEC10X_CallbackArg{
    uint8_t *value;
    uint32_t PicSn;
    uint32_t FramSerialNum;
}Iec10x_CallbackArg_T;

typedef struct IEC10X_NODE{
    uint16_t Length;
    struct IEC10X_NODE *Next;
    void(* CallBack)(Iec10x_CallbackArg_T *Arg);
    Iec10x_CallbackArg_T CallBackArg;
    uint8_t value[1];
}Iec10x_PrioNode_T;

typedef struct IEC10X_QUEUE{
    struct IEC10X_NODE *Header;
    struct IEC10X_NODE *Tail;
    unsigned char ElementNum;
}Iec10x_PrioQueue_T;


/*
*********************************************************************************************************
*                                           BLOCK SIZE AND NUMBER
*
* if add part, please change OS_MAX_MEM_PART in ucosii.h.
*********************************************************************************************************
*/

/* IEC10X_PARTITION 1   size==256*8 */
#define IEC10X_PARTITION_NUM_1       5
#define IEC10X_PARTITION_SIZE_1      512
#define IEC10X_PARTITION_1           (IEC10X_PARTITION_SIZE_1-16)

/* IEC10X_PARTITION 2   size==(1300+16)*1 */
#define IEC10X_PARTITION_NUM_2       20
#define IEC10X_PARTITION_2           128
#define IEC10X_PARTITION_SIZE_2      (IEC10X_PARTITION_2+sizeof(Iec10x_PrioNode_T))

/* IEC10X_PARTITION 1   size==64*20 */
#define IEC10X_PARTITION_NUM_3       10
#define IEC10X_PARTITION_3           32
#define IEC10X_PARTITION_SIZE_3      (IEC10X_PARTITION_3+sizeof(Iec10x_PrioNode_T))

/*
*********************************************************************************************************
*                                           ASDU
*********************************************************************************************************
*/

/*
 * ASDU TYPE
 * */
#define     Iec10x_M_EI_NA_1                0X46
#define     IEC10X_C_IC_NA_1                0X64        /*CALL*/
#define     IEC10X_C_CS_NA_1                103
#define     IEC10X_C_CD_NA_1                106         /*CALL*/

/*Set data*/
#define     IEC10X_C_SE_NA_1                48
#define     IEC10X_C_SE_NC_1                50

/*
 * ASDU REASON
 * */
#define     IEC10X_ASDU_REASON_INIT         0X04
#define     IEC10X_ASDU_REASON_ACT          0X06
#define     IEC10X_ASDU_REASON_ACTCON       0X07        /*CALL*/
#define     IEC10X_ASDU_REASON_ACTFIN       0X08        /*CALL FINISH*/
#define     IEC10X_ASDU_REASON_ACTTERM      0X0a        /*CALL Terminal*/
#define     IEC10X_ASDU_COT_UNKNOW          45

/*
* INFO ADDR
* */
#define     IEC10X_INFO_ADDR_NONE           0X00

/*
*********************************************************************************************************
*                           REMOTE SIGNAL
*********************************************************************************************************
*/
/*
 * Asdu type (TI)
 * */
#define     IEC10X_M_SP_NA_1                0X01
#define     IEC10X_M_DP_NA_1                0X03
#define     IEC10X_M_SP_TB_1                30
#define     IEC10X_M_DP_TB_1                31
/*
 * Asdu reason (COT)
 * */
#define     IEC10X_COT_BACK                 2
#define     IEC10X_COT_SPONT                3
#define     IEC10X_COT_ACT                  6
#define     IEC10X_COT_ACTCON               7
#define     IEC10X_COT_REQ                  5
#define     IEC10X_COT_RETREM               11
#define     IEC10X_COT_RETLOC               12
#define     IEC10X_COT_INTROGEN             20
#define     IEC10X_COT_INTRO1               21
#define     IEC10X_COT_INTRO2               22

/*
 * Asdu addr
 * */
#define     IEC10X_INFO_ADDR_SIG_BASE           0X0001
#define     IEC10X_INFO_ADDR_SIG_TEMP_HX_OFF    0x1000

/*
*********************************************************************************************************
*                           REMOTE DECTET
*********************************************************************************************************
*/
/*
 * Asdu type (TI)
 * */
#define     IEC10X_M_ME_NA_1                9
#define     IEC10X_M_ME_NC_1                13
#define     IEC10X_M_ME_TD_1                34
#define     IEC10X_M_ME_TF_1                36

/*
 * Asdu reason (COT)
 * */
#define     IEC10X_COT_PER_CYC               1
#define     IEC10X_COT_BACK                  2
#define     IEC10X_COT_SPONT                 3
#define     IEC10X_COT_REQ                   5
#define     IEC10X_COT_INTROGEN              20
#define     IEC10X_COT_INTRO9                29
#define     IEC10X_COT_INTRO10               30

/*
 * Asdu addr
 * */
#define     IEC10X_INFO_ADDR_DET                 0X4001
#define     IEC10X_INFO_ADDR_DET_TEMP_HX_OFF     0x1000

/*
*********************************************************************************************************
*                           CALL QOI
*********************************************************************************************************
*/

#define IEC10X_CALL_QOI_TOTAL               20
#define IEC10X_CALL_QOI_GROUP1              21
#define IEC10X_CALL_QOI_GROUP2              22
#define IEC10X_CALL_QOI_GROUP9              29
#define IEC10X_CALL_QOI_GROUP10             30

/*
*********************************************************************************************************
*                           INFO ADDREST        
*               TD -- temprature device
*               PD -- pullotion device
*********************************************************************************************************
*/
#define IEC10X_SET_TD_UPCYCLE               0X100001
#define IEC10X_SET_TD_MAXIMUM               0X100002
#define IEC10X_SET_TD_MINIMUM               0X100003
#define IEC10X_SET_TD_DETCYCLE              0X100004
#define IEC10X_SET_TD_TEMPRISE              0X100005
#define IEC10X_SET_PD_DETCYCLE              0X100006
#define IEC10X_SET_PD_PULPSE                0X100007
#define IEC10X_SET_PD_SELFDETCYCLE          0X100008
/*
*********************************************************************************************************
*                           Fujian Huixing Add
*********************************************************************************************************
*/
/* TI. */
#define     IEC10X_TI_FIRM_UPDATE              128
#define     IEC10X_TI_AP_FIRM_BACKOFF          129
#define     IEC10X_TI_AP_BASE_INFO             130
/* COT. */
#define     IEC10X_COT_ACT_TERMINAL            8
#define     IEC10X_COT_ACT_TERMINAL_ACK        9

#define     IEC10X_COT_DATA                    14
#define     IEC10X_COT_DATA_ACK                15
#define     IEC10X_COT_DATA_FIN                16
#define     IEC10X_COT_DATA_FIN_ACK            17
#define     IEC10X_COT_DATA_NEEDACK            0X100E


#pragma pack(1)

/*
 *  asdu number
 * */
typedef struct {

    uint8_t _num:7;
    uint8_t _sq:1;

}ASDU_NUM_T;
/*
 *  asdu reason
 * */
typedef   struct {
#ifdef  IEC101_STM32
    uint8_t _reason:6;
    uint8_t _pn:1;
    uint8_t _test:1;
#elif defined(IEC104_STM32)
    uint16_t _reason:14;
    uint16_t _pn:1;
    uint16_t _test:1;
#endif

}ASDU_REASON_T;
/*
 *  asdu
 * */
typedef   struct{

    uint8_t         _type;
    ASDU_NUM_T      _num;
    ASDU_REASON_T   _reason;
    uint16_t        _addr;
    uint8_t         _info[1];
}IEC10X_ASDU_T, *PIEC10X_ASDU_T;
/*
 *  asdu info
 * */
typedef   struct{

#ifdef  IEC101_STM32
    uint16_t _addr;
#elif defined(IEC104_STM32)
    uint8_t  _addr[3];
#endif
    uint8_t _element[1];
}ASDU_INFO_T, *PASDU_INFO_T;

/*
 *  asdu info remote signal with time flag
 * */
typedef   struct{

    uint8_t _signal;
    uint8_t _time;
}SIGNAL_TIME_T, *PSIGNAL_TIME_T;


/*
 *  asdu info remote detect with time flag
 * */
/*int */
typedef   struct{

    int16_t _detect;
    uint8_t _qds;

}IEC10X_DETECT_T, *PIEC10X_DETECT_T;
/*float*/
typedef   struct{

    float _detect;
    uint8_t _qds;

}IEC10X_DETECT_F_T, *PIEC10X_DETECT_F_T;


/*int  sq=0 */
typedef   struct{

#ifdef  IEC101_STM32
    uint16_t _addr;
#elif defined(IEC104_STM32)
    uint8_t  _addr[3];
#endif
    int16_t _detect;
    uint8_t _qds;

}IEC10X_DETECT_SQ0_T, *PIEC10X_DETECT_SQ0_T;
/*float sq=0*/
typedef   struct{

#ifdef  IEC101_STM32
    uint16_t _addr;
#elif defined(IEC104_STM32)
    uint8_t  _addr[3];
#endif
    float _detect;
    uint8_t _qds;

}IEC10X_DETECT_SQ0_F_T, *PIEC10X_DETECT_SQ0_F_T;

/*
 *  asdu info CP56Time2a
 * */
typedef struct{

    uint8_t _minutes:6;
    uint8_t _res:1;
    uint8_t _iv:1;
}IEC10X_Time_Min_T, *PIEC10X_Time_Min_T;

typedef struct{

    uint8_t _hours:5;
    uint8_t _res:2;
    uint8_t _su:1;
}IEC10X_Time_Hour_T, *PIEC10X_Time_Hour_T;
typedef struct{

    uint8_t _dayofmonth:5;
    uint8_t _dayofweek:3;
}IEC10X_Time_Day_T, *PIEC10X_Time_Day_T;
typedef struct{

    uint8_t _month:4;
    uint8_t _res:3;
}IEC10X_Time_Month_T, *PIEC10X_Time_Month_T;
typedef struct{

    uint8_t _year:7;
    uint8_t _res:1;
}IEC10X_Time_Year_T, *PIEC10X_Time_Year_T;
typedef struct{

    uint16_t _milliseconds;
    IEC10X_Time_Min_T _min;
    IEC10X_Time_Hour_T _hour;
    IEC10X_Time_Day_T _day;
    IEC10X_Time_Month_T _month;
    IEC10X_Time_Year_T _year;

}CP56Time2a_T, *PCP56Time2a_T;

/*
 *  asdu info remote detect with time flag
 * */
typedef   struct{

    uint16_t _detect;
    uint8_t _qds;
    CP56Time2a_T _time;
}DETECT_TIME_T, *PDETECT_TIME_T;

#pragma pack()

/*
 * Iec10x init
 */
typedef struct _iec10x {

    char * name;
    int (* Init)(void);
    void (* Delay_ms)(uint16_t);
    void (* CloseLink)(void);
    void *(* Malloc)(uint8_t NumByte);
    void (* Free)(void *buffer);
    uint8_t (* enqueue)(Iec10x_PrioQueue_T *QueueHdr, Iec10x_PrioNode_T *NewNode);
    Iec10x_PrioNode_T *(* dequeue)(Iec10x_PrioQueue_T * QueueHdr);
    Iec10x_PrioNode_T *(* FindQHead)(Iec10x_PrioQueue_T * QueueHdr);
    char (* GetPrio)(void);
    void (* InitQueue)(Iec10x_PrioQueue_T *PrioQueue);
    void (* ClearQueue)(Iec10x_PrioQueue_T * QueueHdr);
    uint8_t (* Send)(int socketfd,char *data,int len);
    uint32_t (* SetTime)(PCP56Time2a_T time);
    uint32_t (* GetTime)(PCP56Time2a_T time);
    int8_t (* GetStationState)(uint16_t Addr, uint8_t DevType);
    float (* GetStaValue)(uint16_t Addr, uint8_t DevType);
    uint16_t (* GetLinkAddr)(void);
    int8_t (* GetInfoNum)(uint8_t *InfoNum, uint8_t DevType);
    int8_t (* SetConfig)(long Value, uint32_t addr);
    int8_t (* SaveFirmware)(uint8_t FirmLen, uint8_t *buf,uint32_t FirmwareType, uint32_t Iec10x_Update_SeekAddr);
    int8_t (* CheckFirmware)(uint32_t FirmwareType, uint32_t TotalLen);
    int8_t (* UpdateFirmware)(uint32_t FirmwareType);
    int8_t (* BackoffFirmware)(uint32_t FirmwareType);

#ifdef IEC10XLOCK
    void (* LOCK)(void);
    void (* UNLOCK)(void);
#endif

} *PIEC10X_T, IEC10X_T;


/*
 * BIG ENDIAN
 * */
#ifdef BIG_ENDIAN
#define   n2hs(x)    (x)
#define   h2ns(x)    (x)

#define   n2hl(x)    (x)
#define   h2nl(x)    (x)
/* LITTLE ENDIAN */
#else
#define   n2hs(x)    ((((x)>>8) & 0xFF) | (((x) & 0xFF)<<8))
#define   h2ns(x)    ((((x)>>8) & 0xFF) | (((x) & 0xFF)<<8))

#define   n2hl(x)    ( (((x)>>24) & 0xFF) | (((x)>>8) & 0xFF00) | \
                        (((x) & 0xFF00)<<8) | (((x) & 0xFF)<<24)   \
                      )
#define   h2nl(x)    ( (((x)>>24) & 0xFF) | (((x)>>8) & 0xFF00) | \
                        (((x) & 0xFF00)<<8) | (((x) & 0xFF)<<24)   \
                      )
#endif  /* BIG_ENDIAN */

/*
*********************************************************************************************************
*                                           EXTERN VARIABLE
*********************************************************************************************************
*/
extern      PIEC10X_T           IEC10X;
extern      uint16_t            Iec10x_Sta_Addr;
extern      int32_t             Iec104_BuildSendSn;
extern      int32_t             Iec104_BuildRecvSn;

extern      uint16_t            IEC10X_Cp16time2a;
extern      uint16_t            IEC10X_Cp16time2a_V;
extern      CP56Time2a_T        IEC10X_Cp56time2a;
extern      uint8_t             Iec10x_FirmwareUpdateFlag;
/*
*********************************************************************************************************
*                                          QUEUE FUNCTION PROTOTYPES
*********************************************************************************************************
*/
/*
 * Prio
 * */
uint8_t             IEC10X_PrioEnQueue(Iec10x_PrioQueue_T *QueueHdr, Iec10x_PrioNode_T * new_p);
Iec10x_PrioNode_T   *IEC10X_PrioDeQueue(Iec10x_PrioQueue_T * QueueHdr);
char                IEC10X_HighestPrio(void);
void                IEC10X_PrioInitQueue(Iec10x_PrioQueue_T *PrioQueue);
void                IEC10X_Prio_ClearQueue(Iec10x_PrioQueue_T * QueueHdr);
Iec10x_PrioNode_T   *IEC10X_PrioFindQueueHead(Iec10x_PrioQueue_T * QueueHdr);
/*
 * Iec10x queue
 * */
void                IEC10X_InitQ(void);
void                IEC10X_ClearQ(void);
uint8_t             IEC10X_GetPrio(uint8_t State);
void                IEC10X_Enqueue(uint8_t *EnQBuf, uint16_t Length,uint8_t Prio,
                            void(* IEC10XCallBack)(Iec10x_CallbackArg_T *Arg),Iec10x_CallbackArg_T *CallbackArg);
void                Iec10x_Scheduled(int socketfd);
Iec10x_PrioNode_T   *IEC10X_Dequeue(void);
int32_t             RegisterIEC10XMoudle(void * _IEC10X);
void                IEC10X_ClearQ(void);

void Iec10x_Lock(void);
void Iec10x_UnLock(void);

#endif  /*_IEC10X_H*/
