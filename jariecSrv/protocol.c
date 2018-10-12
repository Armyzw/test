#include <netinet/in.h>    
#include <sys/types.h>    
#include <sys/stat.h>
#include <sys/socket.h>    
#include <arpa/inet.h>
#include <stdio.h>        
#include <stdlib.h>        
#include <string.h>       
#include <unistd.h>
#include "usrLTE.h"
#include "server.h"
#include "dataBase.h"
#include "zigbee.h"
#define HEELO_WORLD_SERVER_IP	   "115.159.209.106"
#define HELLO_WORLD_SERVER_PORT    5001 
#define BUFFER_SIZE 1024
#define FILE_NAME_MAX_SIZE 512
 
extern unsigned char xg_ZigbeeData[128];
extern int adjust_light_anw_flag;
extern unsigned char sg_ucaRxMsg[256];
void getTimeString(char *strTime)
{
    char buffer[16] = "";
    time_t timep;
    struct tm *p;

    time(&timep);
    p = localtime(&timep);

    sprintf(buffer, "%04d-%02d-%02d %02d:%02d:%02d",
        1900+p->tm_year, 1+p->tm_mon,
        p->tm_mday, p->tm_hour, p->tm_min,
        p->tm_sec);
    if(p)
        strcpy(strTime, buffer);
}
int show_hex(unsigned char *buffer, int length, const char *title)
{
    int i;
    char strTime[32] = "";
    getTimeString(strTime);
    printf("[--%s(%d)%s---------------]\n", title, length, strTime);
    for(i=0; i<length;i++){
        printf("0x%02x ", buffer[i]);
    }
    printf("\n");
}

int XSend(int fd ,unsigned char *data, int size, int netType)
{
    if(netType == 1){	//socket
	return send(fd, data, size, 0);
    }else if(netType == 2){
	return LTEXSend(fd, data, size);
    }
}

