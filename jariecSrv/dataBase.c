/*
  Copyright (c) 2009-2017 Dave Gamble and cJSON contributors

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"
#include "server.h"
/* Used by some code below as an example datatype. */
struct record
{
    const char *precision;
    double lat;
    double lon;
    const char *address;
    const char *city;
    const char *state;
    const char *zip;
    const char *country;
};

struct lightCtl{
    char addr[32];
    int id;
    int brightness;
    int status;
};

char* read_file(const char *filename) {
    FILE *file = NULL;
    long length = 0;
    char *content = NULL;
    size_t read_chars = 0;

    /* open in read binary mode */
    file = fopen(filename, "rb");
    if (file == NULL)
    {
        goto cleanup;
    }

    /* get the length */
    if (fseek(file, 0, SEEK_END) != 0)
    {
        goto cleanup;
    }
    length = ftell(file);
    if (length < 0)
    {
        goto cleanup;
    }
    if (fseek(file, 0, SEEK_SET) != 0)
    {
        goto cleanup;
    }

    /* allocate content buffer */
    content = (char*)malloc((size_t)length + sizeof(""));
    if (content == NULL)
    {
        goto cleanup;
    }

    /* read the file into memory */
    read_chars = fread(content, sizeof(char), (size_t)length, file);
    if ((long)read_chars != length)
    {
        free(content);
        content = NULL;
        goto cleanup;
    }
    content[read_chars] = '\0';


cleanup:
    if (file != NULL)
    {
        fclose(file);
    }

    return content;
}

cJSON *parse_file(const char *filename)
{
    cJSON *parsed = NULL;
    char *content = read_file(filename);

    if(content){ 	
	parsed = cJSON_Parse(content);
	if (content != NULL)
        {
            free(content);
        }
    }else
	return NULL;

    return parsed;
}


void printJson(cJSON * root)//以递归的方式打印json的最内层键值对
{
    char *value;
    int i;
    for(i=0; i<cJSON_GetArraySize(root); i++)   //遍历最外层json键值对
    {
        cJSON * item = cJSON_GetArrayItem(root, i);        
        if(cJSON_Object == item->type){      //如果对应键的值仍为cJSON_Object就递归调用printJson
            printf("item->type is Object \n");
	    printJson(item);
       }
        else                                //值不为json对象就直接打印出键和值
        {
            printf("h|%s->", item->string);
	    value = cJSON_Print(item);
            printf("c|%s\n", value);
	    free(value);
        }
    }
}

int countOfDevice(cJSON *root)
{
	return cJSON_GetArraySize(root);
}


int getDeviceItem(cJSON *root, int index, struct LightInfo *li)
{
	char *value;
	int i;
	cJSON *item = cJSON_GetArrayItem(root ,index);
	if(item->type== cJSON_Object){
	    for(i=0; i<cJSON_GetArraySize(item);i++)
	    {
		cJSON *child = cJSON_GetArrayItem(item, i);
		value = cJSON_Print(child);
		if(strcmp(child->string, "MAC addr")==0){
		    strncpy(li->mac, value+1, strlen(value)-2);
		}else if(strcmp(child->string, "Brightness")==0){
		    li->value = atoi(value);   
		}else if(strcmp(child->string, "Id")==0){
		    li->id = atoi(value);
		}else if(strcmp(child->string, "Status")==0){
		    li->status = atoi(value);
		}else if(strcmp(child->string, "NetAddr")==0){
		    strncpy(li->netAddr, value+1, strlen(value)-2);
		}
		free(value);
	    }
	}else{
		value = cJSON_Print(item);
		if(strcmp(item->string, "MAC addr")==0){
		    strcpy(li->mac, value);
		}else if(strcmp(item->string, "Brightness")==0){
		    li->value = atoi(value);   
		}else if(strcmp(item->string, "Id")==0){
		    li->id = atoi(value);
		}else if(strcmp(item->string, "Status")==0){
		    li->status = atoi(value);
		}
		free(value);
	}
	return 0;
}




/* Create a bunch of objects as demonstration. */
static void createDatabase(void)
{
    /* declare a few. */
    cJSON *root = NULL;
    cJSON *fmt = NULL;
    cJSON *img = NULL;
    cJSON *thm = NULL;
    cJSON *fld = NULL;
    int i = 0;

    struct lightCtl list[3] = {
		{
			"EABAFBCD01",
			1,
			50,
			1
		},
		{
			"EABAFBCD02",
			2,
			50,
			1
		},
		{
			"EABAFBCD03",
			3,
			50,
			1
		},
	};

    /* Our array of "records": */
    root = cJSON_CreateArray();
    for (i = 0; i < 3; i++)
    {
        cJSON_AddItemToArray(root, fld = cJSON_CreateObject());
        cJSON_AddStringToObject(fld, "MAC addr", list[i].addr);
        cJSON_AddNumberToObject(fld, "Id", list[i].brightness);
        cJSON_AddNumberToObject(fld, "Brightness", list[i].brightness);
        cJSON_AddNumberToObject(fld, "Status", list[i].status);
    }

    char *data = cJSON_Print(root);
        printf("%s\n", data);

    printf("write data to %s\n", DATABASE); 
    FILE *fp = fopen(DATABASE, "w");
    if(fp){
	fprintf(fp, "%s", data);
	fclose(fp);
	}   
    free(data);

    cJSON_Delete(root);
}

