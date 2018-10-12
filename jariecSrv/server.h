#ifndef _J_SERVER_
#define _J_SERVER_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include <ctype.h>
#define UART_LTE	"/dev/ttySP3"
#define UART_ZIGBEE	"/dev/ttySP0"
#define VERSION		"0.0.4"
#define DATABASE	"light.db"
#define ARMCFG		"arm.db"
#define POWERCFG	"power.db"
#define QNQIAOCFG	"anqiao.db"
#define sw16(x) ((short)((((short)(x)&(short)0x00ffU)<<8)|(((short)(x)&(short)0xff00U) >> 8 )))  

//#define _DBGMSG 0
#if 0
#ifndef _DBGMSG
#define DBGMSG(fmt,args...) printf(fmt,##args)
#else
#define DBGMSG(fmt,args...) ((void)0)
#endif
#endif
//#define NDEBUG 0
#ifdef NDEBUG
#define JLOG(fmt, args...) printf(fmt, ##args)
#else
#define JLOG(fmt, args...) 
#endif

#define HEELO_WORLD_SERVER_IP	   "115.159.209.106"
#define HELLO_WORLD_SERVER_PORT    5001 

#define SOCK_HEAD_SIZE  4
struct packData{
        unsigned char start;
        unsigned char cmd;
        unsigned char lenH;
        unsigned char lenL;
        int len;
        unsigned char *data;
};


struct LightInfo{
	char mac[64];
        char id;
        char value;    /*0x0 closed, 0x01-0x64 value, 0x65-error*/
	int status;
	char netAddr[8];
};
struct DeviceInfo{
	char armId[16];
	short panId;
	short netId;
	int status;
	unsigned char chanId;
};
struct ControlInfo{
	char id[16];
	int count;
	struct LightInfo li[32];
};


struct QnqiaoZoon{
	int start;
	int end; 
	int status;
};

struct QnqiaoZoonInfo{
	int count;
	struct QnqiaoZoon az[32];
};

#if 0
int show_hex(unsigned char *buffer, int length, const char *title)
{
    int i;
    printf("\n-----------%s(%d)---------------\n", title, length);
    for(i=0; i<length;i++){
        printf("0x%02x ", buffer[i]);
    }
    printf("\n--------------------------\n");
}
#endif
#endif
