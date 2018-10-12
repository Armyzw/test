#include "gps.h"
#include "gpsS1216F8.h"

static int sg_iFd=0;

nmea_msg gGpsInfo;
static unsigned char sg_szGpsMsg[128];
static unsigned short sg_wGpsMsgLen=0;

static void MainLoop(int *);
static void RxMsgParse(void);
static bool GetFieldStr(unsigned char ucDstField, char *pcFieldStr);

void func_gps_thread(int *iFd)
{
    //设置线程为可取消
    if (pthread_setcancelstate (PTHREAD_CANCEL_ENABLE , NULL))
    {
        perror ("Gps thread setcancelstate failed");
        pthread_exit(NULL);
    }

    if (!iFd)
        pthread_exit(NULL);
    sg_iFd=*iFd;
    if (sg_iFd<=0)
        pthread_exit(NULL);

    while(true)
        MainLoop(&sg_iFd);

    pthread_exit(NULL);
}

static void MainLoop(int *fd)
{
    unsigned char buffer[128] = "";
    int len=0;
    int ret;
    nmea_msg *p = &gGpsInfo;
    struct timeval tv;
    fd_set sRset;
    char data;
    int count = 0;
    memset(p , 0, sizeof(nmea_msg));
    while(1)
    {

        tv.tv_sec=0;
        tv.tv_usec=5*1000;

        FD_ZERO(&sRset);
        FD_SET(*fd,&sRset);

        if (select(*fd+1, &sRset , NULL , NULL , &tv) >0)
        {
            if (read(*fd, &data, 1)<=0)
            {
                break;
            }else{
                buffer[len++] = data;
                if(buffer[len-1] == 0xa && buffer[len-2] == 0xd)		//接收到一行完整数据
                {
                        //showRecv(buffer, len);
                        ret = NMEA_GNGGA_Analysis(p, buffer);			//进行GNGGA分析
                        //printf("GPS,ret=%d,  local type %d\n", ret, p.gpssta);
                        ret = NMEA_GNRMC_Analysis(p ,buffer);			//GNRMC分析
	
                        ret = NMEA_GNGSA_Analysis(p, buffer);			//进行GNGSA分析
		 if(ret == 0){
                            if(p->fixmode>1){		//定位成功
				if((count++%10) == 0){				
                                printf("GPS,%02d:%02d:%02d, %c:%d, %c:%d\n",
                                p->utc.hour, p->utc.min, p->utc.sec,
                                p->nshemi, p->latitude,p->ewhemi, p->longitude);
				}
				//memcpy();
                            }
                        }
                        len = 0;
                        memset(buffer, 0, sizeof(buffer));
                }
            }
	}
    }
}