int getArmId(char *buffer)
{
	FILE *fp = fopen(ARMCFG, "r");
	if(fp){
		fscanf(fp, "%s", buffer);
		fclose(fp);
		return 0;
	}
	return -1;
}

int getArmPanid()
{
	char buffer[32] = "";
	int chan=0;
        FILE *fp = fopen(ARMCFG, "r");
        if(fp){
                fscanf(fp, "%s", buffer);
                fscanf(fp, "%s", buffer);
		memset(buffer, 0, sizeof(buffer));
                fscanf(fp, "%s", buffer);
                fclose(fp);

		sscanf(buffer, "%x", &chan);
                return chan;
        }
        return -1;
}
int getArmChan()
{
	char buffer[32] = "";
	int chan=0;
        FILE *fp = fopen(ARMCFG, "r");
        if(fp){
                fscanf(fp, "%s", buffer);
		memset(buffer, 0, sizeof(buffer));
                fscanf(fp, "%s", buffer);
                fclose(fp);

		sscanf(buffer, "%x", &chan);
                return chan;
        }
        return -1;
}

int setAllPowerConsump(int value)
{
	char buffer[32] = "";
	FILE *fp = fopen(POWERCFG, "w");
	if(fp){
		fprintf(fp, "%d", value);
		fclose(fp);
		return 0;
	}
	return -1;

}
int getAllPowerConsump()
{
	char buffer[32] = "";
	FILE *fp = fopen(POWERCFG, "r");
	if(fp){
		fscanf(fp, "%s", buffer);
		fclose(fp);
		return atoi(buffer);
	}
	return -1;

}

int getQnqiaoItem(cJSON *root, int index, struct QnqiaoZoon  *li)
{
        char *value;
        int i;
        cJSON *item = cJSON_GetArrayItem(root ,index);
        if(item->type== cJSON_Object){
            for(i=0; i<cJSON_GetArraySize(item);i++)
            {
                cJSON *child = cJSON_GetArrayItem(item, i);
                value = cJSON_Print(child);
                if(strcmp(child->string, "Start")==0){
                    li->start = atoi(value);
                }else if(strcmp(child->string, "End")==0){
                    li->end = atoi(value);
                }else if(strcmp(child->string, "Status")==0){
                    li->status = atoi(value);
                }
                free(value);
            }
        }else{
                value = cJSON_Print(item);
                if(strcmp(item->string, "Start")==0){
                    li->start = atoi(value);
                }else if(strcmp(item->string, "End")==0){
                    li->end = atoi(value);
                }else if(strcmp(item->string, "Status")==0){
                    li->status = atoi(value);
                }
                free(value);
        }
        return 0;
}


int getQnqiaoInfo(struct QnqiaoZoonInfo *qz)
{
    int count=0, i;
    
    memset(qz, 0, sizeof(struct QnqiaoZoonInfo));

    cJSON *tree;

    tree = parse_file(QNQIAOCFG);
    if(tree){
	count = countOfDevice(tree);
	qz->count = count;
	for(i=0; i<count; i++){
	   getQnqiaoItem(tree,i, &(qz->az[i])); 
	}
    }else{
	count = -1;
    }

    cJSON_Delete(tree);
    return count;
}

int getArmItem(cJSON *root, int index, struct DeviceInfo  *li)
{
        char *value;
        int i;
        cJSON *item = cJSON_GetArrayItem(root ,index);
        if(item->type== cJSON_Object){
            for(i=0; i<cJSON_GetArraySize(item);i++)
            {
                cJSON *child = cJSON_GetArrayItem(item, i);
                value = cJSON_Print(child);
                if(strcmp(child->string, "ArmId")==0){
					strncpy(li->armId, value+1, strlen(value)-2);
                }else if(strcmp(child->string, "ChanelId")==0){
					sscanf(value+1, "%hx", &(li->chanId));
                }else if(strcmp(child->string, "PanId")==0){
					sscanf(value+1, "%hx", &(li->panId));
				}else if(strcmp(child->string, "NetId")==0){
					sscanf(value+1, "%hx", &(li->netId));
				}else if(strcmp(child->string, "Status")==0){
                    li->status = atoi(value);
                }
                free(value);
            }
        }else{
                value = cJSON_Print(item);
				if(strcmp(item->string, "ArmId")==0){
                    strncpy(li->armId, value+1, strlen(value)-2);
                }else if(strcmp(item->string, "ChanelId")==0){
                    sscanf(value+1, "%hx", &(li->chanId));
                }else if(strcmp(item->string, "PanId")==0){
					sscanf(value+1, "%hx", &(li->panId));
				}else if(strcmp(item->string, "NetId")==0){
					sscanf(value+1, "%hx", &(li->netId));
				}else if(strcmp(item->string, "Status")==0){
                    li->status = atoi(value);
                }
                free(value);
        }
        return 0;
}


