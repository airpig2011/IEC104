#ifndef __MAIN_H__
#define __MAIN_H__

#define MAXLINE     1500
#define MAXNUM      10000
#define SERV_PORT   10000

//#define IEC10XLOCK

extern int listenfd,connfd;

extern int sfp,nfp;

extern int socketfd[MAXNUM];

extern pthread_mutex_t mutex;


#define VERSION     0.1
#define NAME        Iec104_monitor

#define SOCKET_MODE_SERVER      0
#define SOCKET_MODE_CLIENT      1

typedef struct{

    int socketfd;
    unsigned short int port;
    unsigned int addr;

}SocketArg_T, *PSocketArg_T;

typedef struct{

    int socketfd;
    int ClientId;
}ClientInfo_T,*PClientInfo_T;

#endif
