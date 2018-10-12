#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include "zigbee.h"
#include "server.h"

static int sg_iFd=0;
sem_t semFd;

unsigned char xg_ZigbeeData[128];
static unsigned char sg_ucaRxMsg[256];
static unsigned char sg_uclocal[256];//2017/8/8
static unsigned short sg_wRxMsgLen;
static unsigned char sg_ucHeadA,sg_ucHeadB;
static void MainLoop(void);
void SendMsgWithPackage(unsigned char *pucBuffer, unsigned short wLen);
void SendMsg(unsigned char *pucBuffer, unsigned short wLen);
static void RxMsgParse(unsigned char *pucBuffer, unsigned short wLen);
/*zhangmodify2017/8/8*/  
struct SZigbeeDevInfo zigbeeInfo;
static unsigned short host_Addr;
int temp_modify_addr_flag=0;//2017/8/31��ʱ�޸ĵ�ַ��־,��ʼ��Ϊ0
int adjust_light_anw_flag=0;//2017/8/31����ظ���־����ʼ��Ϊ0

char szDevName1[]={0x5a,0x4c,0x47,0x20,0x44,0x65,0x76,0x69,0x63,0x65,0x00,0x00,0x00,0x00,0x00,0x00};
char szDevPwd1[]={0x38,0x38,0x38,0x38,0x38,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

void zigbee_temp_modify_dstaddr_answ();//2017/8/31��ʱ�޸ĵ�ַָ��ظ�
void zigbee_temp_dimming_node_answ(unsigned char  flag);//2017/8/31�����޸�ָ��ظ�

/*zhangmodify2017/8/8*/  

void func_zigbee_thread(int *iFd)
{

    int i;	
    sem_init(&semFd, 0, 1);   
    printf("zigbee thread start.....\n");
    //�����߳�Ϊ��ȡ��
    if (pthread_setcancelstate (PTHREAD_CANCEL_ENABLE , NULL))
    {
        perror ("Zigbee thread setcancelstate failed");
        pthread_exit(NULL);
    }
    
    if (!iFd)
        pthread_exit(NULL);
    sg_iFd=*iFd;
    if (sg_iFd<=0)
        pthread_exit(NULL);
    /*zhangmodify2017/8/8*/

    memset(&zigbeeInfo, 0, sizeof(zigbeeInfo));

	#if 0
	//zigbee_temp_modify_dstaddr(0x61);
	//usleep(10000);
	//zigbee_temp_dimming_node(0x00);
	//usleep(10000);
	zigbee_read_local_config();
	usleep(30000);
	#endif
	
	#if 0
  	int j;
	for(j=0;j<16;j++)
	{
		zigbeeInfo.szDevName[j]=szDevName1[j];
		zigbeeInfo.szDevPwd[j]=szDevPwd1[j];
	}
  	zigbeeInfo.ucDevMode=0x00;
	zigbeeInfo.ucChan=0x1A;
	zigbeeInfo.wPanID=0x1012;
	zigbeeInfo.wMyAddr=0x2002;
	int m;
	for(m=0;m<8;m++)
	{
		zigbeeInfo.szMyIEEE[m]=0x00;
		zigbeeInfo.szDstIEEE[m]=0x00;
	}
	zigbeeInfo.wDstAddr=0x01;
	zigbeeInfo.ucReserve=0x00;
	zigbeeInfo.ucPowerLevel=0x03;
	zigbeeInfo.ucRetryNum=0x05;
	zigbeeInfo.ucTranTimeOut=0x0A;
	zigbeeInfo.ucSerialRate=0x07;
	zigbeeInfo.ucSerialDataB=0x08;
	zigbeeInfo.ucSerialStopB=0x01;
	zigbeeInfo.ucSerialParityB=0x00;
	zigbeeInfo.ucSendMode=0x00;
	zigbeeInfo.ucRunMode=0xAA;
	zigbeeInfo.wDeviceType=0x00;
	zigbeeInfo.wfirmware=0x01;

	host_Addr=0x2001;
	zigbee_modify_remote_config(host_Addr,&zigbeeInfo);
	sleep(1);
	zigbee_reset_node(0x2002,0x0001);
	sleep(1);
	#endif
     /*zhangmodify2017/8/8*/

    while(true)    
  {
  	
  	 MainLoop();
  	 	  
  } 

    pthread_exit(NULL);
}

static void MainLoop(void)
{
    unsigned char pucBuffer[512]= "";
    short i, nLen;
    struct timeval tv;
    fd_set sRset;

    tv.tv_sec=0;
    tv.tv_usec=50*1000;

    FD_ZERO(&sRset);
    FD_SET(sg_iFd,&sRset);
    if (select(sg_iFd+1, &sRset , NULL , NULL , &tv) >0)
    {
	usleep(10*1000);
    	nLen=read(sg_iFd, pucBuffer, 512);

    	if (nLen>0){
       

    		DBGMSG("Zigbee recv %d data: ", nLen);
    		for(i=0;i<nLen;i++)
        		DBGMSG("%02X ", pucBuffer[i]);
    		DBGMSG("\n");

    		RxMsgParse(pucBuffer, nLen);
    	}
    }
}

int parseRequest(unsigned char *buffer,int lenght, loraInfo *li)
{
    int po=0;
    int i;
    int flag = 0;

    for(i=0;i<lenght;i++)
    {
        //if format is right
        if(buffer[i] == 0x7e){

            if(flag == 0){
                flag = 1;
                li->start       = buffer[++i];
                li->addr[0]     = buffer[++i];
                li->addr[1]     = buffer[++i];
                li->addr[2]     = buffer[++i];
                li->dstAddr[0]  = buffer[++i];
                li->dstAddr[1]  = buffer[++i];
                li->dstAddr[2]  = buffer[++i];
                li->type        = buffer[++i];
                li->cmd         = buffer[++i];
                po = i+1;
            }else{
                printf("there are %d bytes data\n", i-po-1);
                li->size = i-po-1;
                memcpy(li->data, buffer+po, i-po-1);
            }
        }
    }

    return 0;
}


void SendMsgWithPackage(unsigned char *pucBuffer, unsigned short wLen)
{
    unsigned char *pucFrame=malloc(wLen+4);
    unsigned short i, wFrameLen=0;

    assert(pucBuffer);

    //֡ͷ
    pucFrame[wFrameLen++]=0xEA;
    pucFrame[wFrameLen++]=0xEB;
    pucFrame[wFrameLen++]=0xEC;

    //���ܶγ���
    pucFrame[wFrameLen++]=wLen;

    //������
    memcpy(&pucFrame[wFrameLen], pucBuffer, wLen);
    wFrameLen+=wLen;

    write(sg_iFd, pucFrame, wFrameLen);
  
    DBGMSG("Zigbee send %d data: ", wFrameLen);
    for(i=0;i<wFrameLen;i++)
        DBGMSG("%02X ", pucFrame[i]);
    DBGMSG("\n");

    free(pucFrame);
}

void SendMsg(unsigned char *pucBuffer, unsigned short wLen)
{
    unsigned short i;

    assert(pucBuffer);
    
    write(sg_iFd, pucBuffer, wLen);

    DBGMSG("Zigbee send %d data: ", wLen);
    for(i=0;i<wLen;i++)
        DBGMSG("%02X ", pucBuffer[i]);
    DBGMSG("\n");
}
u8 calcSum(u8 *buffer,int lenght)
{
    int dataLen=0;
    int i;
    int flag = 0;
    u8 sum=0;
    if(buffer[0] != 0x7E || buffer[lenght-1] != 0x7E)
        return -1;

    if(buffer[lenght-3]==0x7D && buffer[lenght-2]==0x5E)
        dataLen = lenght-2-2;

    else if(buffer[lenght-3]==0x7D && buffer[lenght-2]==0x5D)
        dataLen = lenght-2-2;
    else
        dataLen = lenght-2-1;

    for(i=1;i<=dataLen;i++)
    {
        //if format is right

        sum ^= buffer[i];
    }
    return sum;
}

unsigned char AccountSum(unsigned char *pucBuffer, unsigned short wLen)
{
    unsigned short i;
    unsigned char ucSum=0;

    assert(pucBuffer);
    
    for(i=0;i<wLen;i++)
        ucSum+=pucBuffer[i];
    
    return ucSum;
}

void zigbee_temp_modify_chan(unsigned char ucChan)
{
    unsigned char *pucBuffer=malloc(16);
    unsigned short wLen=0;

    //������
    pucBuffer[wLen++]=0xDE;
    pucBuffer[wLen++]=0xDF;
    pucBuffer[wLen++]=0xEF;

    //������
    pucBuffer[wLen++]=0xD1;

    //ͨ��
    pucBuffer[wLen++]=ucChan;

    DBGMSG("��ʱ�޸�ͨ����Ϊ %02x\n", ucChan);

    //SendMsgWithPackage(pucBuffer, wLen);
    SendMsg(pucBuffer, wLen);//edit by alert,it's zigbee's protocal not jariec's

    free(pucBuffer);
}


void zigbee_temp_modify_dstaddr(unsigned short wAddr)
{
    unsigned char *pucBuffer=malloc(16);
    unsigned short wLen=0;

    //������
    pucBuffer[wLen++]=0xDE;
    pucBuffer[wLen++]=0xDF;
    pucBuffer[wLen++]=0xEF;

    //������
    pucBuffer[wLen++]=0xD2;
    
    //��ַ
    pucBuffer[wLen++]=wAddr>>8;
    pucBuffer[wLen++]=wAddr&0xff;

    DBGMSG("��ʱ�޸�Ŀ�������ַ %04x\n", wAddr);

  //  SendMsgWithPackage(pucBuffer, wLen);
    SendMsg(pucBuffer, wLen);
}

void zigbee_temp_modify_visible(bool bVisible)
{
    unsigned char *pucBuffer=malloc(16);
    unsigned short wLen=0;

    //������
    pucBuffer[wLen++]=0xDE;
    pucBuffer[wLen++]=0xDF;
    pucBuffer[wLen++]=0xEF;

    //������
    pucBuffer[wLen++]=0xD3;

    //�Ƿ���ʾ
    pucBuffer[wLen++]=bVisible;

    DBGMSG("��ʱ�޸İ�ͷ��ʾԴ��ַ %02x\n", bVisible);

    //SendMsgWithPackage(pucBuffer, wLen);
	SendMsg(pucBuffer, wLen);
    free(pucBuffer);
}

void zigbee_temp_set_comm_mode(unsigned char ucMode)
{
    unsigned char *pucBuffer=malloc(16);
    unsigned short wLen=0;

    //������
    pucBuffer[wLen++]=0xDE;
    pucBuffer[wLen++]=0xDF;
    pucBuffer[wLen++]=0xEF;

    //������
    pucBuffer[wLen++]=0xD9;

    //ģʽ
    pucBuffer[wLen++]=ucMode;

 //   DBGMSG("��ʱ����ͨѶģʽ %02x\n", ucMode);

   // SendMsgWithPackage(pucBuffer, wLen);
    //sem_wait(&semFd);
    SendMsg(pucBuffer, wLen);
    //sem_post(&semFd);
    free(pucBuffer);
}
void zigbee_request_access()
{
    //printf(">>>>>>>>>>>>>>>>>>>>>>\n");
    sem_wait(&semFd);
    //printf("----------------------\n");
}


void zigbee_release_access()
{
    sem_post(&semFd);
    //printf("<<<<<<<<<<<<<<<<<<<<<<<<<\n");
}
void zigbee_query_signal_strength(unsigned short wAddr)
{
    unsigned char *pucBuffer=malloc(16);
    unsigned short wLen=0;

    //������
    pucBuffer[wLen++]=0xDE;
    pucBuffer[wLen++]=0xDF;
    pucBuffer[wLen++]=0xEF;

    //������
    pucBuffer[wLen++]=0xDA;

    //��ַ
    pucBuffer[wLen++]=wAddr>>8;
    pucBuffer[wLen++]=wAddr&0xff;

    DBGMSG("��ѯ�ڵ��ź�ǿ�� %04x\n", wAddr);

    SendMsg(pucBuffer, wLen);

    free(pucBuffer);
}

void zigbee_modify_local_panid(short panid, short addr, unsigned char chan)
{
    unsigned char *pucBuffer=malloc(100);
    unsigned short wLen=0;
    struct SZigbeeDevInfo *zi = &zigbeeInfo;
	
    if(zi->wMyAddr<0)
    {
	sleep(2);
	if(zi->wMyAddr<0)
	{
		printf("can't read Local Config\n");
    free(pucBuffer);
		return;
	}
    }

   
    if (zi->wPanID == panid && \
    	zi->wMyAddr == addr && \
    	zi->ucChan == chan)
    {
	printf("zigbee, set panid, same config, return\n");
    free(pucBuffer);
	return ;
    }

    unsigned short lAddr = zi->wMyAddr;

     
    zi->wPanID = panid;
    zi->wMyAddr = addr;
    zi->ucChan = chan;
    printf("channel:%x\n", zi->ucChan);
    pucBuffer[wLen++]=0xAB;
    pucBuffer[wLen++]=0xBC;
    pucBuffer[wLen++]=0xCD;

    
    pucBuffer[wLen++]=0xD6;

    memcpy(pucBuffer+4, &(lAddr), 2);
    memcpy(pucBuffer+4+2, zi, 65);
    show_hex((unsigned char *)zi, 65, "zigbeeinfo");
    wLen += 67;

    pucBuffer[wLen]=AccountSum(pucBuffer, wLen);
    wLen++;

    show_hex(pucBuffer, wLen, "config");
    SendMsg(pucBuffer,wLen);
    usleep(1000*1000);
    zigbee_reset_node(sw16(lAddr), 0x0001);
    free(pucBuffer);
}

void zigbee_read_local_config(void)
{
    unsigned char *pucBuffer=malloc(16);
    unsigned short wLen=0;

    //������
    pucBuffer[wLen++]=0xAB;
    pucBuffer[wLen++]=0xBC;
    pucBuffer[wLen++]=0xCD;

     //������
    pucBuffer[wLen++]=0xD1;

    //У��
    pucBuffer[wLen]=AccountSum(pucBuffer, wLen);
    wLen++;

    DBGMSG("��ȡ��������ָ��\n");
		printf("zigbee_read_local_config sendmeg\n");
    //SendMsgWithPackage(pucBuffer, wLen);
   SendMsg(pucBuffer,wLen);
    free(pucBuffer);
}


void zigbee_always_modify_chan(unsigned char ucChan)
{
    unsigned char *pucBuffer=malloc(16);
    unsigned short wLen=0;

    //������
    pucBuffer[wLen++]=0xAB;
    pucBuffer[wLen++]=0xBC;
    pucBuffer[wLen++]=0xCD;

    //������
    pucBuffer[wLen++]=0xD2;

    //ͨ��
    pucBuffer[wLen++]=ucChan;

    //У��
    pucBuffer[wLen]=AccountSum(pucBuffer, wLen);
    wLen++;

    DBGMSG("�����޸�ͨ����Ϊ %02x\n", ucChan);

    SendMsgWithPackage(pucBuffer, wLen);

    free(pucBuffer);
}

void zigbee_query_online_node(void)
{
    unsigned char *pucBuffer=malloc(16);
    unsigned short wLen=0;

    //������
    pucBuffer[wLen++]=0xAB;
    pucBuffer[wLen++]=0xBC;
    pucBuffer[wLen++]=0xCD;

    //������
    pucBuffer[wLen++]=0xD4;

    //У��
    pucBuffer[wLen]=AccountSum(pucBuffer, wLen);
    wLen++;

    DBGMSG("�������߽ڵ�\n");

    SendMsg(pucBuffer, wLen);

    free(pucBuffer);
}

void zigbee_read_remote_config(unsigned short wAddr)
{
    unsigned char *pucBuffer=malloc(16);
    unsigned short wLen=0;

    //������
    pucBuffer[wLen++]=0xAB;
    pucBuffer[wLen++]=0xBC;
    pucBuffer[wLen++]=0xCD;

    //������
    pucBuffer[wLen++]=0xD5;

    //��ַ
    pucBuffer[wLen++]=wAddr>>8;
    pucBuffer[wLen++]=wAddr&0xff;

    //У��
    pucBuffer[wLen]=AccountSum(pucBuffer, wLen);
    wLen++;

    DBGMSG("��ȡԶ��������Ϣ %04x\n", wAddr);

    SendMsg(pucBuffer, wLen);

    free(pucBuffer);
}

void zigbee_modify_remote_config(unsigned short wAddr, struct SZigbeeDevInfo *psDevInfo)
{
    unsigned char *pucBuffer=malloc(128);
    unsigned short wLen=0;
    unsigned char i;

    //������
    pucBuffer[wLen++]=0xAB;
    pucBuffer[wLen++]=0xBC;
    pucBuffer[wLen++]=0xCD;

    //������
    pucBuffer[wLen++]=0xD6;

    //��ַ
    pucBuffer[wLen++]=wAddr>>8;
    pucBuffer[wLen++]=wAddr&0xff;
    
    //�豸����
    for (i=0;i<16;i++)
        pucBuffer[wLen++]=psDevInfo->szDevName[i];

    //�豸����
    for (i=0;i<16;i++)
        pucBuffer[wLen++]=psDevInfo->szDevPwd[i];
    
    //�豸����
    pucBuffer[wLen++]=psDevInfo->ucDevMode;

    //ͨ����
    pucBuffer[wLen++]=psDevInfo->ucChan;

    //����ID
    pucBuffer[wLen++]=psDevInfo->wPanID>>8;
    pucBuffer[wLen++]=psDevInfo->wPanID&0xff;

    //���������ַ
    pucBuffer[wLen++]=psDevInfo->wMyAddr>>8;
    pucBuffer[wLen++]=psDevInfo->wMyAddr&0xff;

    //���ر��������ַ
    for (i=0;i<8;i++)
        pucBuffer[wLen++]=psDevInfo->szMyIEEE[i];

    //Ŀ�������ַ
    pucBuffer[wLen++]=psDevInfo->wDstAddr>>8;
    pucBuffer[wLen++]=psDevInfo->wDstAddr&0xff;

    //Ŀ�걾�������ַ
    for (i=0;i<8;i++)
        pucBuffer[wLen++]=psDevInfo->szDstIEEE[i];

    //����
    pucBuffer[wLen++]=psDevInfo->ucReserve;

    //���书��
    pucBuffer[wLen++]=psDevInfo->ucPowerLevel;

    //�����������Դ���
    pucBuffer[wLen++]=psDevInfo->ucRetryNum;

    //������������ʱ����
    pucBuffer[wLen++]=psDevInfo->ucTranTimeOut;

    //���ڲ�����
    pucBuffer[wLen++]=psDevInfo->ucSerialRate;

    //��������λ
    pucBuffer[wLen++]=psDevInfo->ucSerialDataB;

    //����ֹͣλ
    pucBuffer[wLen++]=psDevInfo->ucSerialStopB;

    //����У��λ
    pucBuffer[wLen++]=psDevInfo->ucSerialParityB;

    //����ģʽ
    pucBuffer[wLen++]=psDevInfo->ucSendMode;

    //У��
    pucBuffer[wLen]=AccountSum(pucBuffer, wLen);
    wLen++;

    DBGMSG("�޸�������Ϣ %04x\n", wAddr);

  //  SendMsgWithPackage(pucBuffer, wLen);
    SendMsg(pucBuffer, wLen);
    free(pucBuffer);
}

void zigbee_reset_node(unsigned short wAddr, unsigned short wType)
{
    unsigned char *pucBuffer=malloc(16);
    unsigned short wLen=0;

    //������
    pucBuffer[wLen++]=0xAB;
    pucBuffer[wLen++]=0xBC;
    pucBuffer[wLen++]=0xCD;

    //������
    pucBuffer[wLen++]=0xD9;

    //��ַ
    pucBuffer[wLen++]=wAddr>>8;
    pucBuffer[wLen++]=wAddr&0xff;

    //����
    pucBuffer[wLen++]=wType>>8;
    pucBuffer[wLen++]=wType&0xff;

    //У��
    pucBuffer[wLen]=AccountSum(pucBuffer, wLen);
    wLen++;

    DBGMSG("��λ�ڵ� %04x:%04x\n", wAddr, wType);

    SendMsg(pucBuffer, wLen);

    free(pucBuffer);
}

void zigbee_restore_factory_defaults(unsigned short wAddr, unsigned short wType)
{
    unsigned char *pucBuffer=malloc(16);
    unsigned short wLen=0;

    //������
    pucBuffer[wLen++]=0xAB;
    pucBuffer[wLen++]=0xBC;
    pucBuffer[wLen++]=0xCD;

    //������
    pucBuffer[wLen++]=0xDA;

    //��ַ
    pucBuffer[wLen++]=wAddr>>8;
    pucBuffer[wLen++]=wAddr&0xff;

    //����
    pucBuffer[wLen++]=wType>>8;
    pucBuffer[wLen++]=wType&0xff;

    //У��
    pucBuffer[wLen]=AccountSum(pucBuffer, wLen);
    wLen++;

    DBGMSG("�ָ��������� %04x:%04x\n", wAddr, wType);

    SendMsg(pucBuffer, wLen);

    free(pucBuffer);
}

void zigbee_temp_dimming_node(unsigned char ucLevel)
{
    unsigned char *pucBuffer=malloc(16);
    unsigned short wLen=0;

    //������
    pucBuffer[wLen++]=0xDE;
    pucBuffer[wLen++]=0xDF;
    pucBuffer[wLen++]=0xEF;

    //������
    pucBuffer[wLen++]=0xDB;

    //ģʽ
    pucBuffer[wLen++]=ucLevel;

    DBGMSG("���ýڵ����ȼ���Ϊ %02x\n", ucLevel);

    //sem_wait(&semFd);
    SendMsgWithPackage(pucBuffer, wLen);
    //sem_post(&semFd);
    free(pucBuffer);
}
void zigbee_temp_current_power(void)
{
	
	unsigned char *pucBuffer=malloc(16);
	unsigned short wLen=0;
	
	//������
	pucBuffer[wLen++]=0xDE;
	pucBuffer[wLen++]=0xDF;
	pucBuffer[wLen++]=0xEF;

	//������
	pucBuffer[wLen++]=0xFE;

	//DBGMSG("������� \n");

      //sem_wait(&semFd);
      SendMsgWithPackage(pucBuffer, wLen);
      //sem_post(&semFd);

      free(pucBuffer);
}
/*zhangmodify2017/08/17*/
void zigbee_temp_current_detection(void)
{
	
	unsigned char *pucBuffer=malloc(16);
	unsigned short wLen=0;
	
	//������
	pucBuffer[wLen++]=0xDE;
	pucBuffer[wLen++]=0xDF;
	pucBuffer[wLen++]=0xEF;

	//������
	pucBuffer[wLen++]=0xFB;

	//DBGMSG("������� \n");

      //sem_wait(&semFd);
      SendMsgWithPackage(pucBuffer, wLen);
      //sem_post(&semFd);

      free(pucBuffer);
}
/*zhangmodify2017/08/17*/

//add new function to read light value when setting dst addr

void zigbee_read_light_by_dstaddr(short wAddr)
{

      //sem_wait(&semFd);
	  char command[64]={0x7E,0xAA,0x00,0x00,0x01,0x00,0x00,0x01,0x81,0x81,0xAA,0x7E};

	  write(sg_iFd, command, 12);
      usleep(100*1000);
      //sem_post(&semFd);
}
void zigbee_set_light_by_dstaddr(short wAddr, unsigned char level)
{
      //sem_wait(&semFd);
      char command[64]={0x7E,0xAA,0x00,0x00,0x01,0x00,0x00,0x01,0x82,0x84,0x64,0xAA,0x7E};

	  command[6] = wAddr>>4;
	  command[7] = wAddr&0xF;
	  command[10] = level*10;//whether need to *10]
	  command[11] = calcSum(command, 13);
	  write(sg_iFd, command, 13);

      usleep(100*1000);
      //sem_post(&semFd);
}

static void RxMsgParse(unsigned char *pucBuffer, unsigned short wLen)
{
    unsigned short i;
	loraInfo li;

	memset(&li, 0, sizeof(loraInfo));
	parseRequest(pucBuffer,wLen,&li);

	switch(li.cmd){
		case 0x81:	//query light
			memcpy(xg_ZigbeeData, li.data, li.size);
			zigbee_temp_dimming_node_answ(1);
			break;
		case 0x84:
			zigbee_temp_dimming_node_answ(1);
			break;
	}
}


void zigbee_temp_modify_dstaddr_answ(void)
{
	temp_modify_addr_flag=1;//��ʱ�޸ĵ�ַ�յ��ظ�������Ϊ1
}

void zigbee_temp_dimming_node_answ(unsigned char flag)
{
	adjust_light_anw_flag=1;//�����յ��ظ�������1

	//printf("adjust_light_anw_flag from %02x\n", flag);
}

int zigbee_temp_dimming_node_answ_get()
{
	int ret = adjust_light_anw_flag;
	adjust_light_anw_flag=0;
	return ret;
	
}

