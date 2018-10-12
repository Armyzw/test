#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
//#include "serial.h"

#define NDEBUG 0

#ifdef NDEBUG
#define JLOG(fmt, args...) printf(fmt, ##args)
#else
#define JLOG(fmt, args...)
#endif

//#include "server.h"
#include "usrLTE.h"

int getResponsebySerial(int fd, const char *cmd, char *buffer,int size, int nSec)
{
	write(fd, cmd, strlen(cmd));
	if(nSec<10)
		sleep(nSec);
	else
		usleep(nSec);
	return read(fd, buffer, size);
	
}

int LTEXSend(int fd, const unsigned char *data, int length)
{
	
    return write(fd, data, length);
}

int nRead(int fd, unsigned char *data, int size, int nsec)
{
        struct timeval tv;
        fd_set sRset;
        int nLen = 0;
        tv.tv_sec=0;
        tv.tv_usec=nsec;

        FD_ZERO(&sRset);
        FD_SET(fd,&sRset);
        if (select(fd+1, &sRset , NULL , NULL , &tv) >0)
        {
           usleep(20*1000);
           nLen = read(fd, data, size);
        }
        return nLen;
}

int LTEXRecv(int fd, unsigned char *data, int size)
{
    return nRead(fd, data, size, 50*1000);
}


int LTEXClose(int fd)
{
    return close(fd);
}

int checkSOCKConnect(int fd)
{
    char cmd[32] = "usr.cnAT+SOCKALK\r\n";
    char buffer[64] = "";
    int n, ret=-1;
    n = getResponsebySerial(fd, cmd, buffer, 
					sizeof(buffer), 100*1000);

    JLOG("checkXCon, %s\n", buffer);
    //show_hex(buffer, n, "checkSOCK");
    if(n>0 && strstr(buffer,"ON")){
	ret =0;
    }
    return ret;
}

int resetSOCKConnect(int fd)
{
   char cmdOnOff[2][32] = {"usr.cnAT+SOCKAEN=OFF\r\n",
			"usr.cnAT+SOCKAEN=ON\r\n"};
   char buffer[64] = "";
   int i, ret, n;
	for(i=0;i<2;i++){
	    n = getResponsebySerial(fd, cmdOnOff[i], buffer, 
					sizeof(buffer), 10*1000);
		if(n>0 && strstr(buffer, "OK")){
			//JLOG("resetXCon, %s%s",cmdOnOff[i], buffer);
			//show_hex(buffer, n, "reset");
			ret = 0;		
		}else{
			ret = -1;
		}
		if(i==0)
		    sleep(1);
    	}
    return ret;
}

int LTEXInit(int fd)
{
    int i, n;
    int ret = 0;
    int count = 2;
    char cmdList[6][64] = {"usr.cnAT+SOCKIND=ON\r\n",
			   "usr.cnAT+HEARTEN=OFF\r\n",};
    char buffer[128] = "";
    for(i=0;i<count;i++){
	write(fd, cmdList[i], strlen(cmdList[i]));
	memset(buffer, 0, sizeof(buffer));
    	n = getResponsebySerial(fd, buffer, buffer, 
					sizeof(buffer), 100*1000);
	if(strstr(buffer, "OK"))
		ret = 0 ;
	else
		ret = -1;
    }	
    return ret;
}
#if 1
int LTEXConnect(int fd, const char *ipaddr, short port)
{
    int ret = -1;
    int n;
    int i;
    char buffer[64]="usr.cnAT+SOCKA\r\n";
    char temp[64] = "";
    char cmdList[6][64] = {"usr.cnAT+WKMOD=NET\r\n",
                           "usr.cnAT+SOCKAEN=ON\r\n",
                           "",
                           "usr.cnAT+SOCKASL=LONG\r\n",
                           "usr.cnAT+Z\r\n",
                           "usr.cnAT+SOCKALK\r\n"};

    sprintf(cmdList[2], "usr.cnAT+SOCKA=TCP,%s,%d\r\n", ipaddr, port);

    LTEXInit(fd);
    sprintf(temp, "%s,%d", ipaddr, port);
    //printf("open %s on %d\n", UART_LTE, gUsrLTEFd);
    n = getResponsebySerial(fd, buffer, buffer, 
					sizeof(buffer), 100*1000);
        if(n>0 && strstr(buffer, temp)){
		JLOG("XConnect, ipaddr has setted, restart connect\n");
		n = resetSOCKConnect(fd);
		if(n==0)
			JLOG("Xconnect, reset connect ok\n");	
		else
			printf("Xconnect, reset failed\n");
                return 1;
	}

        for(i=0; i<5; i++){	    
	    n = getResponsebySerial(fd, cmdList[i], buffer, 
					sizeof(buffer), 100*1000);

            if(n>0 && strstr(buffer, OK)){
                continue;
            }
            else{
                printf("send error when %s", cmdList[i]);
                close(fd);
                return ret;
            }

            JLOG("Xconnect, recv:%s", buffer);
        }
	#if 0
        int count = 15;
        while(count--){
	    n = getResponsebySerial(gUsrLTEFd, cmdList[5], buffer, 
					sizeof(buffer), 100*1000);
            if(n>0 && strstr(buffer, SOCKA_CONNECTED)){
                ret =0;
                break;
            }
            sleep(1);
        }
	#endif
    return ret;
}
#endif