int XHandleCmd(struct packData *sd, int sockFd, int netType)
{
	int ret=0;
	struct LightInfo LI;
	struct ControlInfo cl;
	int i, length;
	int value;
	unsigned short wAddr;
    	unsigned char buffer[64] ={'\xfe', '\x21', '\x00', '\x08'};	
	unsigned char bRead[128] = "";
	memset(&cl, 0, sizeof(cl));	
	i = getDevInfo(&cl);
	printf("XHandleCmd, getDevInfo %d\n", i);
	switch(sd->cmd){
		case 0x0:		//report consump ok, need reset power's value
		    printf("report consump ok , reset value of power\n");
			setAllPowerConsump(0);
		     break;
		
		case 0x1:		//report all light's value 
			buffer[1] = 0x11;
			buffer[2] = 0x0;
			buffer[4] = 0x0;//arm status 
			buffer[5] = cl.count; //number of light
			length = 0;
			for(i=0; i<cl.count; i++){
				printf("%x\n", cl.li[i].id);
				memcpy(buffer+6+length, &(cl.li[i].id),2);
				length+=2;
			}
			buffer[3] = 2+length;//sizeof data
#if 0
		        zigbee_temp_current_detection();
			usleep(30*1000);
			if(adjust_light_anw_flag){
				show_hex(xg_ZigbeeData, 4, "ucaRx");
			}		
					
			show_hex(buffer, 6+length, "handle send data");
#endif
			ret = XSend(sockFd, buffer, 6+length, netType);
			show_hex(buffer, 6+length, "report all");
			printf("H1|send %d bytes to report all light value\n", ret);
		break;
		
		case 0x2:
			printf("H0|will set %d, id(%d)->value(%d)\n", 
				sd->data[0], sd->data[1],sd->data[2]);
			buffer[1] = 0x0;
                        buffer[2] = 0x0;
                        buffer[3] = 0x1;
                        buffer[4] = 0x1;
			show_hex(buffer,  5, "handle send data");
			if(netType == 2){
			    int count = sd->data[0];
			    if(count >0 && sd->len > 2*count){
				for(i=0; i<count; i++)
				{
				    if(cl.li[sd->data[1+i*2]-1].value > 100)
					continue;
				    cl.li[sd->data[1+i*2]-1].value = sd->data[2+i*2];
				    value = cl.li[sd->data[1+i*2]-1].value/10;

				    printf("set id(%d) to value(%d)\n", sd->data[1+i*2], value);
				    sscanf(cl.li[sd->data[1+i*2]-1].netAddr,"%hx", &wAddr);
			            printf("netAddr:%s, wAddr:%x\n", 
				         cl.li[sd->data[1+i*2]-1].netAddr, wAddr);

					/*
				    zigbee_temp_modify_dstaddr(wAddr);
				    usleep(30*1000);
				    zigbee_temp_dimming_node(value);
				    */
				    zigbee_set_light_by_dstaddr(wAddr, value);
				    usleep(30*1000);
				    if(zigbee_temp_dimming_node_answ_get()){
					show_hex(xg_ZigbeeData, 1, "ucaRx");
				    }else{
  					printf("cat't connect with %x\n", wAddr);
				    }		
				    
				}		
			    }
			}
			setDevInfo(&cl);
                        //ret = send(sock_fd, buffer, 5, 0);
			ret = XSend(sockFd, buffer, 5, netType);
                        printf("H1|send %d bytes to report set light value ok\n", ret);
			break;
		case 0x3:
			printf("H0|will set all light ->value(%d)\n", 
				sd->data[0]);
			
			buffer[1] = 0x0;
			buffer[2] = 0x0;
			buffer[3] = 0x1;
			buffer[4] = 0x1;
			show_hex(buffer,  5, "handle send data");
			zigbee_temp_set_comm_mode(0x00);
			usleep(30*1000);
			for(i=0; i<cl.count;i++){
			    if(cl.li[i].value <=100){
				cl.li[i].value = sd->data[0];
				value = cl.li[i].value/10;
				sscanf(cl.li[i].netAddr,"%hx", &wAddr);
			        printf("netAddr:%s, wAddr:%x\n", 
				cl.li[i].netAddr, wAddr);
				zigbee_temp_modify_dstaddr(wAddr);
				usleep(30*1000);
				zigbee_temp_dimming_node(value);
				usleep(30*1000);
					
			    }
			//else light control can't conmunication,don't set it
			}
		
			setDevInfo(&cl);
			ret = XSend(sockFd, buffer, 5, netType);
			printf("H1|send %d bytes to report set light value ok\n", ret);
		break;
		case 0x4:
			printf("H0|will report ARM ID to server\n");
			buffer[1] = 0x21;
			buffer[2] = 0x0;
			buffer[3] = 0x08;
    			memcpy(buffer+4, cl.id, 8);
			ret = XSend(sockFd, buffer, 12, netType);
			show_hex(buffer, 12, "ARM ID");
			printf("H1|send %d bytes to report set ARM ID OK\n", ret);
		break;
		case 0x5:
			printf("H0|will report the power consumption\n ");
			buffer[1] = 0x22;
			buffer[2] = 0x0;
			buffer[3] = 0xd;//sizeof data
			buffer[4] = 0xc;
			memcpy(buffer+5, sd->data, 8);
			value = getAllPowerConsump();	
			value = htonl(value);
			memcpy(buffer+13, &value, 4);
#if 0
			value = 0 ;
			for(i=0; i<cl.count;i++){
			    if(cl.li[i].value <= 100){
				sscanf(cl.li[i].netAddr,"%hx", &wAddr);
			        	printf("netAddr:%s, wAddr:%x\n", 
				cl.li[i].netAddr, wAddr);
				zigbee_temp_modify_dstaddr(wAddr);
				usleep(100*1000);
				zigbee_temp_current_power();
				usleep(100*1000);
			    	if(zigbee_temp_dimming_node_answ_get){
				  show_hex(xg_ZigbeeData, 4, "ucaRx");
				  value  +=  xg_ZigbeeData[0]<<24|xg_ZigbeeData[1]<<16|xg_ZigbeeData[2]<<8|xg_ZigbeeData[3];
			        }else{
				  printf("can't read data");
				}
				printf("%x:%d\n", wAddr, value);	
			    }
			}
		
			length = value /1000/1000;
			length = htonl(length);
			memcpy(buffer+13, &length, 4);
#endif
			ret = XSend(sockFd, buffer, 13+4, netType);
			printf("H1|send %d bytes to report power %dwOK(%d)\n", 
				ret,ntohl(length), value);
		break;
		case 0x6:
			buffer[1] = 0x6;
			buffer[2] = 0x0;
			buffer[3] = 0x10;//sizeof data
			memcpy(buffer+4, sd->data, 16);
			ret = XSend(sockFd, buffer, 0x10+4, netType);
			show_hex(buffer, 0x14, "GPS");
			printf("H1|send %d bytes to report GPS\n",ret); 
			
		break;
		case 0x7:
			printf("heart beat\n");
		break;
		case 0x8:
			printf("report GPS info ok.\n");
		break;
		default:
			printf("unkown cmd %d\n", sd->cmd);

	}

	return 0;
}

