# IEC104
Protocol of IEC104 and IEC101

##
by chendajie 2014-2015
##

电网IEC104/IEC101通信协议
==============================

隔离协议内容与平台或者硬件相关性，已经在stm32平台和linux平台移植测试通过，可轻松移植到其他平台。
============================================================================
队列实现的隔离，目前实现简单的先入先出队列，可根据需求用其他队列算法替换。

移植调用实现：

    1.在不同平台下，实现_iec10x结构体的函数指针（利用函数指针实现的接口）。
    
    2.调用RegisterIEC10XMoudle，注册实现的_iec10x接口。
    
    3.创建线程一调用Iex104_Receive管理收包解析。
    
    4.创建线程二调用Iec10x_Scheduled实现出队调度。
    
    5.创建线程三实现状态机Iec104_StateMachine，实现协议状态管理，协议包组包入队。

代码内容

    IEC0x目录
    
    iec101.c   iec101协议包内容
    
    iec104.c   iec104协议包内容
    
    iec10x.c   队列初始化，入队出对实现，队列优先级等相关实现，队列调度实现
    
    PRIO_QUEUE_Iec10x.c   队列具体算法实现
    
    test目录
    
    linux上简单测试代码，仅供参考，作者具体协议应用在stm32上，利用状态机控制数据包入队与出队调度
    
    
重要函数：

    Iec10x_Scheduled：出队调度
  
    IEC10X_Enqueue：入队函数
    
    IEC10X_Dequeue：出队函数
    
    IEC10X_FindQHead：查找最高优先级数据包
  
    IEC10X_XXX：前缀为IEC10X_的函数为各个协议包的组包函数
    
    Iec104_StateMachine： 104协议状态机
    
    Iex104_Receive：收包解析
    
    RegisterIEC10XMoudle：协议模块注册，初始化
    

重要结构体：

用函数指针实现的移植接口，根据不同平台与硬件特性，选择性实现以下接口

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

    
  
