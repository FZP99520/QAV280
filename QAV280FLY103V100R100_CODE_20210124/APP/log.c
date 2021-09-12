#include "log.h"
#include "usart.h"
#include "stdarg.h"
#include "stdio.h"
#define LOG_USE_USART2

u8 log_buff[128];

void log(void)
{
    #ifdef LOG_USE_USART1
        USART1_SendData(log_buff,sizeof(log_buff));
    #endif
    
    #ifdef LOG_USE_USART2
        USART2_SendData(log_buff,sizeof(log_buff));
    #endif
}





