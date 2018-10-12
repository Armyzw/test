#include "zigbee.h"
#include "testZigbee.h"



extern unsigned char xg_ZigbeeData[128];
extern int adjust_light_anw_flag;
extern unsigned char sg_ucaRxMsg[256];


//线程定义
pthread_t g_sNetworkThread, g_sModbusThread, g_sZigbeeThread, g_sGpsThread,g_ModbusNodata;

static int sg_iaFdCom[4];

/*zhangmodify2017/8/11*/
int Timer(void);
int posCalcnum=0;
extern int posTimerStart;

int no_data_count=0;//2017/8/19 modbus 没有数据时计数
extern int no_data_modbusStart;

/*zhangmodify2017/8/11*/

static void OpenModules(void)
{
    //com1 for gps
    sg_iaFdCom[0] = open( "/dev/ttySP1", O_RDWR | O_NOCTTY);
    if (sg_iaFdCom[0]<1)
        perror("Can't open the /dev/ttySP1");
    else
        InitCom(sg_iaFdCom[0],B115200,NONE);

    //com2 for modbus
    sg_iaFdCom[1] = open( "/dev/ttySP2", O_RDWR | O_NOCTTY);
    if (sg_iaFdCom[1]<1)
    {
    	 perror("Can't open the /dev/ttySP2");
	 printf("ttySP2you cuowu\n");
    }  
    else
        InitCom(sg_iaFdCom[1],B19200,EVEN);//偶校验
        
   //com0 fro zigbee
    sg_iaFdCom[2] = open( "/dev/ttySP0", O_RDWR | O_NOCTTY);
    if (sg_iaFdCom[2]<1)
        perror("Can't open the /dev/ttySP0");
    else
        InitCom(sg_iaFdCom[2],B115200,NONE);
   
    //com3 fro LTE
    sg_iaFdCom[3] = open( "/dev/ttySP3", O_RDWR | O_NOCTTY);
    if (sg_iaFdCom[3]<1)
        perror("Can't open the /dev/ttySP0");
    else
        InitCom(sg_iaFdCom[3],B115200,NONE);
}

static void CloseModules(void)
{
    int i;
    
    for(i=0; i<4; i++){
    	if (sg_iaFdCom[i]>0)
    	{
            (void)close (sg_iaFdCom[i]);
            sg_iaFdCom[i]=0;
        }
    }
}

static void CreatThread(void)
{
#if 0
    if (pthread_create (&g_sNetworkThread, NULL , (void *)&func_network_thread, &sg_iaFdCom[3]))
        perror("Network thread create failed");
#endif
#if 0
    if (pthread_create (&g_sModbusThread, NULL , (void *)&func_modbus_thread, &sg_iaFdCom[1]))
        perror("Modbus thread create failed");
#endif
#if 1
    if (pthread_create (&g_sZigbeeThread, NULL , (void *)&func_zigbee_thread, &sg_iaFdCom[2]))
        perror("Zigbee thread create failed");
#endif
#if 0
    if (pthread_create (&g_sGpsThread , NULL , (void *)&func_gps_thread, &sg_iaFdCom[0]))
        perror("Zigbee thread create failed");
#endif
#if 0
    if(pthread_create (&g_ModbusNodata , NULL , (void *)&func_modbusnodata_thread, NULL))
	 perror("Modbus Nodata thread create failed");	
#endif
}

static void CancelThread(void)
{
    if (pthread_cancel(g_sNetworkThread))
        perror("Network thread cancel failed");

    if (pthread_cancel(g_sModbusThread))
        perror("Modbus thread cancel failed");

    if (pthread_cancel(g_sZigbeeThread))
        perror("Zigbee thread cancel failed");

    if (pthread_cancel(g_sGpsThread))
        perror("Gps thread cancel failed");

    if (pthread_cancel(g_ModbusNodata))
	 perror("Modbus Nodata thread cancel failed");
}


