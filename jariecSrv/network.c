#include <sys/socket.h> //socket 操作定义
#include <netinet/in.h> //sockaddr_in及其它相关定义
#include <arpa/inet.h>  //inet(3) functions 
#include "network.h"
#include "usrLTE.h"
#include "server.h"
#include "gps.h"
static int sg_iFd=0;

static void MainLoop(int fd);
static bool InitServer(void);
extern nmea_msg gGpsInfo;
nmea_msg gpsInfo;
void func_network_thread(int *fd)
{
    printf("network for  LTE start....\n");
    //设置线程为可取消
    if (pthread_setcancelstate (PTHREAD_CANCEL_ENABLE , NULL))
    {
        perror ("Network thread setcancelstate failed");
        pthread_exit(NULL);
    }
	//通过4G usr模块进行网络连接 
    sg_iFd=LTEXConnect(*fd, HEELO_WORLD_SERVER_IP, HELLO_WORLD_SERVER_PORT);
    printf("LTEXConnect on %d(%d)\n", sg_iFd, *fd);
    memset(&gpsInfo, 0, sizeof(gpsInfo));
    while(true)
        MainLoop( *fd);

    pthread_exit(NULL);
}
int isNeedResetLTE(unsigned char *buffer, int size, int fd)
{
    int ret = 0;
    if(size>20){
	if(strstr(buffer, "All Communication time")||\
             strstr(buffer, "will be reset")){
		ret = 1;
                write(fd, "usr.cnAT+Z\r\n", 12);
                printf("++++reset LTE module\n");
        }
    }
    return ret;
}
int isProtocol(unsigned char *buffer, int size, int fd)
{
	int ret = 1;
	if(size>6+4)
	{
		if(buffer[6] == 0xFE)
			ret = 1;
		else
			ret = 0;	
	}
	if(ret == 0){
	    isNeedResetLTE(buffer, size, fd);	
	}

	return ret;
}

int isLTENotice(unsigned char *data, int size)
{
    int ret = 0;
    if((data[size-2] == 0x0d) && (data[size-1] == 0x0a))
    {
	ret = 1;
    }
    return ret;
}


int recvXData(int fd, struct packData *sd)
{
   int length = 0, flag = 0, len_data=0, size_data;
   int i;
   int ret=-1, offset=strlen(RECV_SOCKA);
   int retry = 3;

   unsigned char header[128] = "";

   length = LTEXRecv(fd, header, sizeof(header));   //接收网络数据
   if(length <= 0)
	return -1;
    //printf("\n------recvX, read %d bytes from uart\n", length);
   show_hex(header, length, "recvX data from uart");
   while(retry--){
	if(isLTENotice(header, length) == 0){			//判断是网络数据还是模块提示信息
   	    if((length<(6+4))){
			//数据不完整处理
	        size_data = read(fd, header+length, sizeof(header)-length);
	        length += size_data;
                show_hex(header, size_data, "++++recv data head$$$$$");
	        if(isProtocol(header, length, fd)==0){
		    printf("$$$$$not protocol follow jariec\n");	
	            return -1;
	        }
            }else if(length <(6+4+header[6+3])){
	        size_data = read(fd, header+length, sizeof(header)-length);
	        length += size_data;
                show_hex(header, size_data, "++++recv data head#####");
	        if(isProtocol(header, length, fd)==0){
		    printf("####not protocol follow jariec");	
	            return -1;
	        }
	    }else
		break;
        }else{
        //判断是否要重启模块进行网络重联
	    //message from LTE MODULE, like socket disconnect/LTE ready etc...
	    printf("----notice message");
	    isNeedResetLTE(header, length, fd);
            break;
	}
	usleep(30*1000);
   }
#if 0
   if(length>50){
	if(strstr(header, "All Communication timeout")){
		write(fd, "usr.cnAT+Z\r\n", 12);
		printf("++++reset LTE module\n");
		return -1;
	}
   }
#endif
   if(length>0 && strstr(header, RECV_SOCKA)){
   	    //网络通信数据完整，进行协议分析
        offset += strstr(header, RECV_SOCKA)-(char *)header;
        sd->start = header[offset+0];
        sd->cmd   = header[offset+1];
        sd->lenH  = header[offset+2];
        sd->lenL  = header[offset+3];
        sd->len = sd->lenH<<8 | sd->lenL;

        //printf("R1|cmd=0x%x, length=%d\n",sd->cmd, sd->len);

        if(sd->len>0){ 
            sd->data = (unsigned char *)malloc(sd->len);
            memset(sd->data, '\0', sd->len);
            for(i=0; i<sd->len;i++){
                if(offset+SOCK_HEAD_SIZE+i<=length){
                    sd->data[i] = header[offset+SOCK_HEAD_SIZE+i];
                }
                else{
                    printf("recvX, data no entiried\n");
		    ret = -1;
		    return ret;
                    //todo need read data again from uart
                }
            }
            ret = length-offset;
            //printf("R2|recv data(%d):0x%x\n",sd->len, sd->data[0] );
        }else{
                ret = 0;
                printf("R2|needn't read data\n");
        }
     
    }

    return ret;
}



