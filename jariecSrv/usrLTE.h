#ifndef _J_USRLTE_
#define _J_USRLTE_

#define SOCKA_CONNECTED	"SOCKALK:ON"
#define RECV_SOCKA	"SOCKA:"
#define OK		"OK"
int LTEXSend(int fd, const unsigned char *data, int length);
int LTEXRecv(int fd, unsigned char *data, int size);
int LTEXConnect(int fd, const char *ipaddr, short port);
int LTEXClose(int fd);
#endif

