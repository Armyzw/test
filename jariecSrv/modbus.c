#include "modbus.h"
#include "zigbee.h"
#include "dataBase.h"
unsigned char g_ucAnQiaoAddr=0; 

int g_iLocation=-1;

static int sg_iFd=0;
/*zhangModigy2017/8/10*/
//int light_pos[16][2]={{0,0x0A},{400,0x0A},{1200,0x0A},{2100,0x0A},{3000,0x0A},{3900,0x0A},{4500,0x0A},{5400,0x0A},{6300,0x0A},{7200,0x0A},
				//{8200,0x0A},{9100,0x0A},{0,0x0A},{0,0x0A},{0,0x0A},{0,0x0A}};//8��
//int light_pos[15][2]={{0,0x0A},{0,0x0A},{1900,0x0A},{2900,0x0A},{2900,0x0A},{3900,0x0A},{4800,0x0A},{5400,0x0A},{6400,0x0A},{7400,0x0A},
				//{8300,0x0A},{9100,0x0A},{0,0x0A},{0,0x0A},{0,0x0A}};//11��


int light_pos[32][2]={{0,0x0A},{400,0x0A},{400,0x0A},{5100,0x0A},{6000,0x0A},{6700,0x0A},{7300,0x0A},{8400,0x0A},
				{9200,0x0A},{9200,0x0A},{0,0x0A},{0,0x0A},{0,0x0A},{0,0x0A}};//��һ��Ϊ�ƾ߰�װλ�ã��ڶ���Ϊ�ƾ߹���*/2017/11/7֮ǰΪ6300


int zone_section[32][2]={{0,400},{400,5100},{5100,6000},{6000,6700},{6700,7300},{7300,8400},{8400,9200},{0,0}};//��һ�����������ޣ��ڶ�������������

//int light_num=12;
int light_num=10;//20171124

//int zone_section_num=9;//2017/8/25,ԭ��Ϊ11
//int zone_section_num=11;//2017/11/7
int zone_section_num=7;//2017/11/24

struct SZigbeeDevInfo zigbeeInfo;
static unsigned short host_Addr;
int zone_previous=-1;
int posTimerStart=0;
int pos_previous=0;
int move_flag=0;
int object_back_sflag=0;//Ϊ���ж�ͣ������10���Ӻ�С��������ʼ���ˣ�����Ǻ�������1������Ϊ��
int elec_on_fwa_flag=0;//ARM�ϵ磬��С��ֱ�Ӻ���
extern int posCalcnum;	
int max_zone_now=0;//2017/8/30Ϊ��modbusû�������ж�
//int max_zone_previous=8;//ԭ���������ID��Ϊ10��2017/8/25�������ID��Ϊ9
//int max_zone_previous=10;//2017/11/7
int max_zone_previous=6;//2017/11/24

extern int no_data_count;//modbus û�����ݵļ���
int no_data_modbusStart=0;

extern int temp_modify_addr_flag;//2017/8/31��ʱ�޸ĵ�ַ��־
extern int adjust_light_anw_flag;//2017/8/31����ظ���־

//unsigned short light_addr[]={0x4C,0x71,0x70,0x4E,0x4D,0x50,0x5C,0x46,0x5E,0x49,0x52,0x68,0x33,0x6E,0x44,0x5B};
unsigned short light_addr[32]={0x5A,0x3C,0x3D,0x56,0x3E,0x3B,0x32,0x6F,0x5D,0x67,0x30,0x57,0x34,0x53, 0x0};



/*zhangModigy2017/8/10*/

//crc ��λ�ֽ�ֵ��
static const unsigned char sg_ucaCrcHi[256] = { 
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 
    0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 
    0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 
    0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 
    0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 
    0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 
    0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40 
} ; 
// crc��λ�ֽ�ֵ��
static const unsigned char sg_ucaCrcLow[256] = { 
    0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 
    0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD, 
    0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09, 
    0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A, 
    0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 0xD4, 
    0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3, 
    0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 
    0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4, 
    0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A, 
    0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29, 
    0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED, 
    0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26, 
    0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 
    0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67, 
    0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F, 
    0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68, 
    0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E, 
    0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5, 
    0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 
    0x70, 0xB0, 0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92, 
    0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C, 
    0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B, 
    0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B, 
    0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C, 
    0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 
    0x43, 0x83, 0x41, 0x81, 0x80, 0x40 
} ;

