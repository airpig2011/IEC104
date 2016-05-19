 /* server.c */


#include <stdlib.h>
#include <sys/types.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include "unistd.h"

#include "errno.h"
#include "sys.h"
#include "main.h"

pthread_mutex_t mutex;
pthread_mutex_t mutex2;

int listenfd,connfd;
int sfp,nfp;
//int socketfd[MAXNUM] = {0};

unsigned int StaCount = 0;



void DumpHEX(uint8_t *buffer, uint32_t len){

    uint32_t i;
    printf("[DumpHEX]Length:%d ",len);
    for(i=0; i<len; i++){
        if(i%16 == 0)
            printf("\r\n");
        else if(i%8 == 0)
            printf("    ");
        else
            printf(":");
        printf("%02x",buffer[i]);
    }
    printf("\r\n");
}

void *Iec104_Task(void *arg){

    int socketfd = (*(int *)arg);
    while(1){
        //pthread_mutex_lock(&mutex2);
        Iec10x_Scheduled(socketfd);      
        Iec104_StateMachine();
        //pthread_mutex_lock(&mutex2);
    }
    return ((void *)0); 
}


void *Iec104_main(void *arg){

    struct sockaddr_in s_add,c_add;
    int sin_size,on=1;
    char Iec104_RecvBuf[MAXLINE];
    uint16_t Iec104_RecvLen;
    pthread_t tid1,tid2; 
    int err;  
    void *tret;  

    unsigned int addr =  inet_addr("0.0.0.0");
    unsigned short portnum=((PSocketArg_T)arg)->port;
    
    
    int socketfd = 0;
    //socketfd = *((int *)arg);
    

    
    sfp = socket(AF_INET, SOCK_STREAM, 0);
    if(-1 == sfp){
        printf("socket fail ! \r\n");
        return;
    }

    LOG("Iec104 Socket Ok(%d) !\r\n",portnum);

    setsockopt(sfp, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) );
    
    bzero(&s_add,sizeof(struct sockaddr_in));
    s_add.sin_family=AF_INET;
    s_add.sin_addr.s_addr=htonl(((PSocketArg_T)arg)->addr);
    s_add.sin_addr.s_addr=htonl(INADDR_ANY);
    s_add.sin_port=htons(portnum);

    if(-1 == bind(sfp,(struct sockaddr *)(&s_add), sizeof(struct sockaddr))){
        printf("Bind fail, Port(%d), err(%d)  !\r\n",portnum, errno);
        //printf("Bind fail, Port(%d), err  !\r\n",portnum);
        return;
    }
    LOG("Iec104 Bind Ok(%d) !\r\n",portnum);

    if(-1 == listen(sfp,5)){
        printf("listen fail (%d)!\r\n",portnum);
        return;
    }
    printf("Iec104 Listen Ok(%d)\r\n", portnum);

    while(1){
        sin_size = sizeof(struct sockaddr_in);

        nfp = accept(sfp, (struct sockaddr *)(&c_add), &sin_size);
        if(-1 == nfp){
            printf("accept fail !\r\n");
            return;
        }
        
        err=pthread_create(&tid1,NULL,Iec104_Task,&nfp);
        if(err!=0)  {  
                LOG("pthread_create error:%s\n",strerror(err));  
                exit(-1);  
        } 
        
        printf("Accept ok!\r\nServer start get connect from %#x : %#x\r\n",ntohl(c_add.sin_addr.s_addr),ntohs(c_add.sin_port));
        while(1){
            Iec104_RecvLen = read(nfp,Iec104_RecvBuf,MAXLINE);
            if(Iec104_RecvLen <= 0 || Iec104_RecvLen>1500){
                //printf("the other side has been closed (%d).\n",Iec104_RecvLen);
                continue;
            }
            LOG("#####################received \n");

            DumpHEX(Iec104_RecvBuf,Iec104_RecvLen);
            Iex104_Receive(Iec104_RecvBuf, Iec104_RecvLen);

        }
        close(nfp);
    }
    close(sfp);
    return;
}

uint32_t sta_temp = 0;

