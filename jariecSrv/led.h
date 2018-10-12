#include <stdlib.h>   
#include <stdio.h>      
#include <stdbool.h>
#include <assert.h>     
#include <string.h>     
#include <unistd.h>    
#include <sys/time.h>  
#include <pthread.h> 
#include <fcntl.h>  
#include <sys/ioctl.h> 
#include <termios.h> 

#ifndef _LED_PUBLIC_H
#define _LED_PUBLIC_H

#define NONE                    0
#define ODD                     1
#define EVEN                    2
#define _DBGMSG 0		//此开关打开表明不打印更多的调试信息
#ifndef _DBGMSG
#define DBGMSG(fmt,args...) printf(fmt,##args)
#else
#define DBGMSG(fmt,args...) ((void)0)
#endif

//线程定义
extern pthread_t g_sNetworkThread, g_sModbusThread, g_sZigbeeThread, g_sGpsThread,g_ModbusNodata;//2017/8/30

extern unsigned char g_ucAnQiaoAddr;

extern int g_iLocation;

//#####################
//   外部函数定义
//#####################
extern void func_network_thread(int *iFd);
extern void func_modbus_thread(int *iFd);
extern void func_zigbee_thread(int *iFd);
extern void func_gps_thread(int *iFd);
extern void func_modbusnodata_thread(void);//2017/8/30
extern void func_calc_power_thread(int *iFd);
extern int show_hex(unsigned char *buffer, int length, const char *title);
#endif

