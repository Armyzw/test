#ifndef _J_PROTOCOL_
#define _J_PROTOCOL_
int XHandleCmd(struct packData *sd, int sockFd, int netType);
int show_hex(unsigned char *buffer, int length, const char *title);

#endif