static void MainLoop(void);
static void ReadAnQiao(unsigned char ucLink);
static unsigned short Crc16(unsigned char *pucBuffer, unsigned short wLen); 
static int GetData(unsigned char *pucBuffer, unsigned short wLen);
static int Small_bus_logic(int zone_now,int pos_now);//С�������߼�
static int All_adjust_light();//ȫ������
static int get_veh_zone(int velpos);

extern void SendMsgWithPackage(unsigned char *pucBuffer, unsigned short wLen);
extern void zigbee_temp_modify_dstaddr(unsigned short wAddr);

extern void zigbee_temp_dimming_node(unsigned char ucLevel);//��ʱ����
extern void zigbee_temp_set_comm_mode(unsigned char ucMode);//��ʱ�޸�ͨѶ��ʽ
extern void zigbee_temp_current_detection(void);//�������

void func_modbus_thread(int *iFd)
{
    int i;
    //�����߳�Ϊ��ȡ��
    if (pthread_setcancelstate (PTHREAD_CANCEL_ENABLE , NULL))
    {
        perror ("Modbus thread setcancelstate failed");
        pthread_exit(NULL);
    }

    if (!iFd)
        pthread_exit(NULL);
    sg_iFd=*iFd;
    if (sg_iFd<=0)
        pthread_exit(NULL);

    //init light  and anqiao

    struct ControlInfo cl;
    struct QnqiaoZoonInfo qz;
    short wAddr;
    getDevInfo(&cl);		//��lignt.db �������Ƶ�ID,λ�õ���Ϣ
    if(cl.count>0){
	memset(light_addr, 0, sizeof(light_addr));
	memset(light_pos, 0, sizeof(light_pos));
	light_num = cl.count;
	for(i=0;i<cl.count;i++){
	    sscanf(cl.li[i].netAddr,"%hx", &wAddr);
	    light_addr[i] = wAddr;
	    light_pos[i][0] = cl.li[i].status;
	}
    }

    
    getQnqiaoInfo(&qz);		//anqiao.db ������������������Ϣ
    if(qz.count > 0){
	memset(zone_section, 0, sizeof(zone_section));
  	zone_section_num = qz.count;
	for(i=0; i<qz.count; i++){
	    zone_section[i][0] = qz.az[i].start;
	    zone_section[i][1] = qz.az[i].end;
	}
    }else{
	printf("modbus, can't read info from database\n");
    }   
 
#if 1
	printf("modbus, (%d)light addr is:\n", light_num);
	for(i=0;i<light_num;i++)
	    printf("%x, ", light_addr[i]);
	printf("\n");

	printf("modbus, light postion is:\n");
	for(i=0;i<light_num;i++)
	    printf("%d(%x), ", light_pos[i][0], light_pos[i][1]);
	printf("\n");

 	
	printf("modbus, (%d)zooe section is:\n", zone_section_num);
	for(i=0;i<zone_section_num;i++)
	    printf("%d-%d, ", zone_section[i][0], zone_section[i][1]);
	printf("\n");
	
	
#endif
   
    


    while(true)
        MainLoop();

    pthread_exit(NULL);
}

