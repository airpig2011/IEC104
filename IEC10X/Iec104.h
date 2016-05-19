

#ifndef _IEC104_H
#define _IEC104_H

#include "Iec10x.h"

#define     IEC104_MAX_BUF_LEN          256

#define     IEC104_HEAD                 0X68
/*
 * PRIO
 * */
#define             IEC104_PRIO_INITLINK            0
#define             IEC104_PRIO_CALLALL             1
#define             IEC104_PRIO_CALLGROUP           2
#define             IEC104_PRIO_CLOCK               3
#define             IEC104_PRIO_DELAY               3
#define             IEC104_PRIO_PULSE               0
#define             IEC104_PRIO_SPON                0

/*
 * Ctrl U Function
 * */
#define             IEC104_U_FUNC_STARTDT           0x07		//0B00000111
#define             IEC104_U_FUNC_STOPDT            0x13		//0B00010011
#define             IEC104_U_FUNC_TESTER            0x43		//0B01000011

#define             IEC104_U_FUNC_STARTDT_ACK       0x0b		//0B00001011
#define             IEC104_U_FUNC_STOPDT_ACK        0x23		//0B00100011
#define             IEC104_U_FUNC_TESTER_ACK        0x83		//0B10000011

/*
 * ASDU Type Upload station address
 * */
#define             IEC104_ASDU_TYPE_M_DTU_INF_1    180
#define             IEC104_INFO_SIGNATURE           0XEB90EB90


/*
 * Info Address
 * */
#define             IEC104_DEV_TYPE_HXGF               0X03
#define             IEC104_DEV_TYPE_HXTM               0X01

#define             IEC104_INFOADDR_VALUE_HXGF         0X004001
#define             IEC104_INFOADDR_VALUE_HXTM         0X004201

#define             IEC104_INFOADDR_STATE_HXGF         0X000001
#define             IEC104_INFOADDR_STATE_HXTM         0X000301

/* base device information */
#define             IEC104_INFOADDR_BASE_DEVINFO       0X102001

/*
 * device type 
 */
//#define       ENDDEVICE_TYPE_ERR                  0
//#define       ENDDEVICE_TYPE_HXPF                 2
#define         ENDDEVICE_TYPE_HXTM                 1
#define         ENDDEVICE_TYPE_HXGF                 3

#define         AP_TYPE_BASE_INFO                   4    

/* COT */
#define         AP_COT_BASE_INFO                    18  


#pragma pack(1)

/*
 *  Iec104 package
 * */
/* Control filed I type */
typedef struct {

    uint32_t Type:1;
    uint32_t SendSn:15;
    uint32_t Reserve:1;
    uint32_t RecvSn:15;
}IEC104_CTRL_I_T, *PIEC104_CTRL_I_T;

/* Control filed S type */
typedef struct {

    uint32_t Type1:1;
    uint32_t Type2:1;
    uint32_t Reserve:15;
    uint32_t RecvSn:15;
}IEC104_CTRL_S_T, *PEC104_CTRL_S_TP;

/* Control filed U type */
typedef struct {

    uint32_t Type1:1;
    uint32_t Type2:1;
    uint32_t Startdt:1;
    uint32_t StartdtAck:1;
    uint32_t Stopdt:1;
    uint32_t StopdtAck:1;
    uint32_t Tester:1;
    uint32_t TesterAck:1;
    uint32_t Reserve:24;
}IEC104_CTRL_U_T, *PIEC104_CTRL_U_T;

/* Control filed type */
typedef struct {

    uint32_t Type1:1;
    uint32_t Type2:1;
    uint32_t Reserve:30;

}IEC104_CTRLType_T, *PIEC104_CTRLType_T;

/* Control filed type */
typedef struct {

    uint32_t Func:8;
    uint32_t Reserve:24;

}IEC104_CTRLFunc_T, *PIEC104_CTRLFunc_T;

typedef union{

    IEC104_CTRL_I_T     I;
    IEC104_CTRL_S_T     S;
    IEC104_CTRL_U_T     U;
    IEC104_CTRLType_T   Type;
    IEC104_CTRLFunc_T   Func;
}IEC104_CTRL_T, *PIEC104_CTRL_T;

typedef struct{

    uint8_t         Head;
    uint8_t         Len;
    IEC104_CTRL_T   Ctrl;
    uint8_t         Asdu[1];
}IEC104_DATA_T,*PIEC104_DATA_T;

typedef struct {

    float Current;
    float Temperature;    
}Iec10x_DevA_Info_T, *PIec10x_DevA_Info_T;

#pragma pack()

/*
 * length
 * */
#define     IEC104_HEAD_LEN          (sizeof(IEC104_DATA_T)-1)
#define     IEC104_ASDU_LEN          (sizeof(IEC10X_ASDU_T)-1)
#define     IEC104_INFO_LEN          (sizeof(ASDU_INFO_T)-1)
#define     IEC104_DATA_LEN          (IEC104_HEAD_LEN+IEC104_ASDU_LEN+IEC104_INFO_LEN)


enum {
    IEC104_FLAG_RECV_CLOSED,        /*0*/
    IEC104_FLAG_SEND_CLOSED,        /*1*/
    IEC104_FLAG_CLOSED,             /*2*/
    IEC104_FLAG_LINK_INIT,          /*3*/
    IEC104_FLAG_IDLE,               /*4*/
    IEC104_FLAG_UPLOAD_ADDR,             /*5*/
    IEC104_FLAG_START_LINK,             /*6*/
    IEC104_FLAG_REQ_LINK,             /*7*/
    IEC104_FLAG_RESET_REMOTE_LINK,             /*8*/
    IEC104_FLAG_INIT_FIN,             /*9*/
    IEC104_FLAG_CALL_ALLDATA,             /*10*/
    IEC101_FLAG_CALL_GROURPDATA,             /*11*/
    IEC104_FLAG_CALL_ACT_FIN,             /*12*/
    IEC104_FLAG_CALL_SIG_TOTAL,             /*13*/
    IEC104_FLAG_CALL_DET_TOTAL,             /*14*/
    IEC104_FLAG_CLOCK_SYS,             /*15*/
    IEC104_FLAG_TESTER,             /*16*/
    IEC104_FLAG_TESTER_STOP,             /*17*/
    IEC104_FLAG_CALL_GROUP,             /*18*/
    IEC104_FLAG_CONNECT_SUCESS,             /*19*/
    IEC104_FLAG_S_ACK,             /*20*/
};

/*
*********************************************************************************************************
*                          EXTERN  VARIABLE
*********************************************************************************************************
*/
extern      uint8_t             IEC104_STATE_FLAG_INIT;
extern      uint8_t             IEC104_STATE_FLAG_CALLALL;
extern      uint8_t             IEC104_STATE_FLAG_GROUP;
extern      uint8_t             IEC104_STATE_FLAG_CLOCK;
extern      uint8_t             IEC104_STATE_FLAG_PULSE;

/*
*********************************************************************************************************
*                          FUNCTION
*********************************************************************************************************
*/
uint8_t         Iec104_StateMachine(void);
uint8_t         IEC104_BuildSignal_Spon(uint8_t TimeFlag, uint8_t signalV, uint16_t addrV);
uint8_t         IEC104_BuildDetectF_Spont(uint8_t TimeFlag, float detectV, uint16_t addrV);
void            Iex104_Receive(uint8_t *buf, uint16_t len);
uint8_t         IEC104_BuildDetect_Spont(uint8_t TimeFlag, PIEC10X_DETECT_T detectV, uint16_t addrV);

#endif
