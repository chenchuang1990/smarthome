#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"
#include "list.h"

#define LOCALCONFPATH "./ceconf.json"
#define LISTENPORT "listenport"
#define SERVERADDR "serveraddr"
#define SERVERPORT "serverport"
#define SERIALPORT "serialport"
#define TIMEOUT "timeout"


static struct ceconf{
	char * listenport;
	char * serveraddr;
	char * serverport;

	char * serialport;

	int timeout;

}s_conf = {
	"8989",
	"das.gateway.axtchild.com",
	"8887",
	"/dev/ttyO1",
	180,
};

char * ceconf_getlistenport(){
	return s_conf.listenport;
}

char * ceconf_getserveraddr(){
	return s_conf.serveraddr;
}

char * ceconf_getserverport(){
	return s_conf.serverport;
}

char * ceconf_getserialport(){
	return s_conf.serialport;
}

int ceconf_gettimeout(){
	return s_conf.timeout;
}


cJSON * loadconffile(char * path){
	FILE *f;long len;char *data;
	f=fopen(path,"rb");fseek(f,0,SEEK_END);len=ftell(f);fseek(f,0,SEEK_SET);
	data=(char*)malloc(len+1);fread(data,1,len,f);data[len]='\0';fclose(f);
	cJSON *json;
	json=cJSON_Parse(data);
	free(data); 
	return json;
}       

void loadconf(cJSON *subitem){
	if(!subitem){
		return;
	}       
	cJSON * listenport;
	cJSON * serveraddr;
	cJSON * serverport;
	cJSON * serialport;
	cJSON * timeout;
	listenport = cJSON_GetObjectItem(subitem, LISTENPORT);
	if(listenport && listenport->type == cJSON_String){
		s_conf.listenport = strdup(listenport->valuestring);
	}
	serveraddr = cJSON_GetObjectItem(subitem, SERVERADDR);
	if(serveraddr && serveraddr->type == cJSON_String){
		s_conf.serveraddr = strdup(serveraddr->valuestring);
	}
	serverport = cJSON_GetObjectItem(subitem, SERVERPORT);
	if(serverport && serverport->type == cJSON_String){
		s_conf.serverport = strdup(serverport->valuestring);
	}
	serialport  = cJSON_GetObjectItem(subitem, SERIALPORT);
	if(serialport && serialport->type == cJSON_String){
		s_conf.serialport = strdup(serialport->valuestring);
	}
	timeout = cJSON_GetObjectItem(subitem, TIMEOUT);
	if(timeout && timeout->type == cJSON_String){
		s_conf.timeout = atoi(timeout->valuestring);
	}
}

void ceconf_load()
{
	cJSON * localjson = loadconffile(LOCALCONFPATH);
	loadconf(localjson);
	cJSON_Delete(localjson);
}














