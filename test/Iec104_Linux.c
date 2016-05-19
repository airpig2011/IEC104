/*******************************************************************
Copyright (C):    
File name    :    HXCP_STM32.C
DESCRIPTION  :
AUTHOR       :
Version      :    1.0
Date         :    2014/02/26
Others       :
History      :
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
1) Date: 2014/02/26         Author: ChenDajie
   content:

*******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/in.h>


#include  <errno.h>


#include "Iec10x.h"
#include "sys.h"
#include "PRIO_QUEUE_Iec10x.h"
#include "main.h"


/*******************************************************************************
* Function Name  : stm32f103_init
* Description    : stm32f103_init program
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static int Linux_init(void) {


    return 0;
}

static void *Linux_Malloc(unsigned char NumByte){

    return malloc(NumByte);
}


static void Linux_Free(void *pblock){

    free(pblock);
}



static uint32_t Linux_SetTime(PCP56Time2a_T time){

    return RET_SUCESS;
}
static uint32_t Linux_GetTime(PCP56Time2a_T time){

    return RET_SUCESS;
}


static int8_t Linux_GetStationInfo(uint16_t *Addr,uint8_t n,uint8_t *MaxNum){

    *MaxNum = WTP_SUPPORT_END_NUMBER;
    *Addr = n;
    return RET_SUCESS;
}

static float Linux_GetStaTemp(uint16_t Addr){

    return 30;
}

static uint16_t Linux_GetLinkAddr(void){

    return 0x0001;
}

static void Linux_CloseLink(void){

    //GPRSFlag = SYSTEM_FLAG_GPRS_CLOSE;
    //IEC104_STATE_FLAG_INIT = IEC104_FLAG_CLOSED;
}

void delay_ms(uint16_t ms){

    usleep(ms*1000);
}

uint8_t Linuxsend(int socketfd,char *buf, int len){

    if(-1 == write(socketfd,buf,len)){
        LOG("-%s-,Send error \n",__FUNCTION__);
        return RET_ERROR;
    }
    printf("Send Ok!\r\n");
    
    return RET_SUCESS;
}
void LinuxLock(void){

    pthread_mutex_lock(&mutex);
}
void LinuxUnlock(){

    pthread_mutex_unlock(&mutex);
}
/*******************************************************************************
* Function Name  : stm32f103
* Description    : stm32f103
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static IEC10X_T Linux = {
    "Linux",
    Linux_init,
    delay_ms,
    Linux_CloseLink,
    Linux_Malloc,
    Linux_Free,
    IEC10X_PrioEnQueue,
    IEC10X_PrioDeQueue,
    IEC10X_PrioFindQueueHead,
    IEC10X_HighestPrio,            /* Get the highest Prio Queue*/
    IEC10X_PrioInitQueue,
    IEC10X_Prio_ClearQueue,
    Linuxsend,
    Linux_SetTime,
    Linux_GetTime,
    Linux_GetStationInfo,
    Linux_GetStaTemp,
    Linux_GetLinkAddr,

#ifdef IEC10XLOCK
    LinuxLock,
    LinuxUnlock
#endif
};
/*******************************************************************************
* Function Name  : mstm32f103RegisterShtxxain
* Description    : Main program
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
uint32_t Stm32f103RegisterIec10x(void){
    return RegisterIEC10XMoudle(&Linux);
}