//####### 初始化串口 #######//
static void InitCom(int iFd, unsigned int dwBaudrate, unsigned char ucVerify)
{
    struct termios sTermios;

    memset(&sTermios,0,sizeof(sTermios));       //清空串口设置结构体
    
    sTermios.c_cflag = CLOCAL | CREAD; //CLOCAL:本地连接, CREAD:允许接收数据

    (void)cfsetispeed(&sTermios,dwBaudrate);
    (void)cfsetospeed(&sTermios,dwBaudrate);

    if (ucVerify>0)
    {
        sTermios.c_cflag |= PARENB;       //Enable parity bit

        if (ucVerify==1)
            sTermios.c_cflag |= PARODD;   //Use odd parity instead of even
        else
            sTermios.c_cflag &= ~PARODD;  //Use even parity instead of odd
    }
    else
        sTermios.c_cflag &=~ PARENB;      //Disable parity bit

    
    sTermios.c_cflag &= ~CSTOPB;  // 1 stop bits 
    sTermios.c_cflag &= ~CSIZE;       // Mask the character size bits 
    sTermios.c_cflag |= CS8;      // Select 8 data bits 
    
    //不处理，原始数据输入
    sTermios.c_lflag = 0;
    
    //INPCK   : 奇偶校验检查    
    sTermios.c_iflag = INPCK;

    //不处理，原始数据输出
    sTermios.c_oflag = 0;     

    sTermios.c_cc[VINTR]      = 0;        // Ctrl+c  
    sTermios.c_cc[VQUIT]      = 0;        /*Ctrl+\*/ 
    sTermios.c_cc[VERASE]     = 0;        // del 
    sTermios.c_cc[VKILL]      = 0;        // @ 
    sTermios.c_cc[VEOF]       = 4;        // Ctrl+d 
    sTermios.c_cc[VTIME]      = 0;        // 不使用字符间的计时器,原来是0
    sTermios.c_cc[VMIN]       = 1;        // 阻塞，直到读取到一个字符,原来为1/2017/8/29
    sTermios.c_cc[VSWTC]      = 0;        // '' 
    sTermios.c_cc[VSTART]     = 0;        // Ctrl+q 
    sTermios.c_cc[VSTOP]      = 0;        // Ctrl+s 
    sTermios.c_cc[VSUSP]      = 0;        // Ctrl+z 
    sTermios.c_cc[VEOL]       = 0;        // '' 
    sTermios.c_cc[VREPRINT]   = 0;        // Ctrl+r 
    sTermios.c_cc[VDISCARD]   = 0;        // Ctrl+u 
    sTermios.c_cc[VWERASE]    = 0;        // Ctrl+w 
    sTermios.c_cc[VLNEXT]     = 0;        // Ctrl+v 
    sTermios.c_cc[VEOL2]      = 0;        // '' 

    (void)tcflush(iFd, TCIFLUSH);
    (void)tcsetattr(iFd,TCSANOW,&sTermios);
}

int Timer(void)
{
	//DBGMSG("posTimerStart=%d\n",posTimerStart);
	if(posTimerStart)
	{
	//	DBGMSG("zoneTimerStart1\n");
		posCalcnum++;
	}
	else
	{
	//	DBGMSG("zoneTimerStart2\n");
		posCalcnum=0;
	}

	if(no_data_modbusStart)//modbus 没有数据时开始计数
	{
		no_data_count++;
	}
	else
	{
		no_data_count=0;
	}
}

int main(int argc, char **argv)
{
    signal(SIGPIPE,SIG_IGN);

    /*zhangmodify2017/8/11*/

    struct itimerval tick;

    signal(SIGALRM,Timer);
    memset(&tick,0,sizeof(tick));
    tick.it_value.tv_sec=1;
    tick.it_value.tv_usec=0;

    tick.it_interval.tv_sec=1;
    tick.it_interval.tv_usec=0;

  //  if(setitimer(ITIMER_REAL,&tick,NULL)<0)
   // {
//	   printf("Set Timer failed\n");
 //   }
    /*zhangmodify2017/8/11*/					

    //打开模块
    OpenModules();
    
    //创建线程
    CreatThread();

    short wAddr = 0x2004;
    int value = 0x5;
    int index=1;
    while(index++)
    {
	if(index>0x555555)	
		index = 1;

	if((index%2) == 0)
		wAddr = 0x2004;
	else
		wAddr = 0x2000;
	zigbee_temp_modify_dstaddr(wAddr);
        usleep(30*1000);
        zigbee_temp_dimming_node(value);
        usleep(30*1000);
	if(zigbee_temp_dimming_node_answ_get()){
                printf("Zigbee result %02x\n", xg_ZigbeeData[0], 1);
        }else{
                printf("cat't connect with %x\n", wAddr);
        }
	system("date");
	printf("--------------------------------------\n\n");
        usleep(2*1000*1000);
	
    }

    CancelThread();

    CloseModules();
    
    return EXIT_SUCCESS;
}

