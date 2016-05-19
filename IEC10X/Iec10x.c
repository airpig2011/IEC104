#include "Iec10x.h"
#include "sys.h"

PIEC10X_T                       IEC10X = NULL;
uint16_t                        Iec10x_Sta_Addr = 0;
Iec10x_PrioQueue_T              Iec10x_PrioQueueArray[IEC10X_PRIO_MAX];

uint16_t            IEC10X_Cp16time2a = 0;
uint16_t            IEC10X_Cp16time2a_V = 0;
CP56Time2a_T        IEC10X_Cp56time2a;


/*******************************************************************************
* Function Name  : IEC10X_InitQ
* Description    : Init queue.
* Input          : None
* Output         : None
* Return         : 0 failure
                   len sucess
*******************************************************************************/


void Iec10x_Lock(void){

#ifdef IEC10XLOCK
    if(IEC10X->LOCK != NULL)
        IEC10X->LOCK();
#endif
}
void Iec10x_UnLock(void){

#ifdef IEC10XLOCK
    if(IEC10X->UNLOCK != NULL)
        IEC10X->UNLOCK();
#endif
}
void IEC10X_InitQ(void){

    uint8_t i;
    for(i=0; i<IEC10X_PRIO_MAX; i++){
        IEC10X->InitQueue(&Iec10x_PrioQueueArray[i]);
    }
}
void IEC10X_ClearQ(void){
    uint8_t i;
    for(i=0; i<IEC10X_PRIO_MAX; i++){
        IEC10X->ClearQueue(&Iec10x_PrioQueueArray[i]);
    }

}

uint8_t IEC10X_GetPrio(uint8_t State){

    /*Prio from 0~7*/
    uint8_t Prio = 0;

    return Prio;

}

void IEC10X_Enqueue(uint8_t *EnQBuf, uint16_t Length,uint8_t Prio,
                    void(* IEC10XCallBack)(Iec10x_CallbackArg_T *Arg),Iec10x_CallbackArg_T *CallbackArg){

    Iec10x_PrioNode_T * new_p;

    Iec10x_Lock();
    if(Length < IEC10X_HEADER_LENGTH){
        LOG("IEC10X_Enqueue,buffer too short \r\n");
        goto END;
    }

    new_p = (Iec10x_PrioNode_T *)IEC10X->Malloc(sizeof(Iec10x_PrioNode_T)+Length-1);
    if (new_p == NULL ){
        LOG("IEC10X_Enqueue,malloc error \r\n");
        goto END;
    }

    memcpy(new_p->value, EnQBuf,Length);
    new_p->Length = Length;

    /* Prio from 1~8, Array from 0~7*/
    if(Prio>=IEC10X_PRIO_MAX ){
        LOG("IEC10X_Enqueue, error Prio(%d) \r\n", Prio);
        goto END;
    }

    /* Set callback Argument */
    new_p->CallBack = IEC10XCallBack;
    if(CallbackArg != NULL)
        new_p->CallBackArg = *CallbackArg;

    new_p->CallBackArg.value = new_p->value;
    LOG("IEC10X_Enqueue,Prio(%d) elementNum(%d)len(%d)(%d) \r\n",Prio,Iec10x_PrioQueueArray[Prio].ElementNum,Length,new_p->Length);
    //DumpHEX(new_p->value, new_p->Length);
    IEC10X->enqueue(&Iec10x_PrioQueueArray[Prio], new_p);
END:
    Iec10x_UnLock();
    return;
}

Iec10x_PrioNode_T *IEC10X_Dequeue(void){
    uint8_t Prio;
    Iec10x_PrioNode_T * ret;
    Prio = IEC10X->GetPrio();
    if(Prio >=8){
        //LOG("IEC10X_Dequeue, Error Prio(%d) \r\n",Prio);
        return 0;
    }
    ret = IEC10X->dequeue(&Iec10x_PrioQueueArray[Prio]);
    if(ret){
        Iec10x_PrioQueueArray[Prio].ElementNum --;
    }
    //LOG("IEC10X_Dequeue(%d) \r\n",ret->Length);
    return ret;
}
Iec10x_PrioNode_T *IEC10X_FindQHead(void){

    uint8_t Prio;
    Iec10x_PrioNode_T * ret;
    Prio = IEC10X->GetPrio();
    if(Prio >=8){
        return 0;
    }
    ret = IEC10X->FindQHead(&Iec10x_PrioQueueArray[Prio]);
    return ret;
}

void Iec10x_Scheduled(int socketfd){

    Iec10x_PrioNode_T *DeQNode;

    DeQNode = IEC10X_Dequeue();

    if(DeQNode){

        LOG("<<-------------------IEC10X (%d)-------------------->> \r\n",DeQNode->Length);

        /* call back funtion */
        if(DeQNode->CallBack)
            DeQNode->CallBack(&(DeQNode->CallBackArg));

        DumpHEX(DeQNode->value, DeQNode->Length);
        IEC10X->Send(socketfd,(char *)(DeQNode->value),DeQNode->Length);

        Iec10x_Lock();
        IEC10X->Free(DeQNode);
        Iec10x_UnLock();
    }
}
/*******************************************************************************
* Function Name  : RegisterSim900aMoudle
* Description    : RegisterSim900aMoudle program
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
int32_t RegisterIEC10XMoudle(void * _IEC10X){

    int ret;
    if (NULL == _IEC10X) {
        return -1;
    }else{
        IEC10X = (PIEC10X_T)_IEC10X;
        if (NULL == IEC10X->Init) {
            return -1;
        }else{
            IEC10X_InitQ();
            //IEC10X_InitInfo();
            ret = IEC10X->Init();
            if (0 == ret) {
#ifdef IEC101_STM32
                LOG("\r\nRegister \"%s\" IEC101 Success, < HuiXing 2014-2015 > ...\r\n",IEC10X->name);
#elif defined(IEC104_STM32)
                LOG("\r\nRegister \"%s\" IEC104 Success, < HuiXing 2014-2015 > ...\r\n",IEC10X->name);
#endif
            }
            return ret;
        }
    }
}
