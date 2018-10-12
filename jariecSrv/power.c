#include <sys/socket.h> //socket 操作定义
#include <netinet/in.h> //sockaddr_in及其它相关定义
#include <arpa/inet.h>  //inet(3) functions 
#include "usrLTE.h"
#include "server.h"
#include "dataBase.h"
#include "zigbee.h"
#include "power.h"
extern unsigned char xg_ZigbeeData[128];
extern int adjust_light_anw_flag;
extern unsigned char sg_ucaRxMsg[256];




void func_calc_power_thread(int *fd)
{
    printf("calc power start....\n");
    //zigbee_request_access();
    //zigbee_read_local_config();
    //zigbee_release_access();
    //sleep(1);
    if (pthread_setcancelstate (PTHREAD_CANCEL_ENABLE , NULL))
    {
        perror ("Network thread setcancelstate failed");
        pthread_exit(NULL);
    }

	//获取ARM相关配置信息，及zigbee的基础设置
    struct DeviceInfo di;
    memset(&di, 0, sizeof(struct DeviceInfo));
    getArmInfo(&di);

printf("ArmId:%s\nPanId:%x\nChanId:%x\nNetId:%x\n",
		di.armId, di.panId, di.chanId, di.netId);	
#if 0
    if(di.panId>0){
	zigbee_request_access();	//申请zigbee访问权限
    	zigbee_modify_local_panid(sw16(di.panId),			
		 sw16(di.netId), di.chanId);		//设置zigbee的网段，网络地址及通道号
	zigbee_release_access();	//归还zigbee访问权限
    }
#endif

    while(true)
    {
	MainLoop();			//开始统计能耗
    }

    pthread_exit(NULL);
}
#define INTERVAL_POWER	10
int update_all_light_statue(int interval)	//every 3 min-10W
{
	int ret=0;
        struct LightInfo LI;
        struct ControlInfo cl;
        int i, length, count;
	short wAddr;
	int repeat;
	int power;
	char buffer[32] = "";
	count = getDevInfo(&cl);

	//read all lights to calc power
	for(i=0; i<count; i++){
		repeat = 0;
		sscanf(cl.li[i].netAddr,"%hx", &wAddr);
	        
	retry:
		zigbee_request_access();		//申请zigbee访问权限
		zigbee_read_light_by_dstaddr(wAddr);		//单播的方式读取单灯的亮度
		zigbee_release_access();
		if(repeat++ < 2){				//防止数据丢失，最多读两次
		if(zigbee_temp_dimming_node_answ_get()){
			sprintf(buffer, "wAddr:%04x", wAddr);
                        show_hex(xg_ZigbeeData, 4, buffer);
		        switch (xg_ZigbeeData[1]){
			case 0:				//状态正常
	
			    if(xg_ZigbeeData[0]>=0 && xg_ZigbeeData[0]<=10){
				cl.li[i].value = xg_ZigbeeData[0] * 10;
				power = 2*interval*1* xg_ZigbeeData[0];		//按3分钟400W/h计算
				printf("power, [%04x] is %d\n", wAddr, power);
				power += getAllPowerConsump();				//累加
				setAllPowerConsump(power);					//更新到能耗统计文件中，自动生成
			    }else{
				printf("power, error light value\n");		//格式错误
			    }
			break;
			case 1:									//坏了一种，统计一半能耗，上报故障
			   cl.li[i].value = 101;
			   power = interval*1* xg_ZigbeeData[0];
			   power += getAllPowerConsump();
			   setAllPowerConsump(power);
			break;
			case 2:
			   cl.li[i].value = 102;				//两路都坏了
			   
		     }	    
		    }else{
			cl.li[i].value = 104;					//通信故障
			goto retry;
		    }
		}
	}

	setDevInfo(&cl);				//更新所以单灯的状态包括亮度及故障到light.db
	return 0;
}

void MainLoop()
{
	struct packData sd;
	int ret;
	
	memset(&sd, 0, sizeof(sd));
	while(1){
		update_all_light_statue(1);		//和所有单灯进行通信，根据亮度值来计算前一段时间的能耗
#if 1
		sleep(60*3);					//每三分钟统计一次
#else
		sleep(10);						//调试用
#endif     
   }
}

