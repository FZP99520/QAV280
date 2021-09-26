#include "log.h"
#include "usart.h"
#include "stdarg.h"
#include "stdio.h"
#include "string.h"

#define DebugLog_USE_USART2

#define LOG_Buff_Size_MAX 64
char Log_Buff[LOG_Buff_Size_MAX];

#define DebugLog_EN 1

#if DebugLog_EN==1
void DebugLog(const char* format,...)
{
    va_list args;
    va_start(args,format);
    memset(Log_Buff,0,LOG_Buff_Size_MAX);
    vsnprintf((char*)Log_Buff,LOG_Buff_Size_MAX+1,(const char*)format,args);
    va_end(args);
    #ifdef DebugLog_USE_USART1
        USART1_SendData((char*)Log_Buff,sizeof(Log_Buff));
    #elif defined (DebugLog_USE_USART2)
        USART2_SendData((char*)Log_Buff,sizeof(Log_Buff));
    #endif
}
#elif DebugLog_EN==0
void DebugLog(const char* format,...)
{
    return;
}
#endif





