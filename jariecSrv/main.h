#include <signal.h>
#include "led.h"

#ifndef _MAIN_H
#define _MAIN_H

static void OpenModules(void);

static void CloseModules(void);

static void CreatThread(void);

static void CancelThread(void);

static void InitCom(int iFd, unsigned int dwBaudrate, unsigned char ucVerify);

#endif