static void MainLoop(int fd)
{
	struct packData sd;
	int ret;
 	int login = 0;	
	memset(&sd, 0, sizeof(sd));
	struct timeval last, now;
	struct timezone tz;
	last.tv_sec = 0;
	 while(1){
                if(sd.len>0){
                        free(sd.data);
                }
                memset(&sd, '\0', sizeof(sd));
                ret = recvXData(fd ,&sd);
                if(ret>=0){
                    printf("M2|cmd=%X, len=%d\n",sd.cmd, sd.len);
		    zigbee_request_access();		//申请zigbee访问权限
                    XHandleCmd(&sd, fd, 2);   //处理网络协议
		    zigbee_release_access();
		    if(sd.cmd == 0x4)		//标识为初次登陆
			login = 1;
		    printf("------------------------------------------------\n\n\n");
                }else{
		    //printf("cant'd read from %d\n", fd);
		    usleep(100*1000);
		}

		gettimeofday(&now, &tz);
		//for GPS, if location changed, report to platform
		double latitude, longitude;
		if(gGpsInfo.fixmode>1){	//GPS ready
		    if((gGpsInfo.latitude != gpsInfo.latitude) \
			||(gGpsInfo.longitude != gpsInfo.longitude)\
			|| (login == 1)||(now.tv_sec-last.tv_sec>5*60))
		    {
			sleep(1);
			login = 0;
			latitude = (double)(gGpsInfo.latitude)/100000;
			longitude = (double)(gGpsInfo.longitude)/100000;
			sd.cmd = 0x6;
			sd.len = 0x10;
			sd.data = (unsigned char *)malloc(0x10);
			memcpy(sd.data, &latitude, 0x8);
			memcpy(sd.data+8, &longitude, 0x8);
			XHandleCmd(&sd, fd, 2);
			memcpy(&gpsInfo, &gGpsInfo, sizeof(gpsInfo));
			printf("GPS, %02ld, latitude:%f, longitude:%f\n",
				now.tv_sec%60, latitude, longitude);
			last.tv_sec = now.tv_sec;
		    }
		}else{
		    if((login == 1)||(now.tv_sec-last.tv_sec>5*60))	
		    {
			sleep(1);
			login = 0;
			latitude =0.0;// (double)(gGpsInfo.latitude)/100000;
			longitude = 0.0;//(double)(gGpsInfo.longitude)/100000;
			sd.cmd = 0x6;
			sd.len = 0x10;
			sd.data = (unsigned char *)malloc(0x10);
			memcpy(sd.data, &latitude, 0x8);
			memcpy(sd.data+8, &longitude, 0x8);
			XHandleCmd(&sd, fd, 2);
			memcpy(&gpsInfo, &gGpsInfo, sizeof(gpsInfo));
			printf("GPS, %02ld, latitude:%f, longitude:%f\n",
				now.tv_sec%60, latitude, longitude);
			last.tv_sec = now.tv_sec;
			
		    }
		}
		
	
        }

}
