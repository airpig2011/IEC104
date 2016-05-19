/*******************************************************************
Copyright (C):    
File name    :    PRIO_QUEUE.C
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

#include "sys.h"
#include "Iec10x.h"
#include "PRIO_QUEUE_Iec10x.h"
#include "common_datatype.h"
//#include "includes.h"

extern PIEC10X_T IEC10X;
extern Iec10x_PrioQueue_T Iec10x_PrioQueueArray[IEC10X_PRIO_MAX];

#ifdef PRIO_QUEUE

/*******************************************************************************
* Function Name  : HighestPrio
* Description    : 
* Input          : None
* Output         : None
* Return         : RET_ERROR failure
                   len sucess
*******************************************************************************/
char IEC10X_HighestPrio(void){

    int i,prio;
    uint8_t flag = 0;
    prio = -1;
    //LOG("%s \r\n",__FUNCTION__);
    for(i=0; i<IEC10X_PRIO_MAX; i++){
        if(Iec10x_PrioQueueArray[i].ElementNum > 0  && !flag){
            //return i;
            prio = i;
            flag = 1;
            //LOG("<%s>Prio[%d],Num%d. \n",__FUNCTION__,i,Iec10x_PrioQueueArray[i].ElementNum);
        }
        //LOG("[%d]%d.",i,PrioQueueArray[i].ElementNum);
    }
    //LOG("out(%d)\r\n",prio);
    return prio;
}

void IEC10X_PrioInitQueue(Iec10x_PrioQueue_T *PrioQueue){

    PrioQueue->Header = NULL;
    PrioQueue->Tail = NULL;
    PrioQueue->ElementNum = 0;
}
uint8_t IEC10X_PrioEnQueue(Iec10x_PrioQueue_T *QueueHdr, Iec10x_PrioNode_T * new_p){

    new_p->Next = NULL;

    if (QueueHdr->Header == NULL){
        QueueHdr->Header = new_p;
        QueueHdr->Tail = new_p;
    } else {
        QueueHdr->Tail->Next = new_p;
        QueueHdr->Tail = new_p;
    }

    QueueHdr->ElementNum ++;
    //LOG("%s ElementNum(%d) \r\n",__FUNCTION__,QueueHdr->ElementNum);
    return RET_SUCESS;
}

Iec10x_PrioNode_T *IEC10X_PrioDeQueue(Iec10x_PrioQueue_T * QueueHdr){

    Iec10x_PrioNode_T *PrioNode_DeQ;
    if (QueueHdr->Header == NULL){
        //LOG("PrioDeQueue,error \r\n");
        return NULL;
    }
    PrioNode_DeQ = QueueHdr->Header;

    QueueHdr->Header = QueueHdr->Header->Next;
    if(QueueHdr->Header == NULL){
        QueueHdr->Tail = NULL;
    }

    //LOG("PrioDeQueue(%d) \r\n",PrioNode_DeQ->Length);

    return PrioNode_DeQ;
}

Iec10x_PrioNode_T *IEC10X_PrioFindQueueHead(Iec10x_PrioQueue_T * QueueHdr){

    Iec10x_PrioNode_T *PrioNode_DeQ;
    if (QueueHdr->Header == NULL){
        //LOG("PrioDeQueue,error \r\n");
        return NULL;
    }
    PrioNode_DeQ = QueueHdr->Header;

    //LOG("PrioDeQueue(%d) \r\n",PrioNode_DeQ->Length);

    return PrioNode_DeQ;
}


void *IEC10X_PeekQueue(Iec10x_PrioQueue_T * QueueHdr){

    if (QueueHdr->Header == NULL){
        //LOG("   ");
        return NULL;
    }
    return QueueHdr->Header->value;
}

int IEC10X_Prio_IsEmptyQueue(Iec10x_PrioQueue_T * QueueHdr){

    if (QueueHdr->Header == NULL){
        return 1;
    } else {
        return 0;
    }
}

void IEC10X_Prio_ClearQueue(Iec10x_PrioQueue_T * QueueHdr){

    Iec10x_PrioNode_T * p = QueueHdr->Header;
    while(p != NULL){ 
        QueueHdr->Header = QueueHdr->Header->Next;
        IEC10X->Free(p);
        p = QueueHdr->Header;
    } 
    QueueHdr->Tail = NULL;
    QueueHdr->ElementNum = 0;
    return;
}

#endif


