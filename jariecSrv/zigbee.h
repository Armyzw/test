#include "led.h"


#ifndef _ZIGBEE_H
#define _ZIGBEE_H
#pragma pack (1)
typedef unsigned char u8;
typedef struct {
    u8 start;
    u8 addr[3];
    u8 dstAddr[3];
    u8 type;
    u8 cmd;
    u8 data[64];
    u8 size;
} loraInfo;


struct SZigbeeDevInfo
{
    char szDevName[16];             //设备名称
    char szDevPwd[16];              //设备密码
    unsigned char ucDevMode;        //设备类型
    unsigned char ucChan;           //通道号
    unsigned short wPanID;          //网络ID
    unsigned short wMyAddr;         //本地网络地址
    char szMyIEEE[8];               //本地物理地址
    unsigned short wDstAddr;        //目标网络地址
    char szDstIEEE[8];              //目标物理地址
    unsigned char ucReserve;        //保留
    unsigned char ucPowerLevel;     //发射功率
    unsigned char ucRetryNum;       //发送数据重试次数
    unsigned char ucTranTimeOut;    //发送数据重试时间间隔
    unsigned char ucSerialRate;     //串口波特率
    unsigned char ucSerialDataB;    //串口数据位
    unsigned char ucSerialStopB;    //串口停止位
    unsigned char ucSerialParityB;  //串口校验位
    unsigned char ucSendMode;       //发送模式
    unsigned char ucRunMode;        //运行状态
    unsigned short wDeviceType;     //设备类型
    unsigned short wfirmware;       //固件版本
}SZigbeeDevInfo;
#pragma pack ()
void zigbee_request_access();
void zigbee_release_access();

void zigbee_read_light_by_dstaddr(short wAddr);

//1、临时修改通道号
void zigbee_temp_modify_chan(unsigned char ucChan);

//2、临时修改目的网络地址
void zigbee_temp_modify_dstaddr(unsigned short wAddr);

//3、临时修改包头显示源地址
void zigbee_temp_modify_visible(bool bVisible);

//4、临时设置通讯模式
void zigbee_temp_set_comm_mode(unsigned char ucMode);

//5、查询节点信号强度
void zigbee_query_signal_strength(unsigned short wAddr);

//6、读取本地配置指令
void zigbee_read_local_config(void);

//7、永久修改通道号
void zigbee_always_modify_chan(unsigned char ucChan);

//8、搜索在线节点
void zigbee_query_online_node(void);

//9、获取远程配置信息
void zigbee_read_remote_config(unsigned short wAddr);

//10、修改配置命令
void zigbee_modify_remote_config(unsigned short wAddr, struct SZigbeeDevInfo *psDevInfo);

//11、复位节点
void zigbee_reset_node(unsigned short wAddr, unsigned short wType);

//12、节点恢复出厂设置
void zigbee_restore_factory_defaults(unsigned short wAddr, unsigned short wType);

//13、节点调光
void zigbee_temp_dimming_node(unsigned char ucLevel);

//14、电流检测
void zigbee_temp_current_detection(void);//zhangmodify2017/08/17
void zigbee_temp_current_power(void);
int zigbee_temp_dimming_node_answ_get();
#endif