void *Iec104_Client(void *arg)  
{  
    int  n,rec_len;   
    uint16_t Iec104_RecvLen;
    struct sockaddr_in    servaddr;  
    //uint8_t *dstip = "220.160.62.206";
    int err;
    uint8_t *dstip = "192.168.1.114";
    pthread_t tid1; 
    
    //int staid = ((PClientInfo_T)arg)->ClientId;
    int staid = StaCount++;
    
    int socketfd = 0;
    socketfd = 0;

    printf("-----------------------station ID :%d \n",staid);
    
    err=pthread_create(&tid1,NULL,Iec104_Task,&socketfd);
    if(err!=0)  {  
            LOG("pthread_create error:%s\n",strerror(err));  
            exit(-1);  
    }    
     
    char Iec104_RecvBuf[MAXLINE];  
    if((socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){  
        printf("create socket error: %s(errno: %d)\n", strerror(errno),errno);  
        exit(0);  
    }  
  
  
    memset(&servaddr, 0, sizeof(servaddr));  
    servaddr.sin_family = AF_INET;  
    servaddr.sin_port = htons(6666);  
    if(inet_pton(AF_INET, dstip, &servaddr.sin_addr) <= 0){  
        printf("inet_pton error for %s\n",dstip);  
        exit(0);  
    }  

    if(connect(socketfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0){  
        printf("connect error: %s(errno: %d)  num(%d)\n",strerror(errno),errno,sta_temp++);  
        return; 
    }
    //LOG("sta(%d) Connect OK \n",);

    while(1){
        Iec104_RecvLen = read(socketfd,Iec104_RecvBuf,MAXLINE);
        if(Iec104_RecvLen <= 0 || Iec104_RecvLen>1500){
            //printf("the other side has been closed (%d).\n",Iec104_RecvLen);
            continue;
        }
        LOG("#####################sta(%d) received \n",staid);
        
        /* mutex lock*/
        pthread_mutex_lock(&mutex);
        DumpHEX(Iec104_RecvBuf,Iec104_RecvLen);      
        Iex104_Receive(Iec104_RecvBuf, Iec104_RecvLen);
        pthread_mutex_unlock(&mutex);

    }
    close(socketfd); 
 
    exit(0);  
}


/* Prints the help screen to stdout */

void Iec104_printhelp(void)
{
    printf("HuiXing Iec104 Monitor V_0.1 .\n");
    printf("Author:DajieChen .\n");
    printf("Basic usages:\n");
    printf("  Iec104_monitor [options] hostname port [port] ...\n");
    printf("\n");
    printf("Options:\n"
"    -d,                   the server ip\n"
"    -m,                   select the socket mode (server/client)\n"
"    -n,                   number of the station\n"
"    -p,                   the port\n");
    printf("\n");
}

int main(int argc, char *argv[]){

    pthread_t tid1,tid2; 
    int err;  
    void *tret;  
    SocketArg_T Arg1;
    unsigned int addr1 = 0,addr2 = 0;
    int opt;  
    char *optstring = "p:d:n:m:h";  
    unsigned short port1 = 10000;
    unsigned short port2 = 10001;
    addr1 =  inet_addr("192.168.1.114");
    addr2 =  inet_addr("192.168.1.114");
    uint32_t Addr = 0,i;
    void *ptemp = NULL;
    
    Arg1.port = 10000;
    Arg1.addr = addr1;
    
    
    uint8_t mode = SOCKET_MODE_CLIENT;
    uint8_t DstIp[32] = {0};
    uint32_t Port = 0;
    uint32_t NumSta = 0;
    
    ClientInfo_T clientinfo;
 
    Stm32f103RegisterIec10x(); 

    while ((opt = getopt(argc, argv, optstring)) != -1){ 
    #if 0 
        printf("opt = %c\n", opt);  
        printf("optarg = %s\n", optarg);  
        printf("optind = %d\n", optind);  
    #endif    
        switch(opt){
        
            case 'h':
                Iec104_printhelp();
                return;
                
            case 'd':
                if(optarg == NULL){
                    printf("-d need argument \n");
                    break;
                }
                Addr =  inet_addr(optarg);
                strcpy(DstIp,optarg);
                break;
            case 'm':
                if(strcmp("server", optarg) == 0)
                    mode = SOCKET_MODE_SERVER;
                else if(strcmp("client", optarg) == 0)
                    mode = SOCKET_MODE_CLIENT;
                else{
                    LOG("mode error (%s) \n",optarg);
                    return;
                }
                    
                break; 
                
            case 'p':
                if(optarg == NULL){
                    printf("-p need argument \n");
                    break;
                }
                sscanf(optarg, "%d", &Port);
                break;
            case 'n':
                if(optarg == NULL){
                    printf("-n need argument \n");
                    break;
                }
                sscanf(optarg, "%d", &NumSta);
                break;
            default :
                break;
        }
    }  
    
    LOG("mode :(%d), port: (%d), ip: (%s), station num: (%d) \n",mode, Port,DstIp,NumSta);   
    
    
    pthread_mutex_init(&mutex,NULL);
    pthread_mutex_init(&mutex2,NULL);
    
    
    if(NumSta<=0 || NumSta>10000){
    
        LOG("station number error:%d \n",NumSta);
    } 
    
    if(mode == SOCKET_MODE_SERVER){
        LOG("Iec104 Server Mode \n");
        err=pthread_create(&tid1,NULL,Iec104_main,&Arg1);
        if(err!=0)  {  
            LOG("pthread_create error:%s\n",strerror(err));  
            exit(-1);  
        } 
     }else if(mode == SOCKET_MODE_CLIENT){
        LOG("Iec104 Client Mode \n");
        for(i=0; i<NumSta; i++){
            //err=pthread_create(&tid1,NULL,Iec104_Client,&socketfd[i]);
            clientinfo.ClientId = i;
            clientinfo.socketfd = 0;
            //printf("@@@@@@@@@@@@@@@@create thread:%d \n",i);
            err=pthread_create(&tid1,NULL,Iec104_Client,&clientinfo);
            if(err!=0)  {  
                printf("pthread_create error:%s\n",strerror(err));  
                exit(-1);  
            } 
        }
     }

    
    while(1);
    //Iec104_main(ptemp);
    
    return 0;
}