static void MainLoop(void)
{
    sleep(1);//ÿ��һ��ȥ��С������
    unsigned char *pucBuffer=NULL;
    short nLen;
    short i;
    g_ucAnQiaoAddr=0x01;
    ReadAnQiao(g_ucAnQiaoAddr);

    pucBuffer=malloc(32);
    nLen=0;
 
	
    while(true)
    {
    	
        struct timeval tv;
        fd_set sRset;
    
        tv.tv_sec=0;
        tv.tv_usec=5*1000;
        
        FD_ZERO(&sRset);
        FD_SET(sg_iFd,&sRset);
        no_data_modbusStart=1;
        if (select(sg_iFd+1, &sRset , NULL , NULL , &tv) >0)
        {
	  
           unsigned char data;
	    no_data_modbusStart=1;
           if (read(sg_iFd, &data, 1)<=0)//2017/8/29 
           {
			no_data_modbusStart=1;
			break;
            		
            }    
            else
            {
            		no_data_modbusStart=0;
			no_data_count=0;
            		pucBuffer[nLen++]=data;
			//show_hex(pucBuffer, nLen, "readModbus");
            }  
		
            if (nLen>=32)
            {
                DBGMSG("read data too leng\n");
             	show_hex(pucBuffer, nLen, "modbus");
	        nLen=0;
                break;
            }
        }
        else 
        {
        	break;
    	
        }
            
    	}
    
    if (nLen>0)
    {
        int res;


        for(i=0;i<nLen;i++)
            printf("%02X ", pucBuffer[i]);
        printf("\n");

        res=GetData(pucBuffer, nLen);
	DBGMSG("result of Getdata %x\n", res);
        if (res>0)
            g_iLocation=res;//�õ���λ��

	 //printf("g_iLocation=%d\n",g_iLocation);
	 if((g_iLocation>=-10)&&(g_iLocation<=9300))//2017/9/11�޸ģ�ֻ���ƺ�����յ��
	 {
	 	int pos_now=g_iLocation;
		int zone_now1=get_veh_zone(g_iLocation);
	        zigbee_request_access();	
		Small_bus_logic(zone_now1,pos_now);
		zigbee_release_access();
	 }
	 
    }

    
    free(pucBuffer);
}

static void ReadAnQiao(unsigned char ucLink)
{
    unsigned char *pucBuffer=NULL;
    unsigned short wCrcData=0;
    unsigned short i;

    pucBuffer=malloc(32);

    pucBuffer[0] = ucLink;//��ַ��
    pucBuffer[1] = 0x03;//������
    pucBuffer[2] = 0x0B;//�Ĵ�����ַ2���ֽ�
    pucBuffer[3] = 0xB9;
    pucBuffer[4] = 0;//�Ĵ�������2���ֽ�
    pucBuffer[5] = 1;
    
    wCrcData = Crc16(pucBuffer,6); 
    pucBuffer[6] = wCrcData >> 8;
    pucBuffer[7] = wCrcData & 0xff;

    if (sg_iFd>0)
        write(sg_iFd,pucBuffer,8);

    DBGMSG("sg_iFd=%d",sg_iFd);
    DBGMSG("\nModbus send 8 data:");
    for(i=0;i<8;i++)
        DBGMSG("%02x ",pucBuffer[i]);
    DBGMSG("\n");

    free(pucBuffer);
}

//##### crc16���� #####//
static unsigned short Crc16(unsigned char *pucBuffer, unsigned short wLen) 
{ 
    unsigned char ucCrcHi=0xFF;     
    unsigned char ucCrcLow= 0xFF;      
    unsigned char ucIndexNo;

    while ((wLen--)>0) 
    { 
        ucIndexNo = ucCrcHi ^ (*pucBuffer++) ;  
        ucCrcHi = ucCrcLow ^ sg_ucaCrcHi[ucIndexNo] ; 
        ucCrcLow = sg_ucaCrcLow[ucIndexNo] ; 
    } 
    return ((ucCrcHi<<8) | ucCrcLow) ; 
}


static int GetData(unsigned char *pucBuffer, unsigned short wLen)
{
    unsigned short wCrcData=0;

    wCrcData =Crc16(pucBuffer, wLen-2); 

    if (wCrcData != ((pucBuffer[wLen-2]<<8) + pucBuffer[wLen-1]))
    {
        DBGMSG("crc erro: %04x=%04x\n", pucBuffer[0], pucBuffer[1]);
        return -1;
    }

    if (pucBuffer[0]!=g_ucAnQiaoAddr)
        return -1;
    else
        return ((pucBuffer[4]<<8) | pucBuffer[5]);
        //return ((pucBuffer[3]<<8) | pucBuffer[4]);
}

