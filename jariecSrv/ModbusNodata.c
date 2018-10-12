#include "ModbusNodata.h"


extern int no_data_count;//modbus 
extern int no_data_modbusStart;

extern int max_zone_previous;
extern int max_zone_now;
extern int zone_previous;
extern int pos_previous;
extern int move_flag;
extern int object_back_sflag;//为了判断停车超过10分钟后，小车启动开始后退，如果是后退则置1，否则为零
extern int elec_on_fwa_flag;
extern int posCalcnum;	
extern int posTimerStart;

extern void zigbee_temp_dimming_node(unsigned char ucLevel);
extern void zigbee_temp_set_comm_mode(unsigned char ucMode);
void func_modbusnodata_thread(void);

void func_modbusnodata_thread(void)//modbus没有数据时进行操作
{
	while(1)
	{
		sleep(1);
		DBGMSG("no_data_count=%d\n",no_data_count);
	       if(no_data_count>10)//2017/9/11
	      {
			//max_zone_previous=8;
			max_zone_previous=10;
			max_zone_now=0;
			zone_previous=-1;
			pos_previous=0;
			move_flag=0;
			object_back_sflag=0;
			elec_on_fwa_flag=0;
			posCalcnum=0;
			posTimerStart=0;

			
			no_data_count=0;
			DBGMSG("&&&&&&&chaoguo10ci \n");
			zigbee_request_access();
			zigbee_temp_set_comm_mode(0x01);
			usleep(30000);
			int i;
			for(i=0;i<5;i++)
			{
				zigbee_temp_dimming_node(0x0A);
				usleep(30000);
			}
			
			zigbee_temp_set_comm_mode(0x00);
			zigbee_release_access();

			usleep(30000);
	      }
	
	}


}



