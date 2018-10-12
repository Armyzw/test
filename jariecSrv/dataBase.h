#ifndef _J_DATABASE_
#define _J_DATABASE_
#include "server.h"

int setDevInfo(struct ControlInfo *cl);
int getDevInfo(struct ControlInfo *cl);
int showDevInfo(struct ControlInfo *cl);
int getAllPowerConsump();
int setAllPowerConsump(int value);
int getArmChan();
int getArmPanid();
int getQnqiaoInfo(struct QnqiaoZoonInfo *qz);
int setQnqiaoInfo(struct QnqiaoZoonInfo *qz);
int getArmInfo(struct DeviceInfo *di);
#endif