static int Small_bus_logic(int zone_now,int pos_now)
{
	if(zone_previous==-1)//2017/8/26Ϊ��ʹARM�ϵ磬��С��ֱ�Ӻ���ʱ�򣬵Ʋ��𣬸��ϵ�zone_previous=0
	{
		zone_previous=zone_now;
	}
	if(pos_previous==0)//2017/8/26Ϊ��ʹARM�ϵ磬��С��ֱ�Ӻ���ʱ�򣬵Ʋ�����Ϊ���ϵ�pos_previousΪ0����pos_nowΪ����0�������ǰ���ҿ�����ļ������԰�pos_now��pos_previous
	{
		pos_previous=pos_now;
	}
	printf("before_pos:%d(zone-%d:max%d)-->curent:%d(zone-%d:max%d)\n",
		pos_previous, zone_previous, max_zone_previous,
		pos_now, zone_now, max_zone_now);
	
	DBGMSG("LINE=%d,pos_now=%d,pos_previous=%d\n",__LINE__,pos_now,pos_previous);
	DBGMSG("LINE=%d,zone_now=%d,zone_previous=%d\n",__LINE__,zone_now,zone_previous);
	DBGMSG("LINE=%d,max_zone_now=%d,max_zone_previous=%d\n",__LINE__,max_zone_now,max_zone_previous);

	
	
	if(pos_now!=pos_previous)
	{
		posTimerStart=0;//20170817
		
		if(move_flag==1)
		{
			zigbee_temp_set_comm_mode(0x01);//ʹzigBeeͨ�ŷ�ʽΪ�㲥��ʽ
			usleep(30000);
			int i;
			for(i=0;i<5;i++)
			{
				zigbee_temp_dimming_node(0x08);
				usleep(10000);
			}
			int m;
			for(m=0;m<16;m++)
			{
				light_pos[m][1]=0x08;
			}
			sleep(1);

			int j;
			for(j=0;j<10;j++)
			{
				zigbee_temp_dimming_node(0x0A);
				usleep(10000);
			}
			int n;
			for(n=0;n<16;n++)
			{
				light_pos[n][1]=0x0A;
			}
			sleep(1);

			
			move_flag=0;
			zigbee_temp_set_comm_mode(0);
			usleep(30000);

			if(pos_now<pos_previous)//���С���������෽����ʻ
			{
				object_back_sflag=1;
			}
			
			/*2017/9/11Ϊ��ʹС���ӵȴ�10����������˶�ʱ����������ʼ��״̬*/
			posCalcnum=0;
			//max_zone_previous=8;
			max_zone_previous=10;//2017/11/8
			elec_on_fwa_flag=0;
			/*2017/9/11Ϊ��ʹС���ӵȴ�10����������˶�ʱ����������ʼ��״̬*/
				
		}

		if(pos_now>=4500)//2017/9/11ֻ���������յ
		{
			if(zone_now>zone_previous)//С��ǰ��
			{
				printf("modbus, LINE=%d,qianjian zone_now=%d,zone_previous=%d\n",__LINE__,zone_now,zone_previous);
				printf("modbus, LINE=%d,qianjin  max_zone_now=%d,max_zone_previous=%d\n",__LINE__,max_zone_now,max_zone_previous);
				elec_on_fwa_flag=1;//2017/8/26 Ϊ��ʹARM�ϵ磬��С��ֱ�Ӻ���ʱ�򣬵Ʋ���ֻ���ϵ����ǰ�ߴ�ֵ��Ϊ1������Ϊ0
				printf("modbus, LINE=%d,qianjin  elec_on_fwa_flag=%d\n",__LINE__,elec_on_fwa_flag);
				if(object_back_sflag==1)//�����ʱ��С���Ǿ�ֹ��10���ӣ���С���������෽����ʻ������������ʻ����������������
				{
					//max_zone_previous=8;//2017/8/25 ԭ����10
					max_zone_previous=10;//2017/11/8
					object_back_sflag=0;
				}
				else
				{
					elec_on_fwa_flag=1;//2017/8/26Ϊ��ʹARM�ϵ磬��С��ֱ�Ӻ���ʱ�򣬵Ʋ���
					move_flag=0;
			
					int i;
					for(i=0;i<light_num;i++)//2017/8/25��ԭ����light_num+1:��1����Ϊ��Ϊ12յ������λ��������˸�����ԭ��
					{
						
						if(light_pos[i][0]==zone_section[zone_now][1])
						{			
							
							int count;
							for(count=0;count<5;count++)//��ʱ�޸ĵ�ַ
							{
								zigbee_temp_modify_dstaddr(light_addr[i]);//2017/8/25ԭ��Ϊi-1
								usleep(80000);//����ʱ��Ϊ5ms
							}
							/*2017/8/31����ʱ�޸ĵ�ַ�ظ������ж�*/
							DBGMSG("modbus, LINE=%d,open temp_modify_addr_flag=%d\n",__LINE__,temp_modify_addr_flag);
							if(temp_modify_addr_flag==0)//��ʱ�޸ĵ�ַ��־���Ϊ0����û���յ��ظ�
							{
								int count;
								for(count=0;count<5;count++)//��ʱ�޸ĵ�ַ
								{
									zigbee_temp_modify_dstaddr(light_addr[i]);//2017/8/25ԭ��Ϊi-1
									usleep(80000);//����ʱ��Ϊ5ms
								}
							}
							else if(temp_modify_addr_flag==1)//�յ��ظ�
							{
								temp_modify_addr_flag=0;
							}
							/*2017/8/31����ʱ�޸ĵ�ַ�ظ������ж�*/
							
							light_pos[i][1]=0x0A;
							int count1;
							printf("modbus, LINE=%d,open light_pos(%d:0x%x)=%d\n",__LINE__,i, light_addr[i],light_pos[i][0]);							
						        for(count1=0;count1<5;count1++)//����
							{
								zigbee_temp_dimming_node(light_pos[i][1]);
								usleep(80000);//����ʱ��Ϊ5ms
							}

							/*2017/8/31�Ե���ظ������ж�*/
							//printf("LINE=%d,open adjust_light_anw_flag=%d\n",__LINE__,adjust_light_anw_flag);
							if(adjust_light_anw_flag==0)
							{
								printf("modbus, LINE=%d,open light_pos[i][0]=%d again\n",__LINE__,light_pos[i][0]);							
								int count1;
								for(count1=0;count1<5;count1++)//����
								{
									zigbee_temp_dimming_node(light_pos[i][1]);
									usleep(80000);//����ʱ��Ϊ5ms
								}
							}
							else if(adjust_light_anw_flag==1)//�յ��ظ�
							{
								adjust_light_anw_flag=0;
							}
							/*2017/8/31�Ե���ظ������ж�*/
										
						}
						
					}
					zone_previous=zone_now;
					max_zone_now=zone_now;
				}
				
				
				zigbee_temp_set_comm_mode(0x01);//ʹzigBeeͨ�ŷ�ʽΪ�㲥��ʽ
				usleep(30000);
				zigbee_temp_current_detection();
				usleep(30000);
				zigbee_temp_set_comm_mode(0x00);
				usleep(30000);
			

			}
			else if(zone_now<zone_previous)//С������,��С������˲������¸��������޵ĵ�
			{
				printf("modbus ,LINE=%d,houtui zone_now=%d,zone_previous=%d\n",__LINE__,zone_now,zone_previous);
				move_flag=0;	
				printf("modbus, LINE=%d, into back\n",__LINE__);
				printf("modbus ,LINE=%d,houtui max_zone_now=%d,max_zone_previous=%d\n",__LINE__,max_zone_now,max_zone_previous);
				if(object_back_sflag==0)//Ϊ���ж����С����ֹͣ10���ӵ�״̬����ʼ������Ϊ�����ŷ����ƶ�����ʱ��Ʋ���Ҳ�������˺��߼���������
				{
					DBGMSG("LINE=%d,pos_previous-pos_now=%d\n",__LINE__,pos_previous-pos_now);
					if(pos_previous-pos_now>30)//2017/9/18�ں����װʱ��Ҳ���𣬼�����λ�ñ仯����20����Ϊ���������ĺ���
					{	
						DBGMSG("LINE=%d,elec_on_fwa_flag=%d\n",pos_previous-pos_now,elec_on_fwa_flag);
						if(elec_on_fwa_flag==1)//2017/8/26Ϊ��ʹARM�ϵ磬��С��ֱ�Ӻ���ʱ�򣬵Ʋ���ֻ����С����ǰ���ٺ��˵����elec_on_fwa_flag=1�����ֱ���ϵ�ͺ��˴�ֵΪ0����ִ�к������
						{
							
							if (max_zone_now<max_zone_previous)////���max_zone_now��max_zone_previous֮��ĵƾ�,���������ں���ʱ����һ��С������ǰһ������ʱ������֮��max_zone_now��max_zone_previous���
							{
								
								int i;
								for(i=0;i<light_num;i++)//2017/8/25��֮ǰΪlight_num+1:��1����Ϊ��Ϊ12յ������λ��������˸�����ԭ�㣬������ʵ�൱��13յ
								{
									if(light_pos[i][0]==zone_section[max_zone_now+1][1])//���ҳ��¸����������ֵ,��ʱzone_now�Ѿ���Ϊǰһ���������
									{
										
										int j;
										for(j=0;j<light_num;j++)//2017/8/25��֮ǰlight_num+1:��1����Ϊ��Ϊ12յ������λ��������˸�����ԭ��
										{
											if(light_pos[j][0]>=light_pos[i][0])//�Ӳ��ҳ����¸����������ֵ�������һյ��ȫ���ص�
											{
												int count;
												for(count=0;count<5;count++)//�޸ĵ�ַ
												{
													zigbee_temp_modify_dstaddr(light_addr[j]);//2017/8/25ԭ��Ϊj-1
													usleep(50000);//����ʱ��Ϊ5ms										
												}
												DBGMSG("LINE=%d,close temp_modify_addr_flag=%d\n",__LINE__,temp_modify_addr_flag);
												/*2017/8/31����ʱ�޸ĵ�ַ�ظ������ж�*/
												if(temp_modify_addr_flag==0)//��ʱ�޸ĵ�ַ��־���Ϊ0����û���յ��ظ�
												{
													int count;
													for(count=0;count<5;count++)
													{
														zigbee_temp_modify_dstaddr(light_addr[j]);//2017/8/25ԭ��Ϊj-1
														usleep(50000);//����ʱ��Ϊ5ms										
													}
												}
												else if(temp_modify_addr_flag==1)//�յ��ظ�
												{
													temp_modify_addr_flag=0;
												}
												/*2017/8/31����ʱ�޸ĵ�ַ�ظ������ж�*/
												
												light_pos[j][1]=0x00;
												int count1;
												printf("modbus, LINE=%d,close  light_pos(%d:0x%x)=%d\n",__LINE__,i, light_addr[i],light_pos[i][0]);							
												for(count1=0;count1<5;count1++)//����
												{
													zigbee_temp_dimming_node(light_pos[j][1]);
													usleep(50000);//����ʱ��Ϊ5ms
												}
												/*2017/8/31�Ե���ظ������ж�*/
												DBGMSG("LINE=%d,close adjust_light_anw_flag=%d\n",__LINE__,adjust_light_anw_flag);
												if(adjust_light_anw_flag==0)//�����־���Ϊ0����û���յ��ظ�
												{
													int count1;
													for(count1=0;count1<5;count1++)//����
													{
														zigbee_temp_dimming_node(light_pos[j][1]);
														usleep(50000);//����ʱ��Ϊ5ms
														DBGMSG("LINE=%d,close light_pos[j][0]=%d\n",__LINE__,light_pos[j][0]);
													}
												}
												else if(adjust_light_anw_flag==1)//�յ��ظ�
												{
													adjust_light_anw_flag=0;
												}
												/*2017/8/31�Ե���ظ������ж�*/
											}
										}
									}
								}
							}
							max_zone_previous=max_zone_now;
						}
						else
						{
					
						}

						
						//max_zone_previous=max_zone_now;
						zone_previous=zone_now;
					}
				}
				zigbee_temp_set_comm_mode(0x01);//ʹzigBeeͨ�ŷ�ʽΪ�㲥��ʽ
				usleep(30000);
				zigbee_temp_current_detection();
				usleep(30000);
				zigbee_temp_set_comm_mode(0x00);
				usleep(30000);

			}

			
		}
	}
	 else if(pos_now==pos_previous)//С����ֹ
	{
		posTimerStart=1;
		
		if(posCalcnum>=600)//����10����
		{
			move_flag=1;
			DBGMSG("posCalcnum=%d\n",posCalcnum);
			All_adjust_light();
			posCalcnum=0;
		}
		
		zigbee_temp_set_comm_mode(0x01);//ʹzigBeeͨ�ŷ�ʽΪ�㲥��ʽ
		usleep(30000);
		zigbee_temp_current_detection();
		usleep(30000);
		zigbee_temp_set_comm_mode(0x00);
		usleep(30000);
	}

	
	pos_previous=pos_now;
	
}

