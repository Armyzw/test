CC = arm-linux-gcc
RM = rm

MODULES=main.o network.o gps.o modbus.o zigbee.o ModbusNodata.o usrLTE.o protocol.o dataBase.o power.o gpsS1216F8.o

LedApp:${MODULES} 
	$(CC) -O2 -Wall -D_DBGMSG -D_REENTRANT -o JLedSrv ${MODULES} -lpthread -Llib_arm/ -lcjson

${MODULES}:%.o: %.c %.h led.h

clean:
	${RM} -f *.o LedApp