int getArmInfo(struct DeviceInfo *di)
{
    int count=0, i;
    
    memset(di, 0, sizeof(struct DeviceInfo));

    cJSON *tree;
	
    tree = parse_file(ARMCFG);
    if(tree){
	    getArmItem(tree,0, di); 
		cJSON_Delete(tree);
	}else
		count = -1;
    return count;
}

int getDevInfo(struct ControlInfo *cl)
{
    int i, count;
    char *value;
    struct DeviceInfo di;
    memset(&di, 0, sizeof(struct DeviceInfo));
    memset(cl, 0, sizeof(struct ControlInfo));
	
   // getArmId(cl->id);
    getArmInfo(&di);

    strcpy(cl->id, di.armId);

    cJSON *tree = parse_file(DATABASE);
    if(tree){	
	count = countOfDevice(tree);	
	cl->count = count;
	for(i=0; i<count; i++){
	    getDeviceItem(tree, i, &(cl->li[i])); 
	}
    }else{
	return -1;
    }
    cJSON_Delete(tree);
    return count;
}

int setDevInfo(struct ControlInfo *cl)
{	
    cJSON *root = NULL;
    cJSON *fmt = NULL;
    cJSON *img = NULL;
    cJSON *thm = NULL;
    cJSON *fld = NULL;
    int ret = 0;
    int i = 0;
    /* Our array of "records": */
    root = cJSON_CreateArray();
    for (i = 0; i < cl->count; i++)
    {
        cJSON_AddItemToArray(root, fld = cJSON_CreateObject());
        cJSON_AddStringToObject(fld, "MAC addr", cl->li[i].mac);
        cJSON_AddStringToObject(fld, "NetAddr", cl->li[i].netAddr);
        cJSON_AddNumberToObject(fld, "Id", cl->li[i].id);
        cJSON_AddNumberToObject(fld, "Brightness", cl->li[i].value);
        cJSON_AddNumberToObject(fld, "Status", cl->li[i].status);
    }

    char *p = cJSON_Print(root);
    cJSON_Delete(root);
    FILE *fp = fopen(DATABASE, "w");
    if(fp){
	fprintf(fp, "%s", p);
    	fclose(fp);
    }else
	ret = -2;

    free(p);

    return ret;
}
int setQnqiaoInfo(struct QnqiaoZoonInfo *qz)
{
    cJSON *root = NULL;
    cJSON *fmt = NULL;
    cJSON *img = NULL;
    cJSON *thm = NULL;
    cJSON *fld = NULL;
    int ret = 0;
    int i = 0;
    /* Our array of "records": */
    root = cJSON_CreateArray();
    for (i = 0; i < qz->count; i++)
    {
        cJSON_AddItemToArray(root, fld = cJSON_CreateObject());
        cJSON_AddNumberToObject(fld, "Start", qz->az[i].start);
        cJSON_AddNumberToObject(fld, "End", qz->az[i].end);
        cJSON_AddNumberToObject(fld, "Status", qz->az[i].status);
    }

    char *p = cJSON_Print(root);
    cJSON_Delete(root);
    FILE *fp = fopen(QNQIAOCFG, "w");
    if(fp){
        fprintf(fp, "%s", p);
        fclose(fp);
    }else
        ret = -2;

    free(p);

    return ret;
}

void showDevInfo(struct ControlInfo *cl)
{
        int i;
        printf("Control id: %s\n", cl->id);
        for(i=0; i<cl->count; i++)
        {
                printf("Id:%02d-%3d-%64s\n", cl->li[i].id, 
			cl->li[i].value, cl->li[i].mac);
        }
}
void showQnqiaoInfo(struct QnqiaoZoonInfo *cl)
{
        int i;
        for(i=0; i<cl->count; i++)
        {
                printf("Zoon:%4d-%4d\n", cl->az[i].start, 
			cl->az[i].end);
        }

}
#if 0
int main(void)
{
   int count;
#if 1

   struct QnqiaoZoonInfo qz;

   count = getQnqiaoInfo(&qz);   
   printf("count is  %d\n", count);
   showQnqiaoInfo(&qz);
   
   qz.az[1].start = 4000;
   qz.az[1].end = 4000;
   qz.az[1].status = 2;
   setQnqiaoInfo(&qz);
#else
    struct ControlInfo cl;
    memset(&cl, 0, sizeof(cl));
    getDevInfo(&cl);

    showDevInfo(&cl);
    cl.li[0].value = 68;
    setDevInfo(&cl);
#endif
    return 0;
}
#endif