static int All_adjust_light()
{
	zigbee_temp_set_comm_mode(0x01);//ʹzigBeeͨ�ŷ�ʽΪ�㲥��ʽ
	usleep(30000);
	int i;
	
	for(i=0;i<5;i++)
	{
		zigbee_temp_dimming_node(0x06);
		usleep(50000);
	}

	int j;
	for(j=0;j<16;j++)
	{
		light_pos[j][1]=0x06;
	}

	zigbee_temp_set_comm_mode(0x00);//ʹzigBeeͨ�ŷ�ʽΪ������ʽ

	 /*2017/9/1Ϊ�˷�ֹ�㲥���������µ���*/
	int n;
	for(n=0;n<16;n++)
	{
		int count;
		for(count=0;count<5;count++)//�޸ĵ�ַ
		{
			zigbee_temp_modify_dstaddr(light_addr[n]);//2017/8/25ԭ��Ϊj-1
			usleep(50000);//����ʱ��Ϊ5ms										
		}
	
		/*2017/8/31����ʱ�޸ĵ�ַ�ظ������ж�*/
		if(temp_modify_addr_flag==0)//��ʱ�޸ĵ�ַ��־���Ϊ0����û���յ��ظ�
		{
			int count;
			for(count=0;count<5;count++)
			{
				zigbee_temp_modify_dstaddr(light_addr[n]);//2017/8/25ԭ��Ϊj-1
				usleep(50000);//����ʱ��Ϊ5ms										
			}
		}
		else if(temp_modify_addr_flag==1)//�յ��ظ�
		{
			temp_modify_addr_flag=0;
		}
		/*2017/8/31����ʱ�޸ĵ�ַ�ظ������ж�*/
		
		light_pos[n][1]=0x06;
		int count1;
		for(count1=0;count1<5;count1++)//����
		{
			printf("modbus ,LINE=%d,close light_pos[j][0]=%d\n",__LINE__,light_pos[n][0]);
			zigbee_temp_dimming_node(light_pos[n][1]);
			usleep(50000);//����ʱ��Ϊ5ms
		}
		/*2017/8/31�Ե���ظ������ж�*/
		DBGMSG("LINE=%d,close adjust_light_anw_flag=%d\n",__LINE__,adjust_light_anw_flag);
		if(adjust_light_anw_flag==0)//�����־���Ϊ0����û���յ��ظ�
		{
			int count1;
			for(count1=0;count1<5;count1++)//����
			{
				zigbee_temp_dimming_node(light_pos[n][1]);
				usleep(50000);//����ʱ��Ϊ5ms
				DBGMSG("LINE=%d,close light_pos[j][0]=%d\n",__LINE__,light_pos[n][0]);
			}
		}
		else if(adjust_light_anw_flag==1)//�յ��ظ�
		{
			adjust_light_anw_flag=0;
		}
		/*2017/8/31�Ե���ظ������ж�*/
	}
	
	/*2017/9/1Ϊ�˷�ֹ�㲥���������µ���*/
	posTimerStart=0;
}
/*��ȡС����������*/
static int get_veh_zone(int velpos)
{
	int zone=0;
	int i;
	if(velpos>0)
	{
		for(i=0;i<zone_section_num;i++)
		{
			if((velpos>=zone_section[i][0])&&(velpos<zone_section[i][1]))
				break;
		}
		zone=i;
	}
	return zone;
}

