#include <sys/socket.h> //socket ��������
#include <netinet/in.h> //sockaddr_in��������ض���
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

	//��ȡARM���������Ϣ����zigbee�Ļ�������
    struct DeviceInfo di;
    memset(&di, 0, sizeof(struct DeviceInfo));
    getArmInfo(&di);

printf("ArmId:%s\nPanId:%x\nChanId:%x\nNetId:%x\n",
		di.armId, di.panId, di.chanId, di.netId);	
#if 0
    if(di.panId>0){
	zigbee_request_access();	//����zigbee����Ȩ��
    	zigbee_modify_local_panid(sw16(di.panId),			
		 sw16(di.netId), di.chanId);		//����zigbee�����Σ������ַ��ͨ����
	zigbee_release_access();	//�黹zigbee����Ȩ��
    }
#endif

    while(true)
    {
	MainLoop();			//��ʼͳ���ܺ�
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
		zigbee_request_access();		//����zigbee����Ȩ��
		zigbee_read_light_by_dstaddr(wAddr);		//�����ķ�ʽ��ȡ���Ƶ�����
		zigbee_release_access();
		if(repeat++ < 2){				//��ֹ���ݶ�ʧ����������
		if(zigbee_temp_dimming_node_answ_get()){
			sprintf(buffer, "wAddr:%04x", wAddr);
                        show_hex(xg_ZigbeeData, 4, buffer);
		        switch (xg_ZigbeeData[1]){
			case 0:				//״̬����
	
			    if(xg_ZigbeeData[0]>=0 && xg_ZigbeeData[0]<=10){
				cl.li[i].value = xg_ZigbeeData[0] * 10;
				power = 2*interval*1* xg_ZigbeeData[0];		//��3����400W/h����
				printf("power, [%04x] is %d\n", wAddr, power);
				power += getAllPowerConsump();				//�ۼ�
				setAllPowerConsump(power);					//���µ��ܺ�ͳ���ļ��У��Զ�����
			    }else{
				printf("power, error light value\n");		//��ʽ����
			    }
			break;
			case 1:									//����һ�֣�ͳ��һ���ܺģ��ϱ�����
			   cl.li[i].value = 101;
			   power = interval*1* xg_ZigbeeData[0];
			   power += getAllPowerConsump();
			   setAllPowerConsump(power);
			break;
			case 2:
			   cl.li[i].value = 102;				//��·������
			   
		     }	    
		    }else{
			cl.li[i].value = 104;					//ͨ�Ź���
			goto retry;
		    }
		}
	}

	setDevInfo(&cl);				//�������Ե��Ƶ�״̬�������ȼ����ϵ�light.db
	return 0;
}

void MainLoop()
{
	struct packData sd;
	int ret;
	
	memset(&sd, 0, sizeof(sd));
	while(1){
		update_all_light_statue(1);		//�����е��ƽ���ͨ�ţ���������ֵ������ǰһ��ʱ����ܺ�
#if 1
		sleep(60*3);					//ÿ������ͳ��һ��
#else
		sleep(10);						//������
#endif     
   }
}

