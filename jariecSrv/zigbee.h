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
    char szDevName[16];             //�豸����
    char szDevPwd[16];              //�豸����
    unsigned char ucDevMode;        //�豸����
    unsigned char ucChan;           //ͨ����
    unsigned short wPanID;          //����ID
    unsigned short wMyAddr;         //���������ַ
    char szMyIEEE[8];               //���������ַ
    unsigned short wDstAddr;        //Ŀ�������ַ
    char szDstIEEE[8];              //Ŀ�������ַ
    unsigned char ucReserve;        //����
    unsigned char ucPowerLevel;     //���书��
    unsigned char ucRetryNum;       //�����������Դ���
    unsigned char ucTranTimeOut;    //������������ʱ����
    unsigned char ucSerialRate;     //���ڲ�����
    unsigned char ucSerialDataB;    //��������λ
    unsigned char ucSerialStopB;    //����ֹͣλ
    unsigned char ucSerialParityB;  //����У��λ
    unsigned char ucSendMode;       //����ģʽ
    unsigned char ucRunMode;        //����״̬
    unsigned short wDeviceType;     //�豸����
    unsigned short wfirmware;       //�̼��汾
}SZigbeeDevInfo;
#pragma pack ()
void zigbee_request_access();
void zigbee_release_access();

void zigbee_read_light_by_dstaddr(short wAddr);

//1����ʱ�޸�ͨ����
void zigbee_temp_modify_chan(unsigned char ucChan);

//2����ʱ�޸�Ŀ�������ַ
void zigbee_temp_modify_dstaddr(unsigned short wAddr);

//3����ʱ�޸İ�ͷ��ʾԴ��ַ
void zigbee_temp_modify_visible(bool bVisible);

//4����ʱ����ͨѶģʽ
void zigbee_temp_set_comm_mode(unsigned char ucMode);

//5����ѯ�ڵ��ź�ǿ��
void zigbee_query_signal_strength(unsigned short wAddr);

//6����ȡ��������ָ��
void zigbee_read_local_config(void);

//7�������޸�ͨ����
void zigbee_always_modify_chan(unsigned char ucChan);

//8���������߽ڵ�
void zigbee_query_online_node(void);

//9����ȡԶ��������Ϣ
void zigbee_read_remote_config(unsigned short wAddr);

//10���޸���������
void zigbee_modify_remote_config(unsigned short wAddr, struct SZigbeeDevInfo *psDevInfo);

//11����λ�ڵ�
void zigbee_reset_node(unsigned short wAddr, unsigned short wType);

//12���ڵ�ָ���������
void zigbee_restore_factory_defaults(unsigned short wAddr, unsigned short wType);

//13���ڵ����
void zigbee_temp_dimming_node(unsigned char ucLevel);

//14���������
void zigbee_temp_current_detection(void);//zhangmodify2017/08/17
void zigbee_temp_current_power(void);
int zigbee_temp_dimming_node_answ_get();
#endif

