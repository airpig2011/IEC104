
#ifndef __SYS_H
#define __SYS_H	
#include "common_datatype.h"

#define WTP_SUPPORT_END_NUMBER  9


#define RET_SUCESS  0
#define RET_ERROR   1

//#define IEC101_STM32
#define IEC104_STM32
//#define IEC104_STM32_FUJIAN_HX

//#define SIM900_MODE_SERVER
#define SIM900_MODE_CLIENT

#define SERIAL_DEBUG

#ifdef SERIAL_DEBUG
#define log(...) printf(__VA_ARGS__)
#define LOG(...) printf(__VA_ARGS__)
#define LogInit()
#define PRINT_FUNLINE printf("%s %d\r\n",__FUNCTION__,__LINE__)
#else
#define log(...)
#define LOG(...)
#define DumpHEX(buffer,len)
#define PRINT_FUNLINE
#endif


#ifdef  SERIAL_DEBUG
void DumpHEX(uint8_t *buffer, uint32_t len);
#endif

#endif
