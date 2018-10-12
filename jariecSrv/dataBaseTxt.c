#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include "server.h"
#define DATABASE "devInfo.db"


int setDevInfo(struct ControlInfo *cl)
{
	FILE *fp;
	fp = fopen(DATABASE, "w");
	if(fp){
		fprintf(fp, "%s %d %d %d %d %d %d %d %d %d %d %d %d\n",
		cl->id,
		cl->li[0].value,
		cl->li[1].value,
		cl->li[2].value,
		cl->li[3].value,
		cl->li[4].value,
		cl->li[5].value,
		cl->li[6].value,
		cl->li[7].value,
		cl->li[8].value,
		cl->li[9].value,
		cl->li[10].value,
		cl->li[11].value);
		fclose(fp);
	}
	return 0;
}

int getDevInfo(struct ControlInfo *cl)
{
	FILE *fp;
	int lg[12];
	char armId[16] = "";
	int i;
	fp = fopen(DATABASE, "r");
	if(fp){
		fscanf(fp, "%s %d %d %d %d %d %d %d %d %d %d %d %d",
		armId, &lg[0], &lg[1], &lg[2],&lg[3], &lg[4], &lg[5],
		&lg[6], &lg[7], &lg[8],&lg[9], &lg[10], &lg[11]  );


		fclose(fp);
		strcpy(cl->id, armId);
		cl->count = 12;
		for(i=0;i <cl->count; i++){
			cl->li[i].id = i+1;
			cl->li[i].value = lg[i];
		}
	}else{
		printf("Database, can't find devinfo.db, create it\n");
			
		strcpy(cl->id, "10000002");
		cl->count = 12;
		for(i=0;i <cl->count; i++){
			cl->li[i].id = i+1;
			cl->li[i].value = lg[i];
		}
		setDevInfo(cl);
		
	}
	return 0;
}
void showDevInfo(struct ControlInfo *cl)
{
	int i;
	printf("Control id: %s\n", cl->id);
	for(i=0; i<cl->count; i++)
	{
		printf("Id:%02d-%3d\n", cl->li[i].id, cl->li[i].value);
	}
}
#if 0
int main()
{
	struct ControlInfo cl;
	getDevInfo(&cl);
	showDevInfo(&cl);
	sleep(10);
	cl.li[0].value = 86;
	setDevInfo(&cl);
	getDevInfo(&cl);
	showDevInfo(&cl);
	return 0;
}
#endif
