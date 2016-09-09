#ifndef __PROTOCOL_DATATYPE_H_H
#define __PROTOCOL_DATATYPE_H_H

#include "time.h"

struct protocol_datatype_del_device{
	unsigned long long ieee;
	unsigned int serialnum;
};

struct protocol_datatype_get_device_attr{
	unsigned long long ieee;
	unsigned int serialnum;
	unsigned char endpoint;
};

struct protocol_datatype_set_device_name{
	unsigned long long ieee;
	unsigned int serialnum;
	unsigned char namelen;
	char name[256];
};

struct protocol_datatype_online_status{
	unsigned long long ieee;
	unsigned int serialnum;
	unsigned char period;
};

struct protocol_datatype_login_feedback{
	//unsigned long int timestamp;
	unsigned int timestamp;
	unsigned char result;
};


#endif
