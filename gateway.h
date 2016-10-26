#ifndef __CE_GATEWAY_H_H
#define __CE_GATEWAY_H_H

#include <time.h>
#include "list.h"
#include "mtZdo.h"
#include "protocol_cmdtype.h"


#define MAXNAMELEN 256
#define MAXEPCOUNT 64

#define DEVICE_NULL 0
//#define DEVICE_APP_DEL 1
#define DEVICE_APP_ADD	1
#define DEVICE_SEND_ATTR 2
#define DEVICE_GET_ATTR 4
#define DEVICE_SEND_ACTIVEEP 8
#define DEVICE_SEND_SIMPLEDESC 16
#define DEVICE_GET_ACTIVEEP 32
#define DEVICE_GET_SIMPLEDESC 64
#define DEVICE_ACTIVE 128
#define DEVICE_SS_SEND_ALARM_NOTIFICATION 256
#define DEVICE_SS_SEND_NO_ALARM_NOTIFICATION 512
#define DEVICE_LEAVE_NET 1024 
//#define DEVICE_NONEED_CHECK	(1 << 11)

/*NOTE: when you want to add a new member*/
struct simpledesc{
	SimpleDescRspFormat_t simpledesc; 
	unsigned short zonetype; // used for ss device
	struct protocol_cmdtype_arm arm; // used for ss device
	unsigned char zcl_transnum;		//used for ss device
	unsigned char device_state;	//used for outlet and shade device
}__attribute__((packed));

struct endpoint{
	struct simpledesc simpledesc;
	struct list_head list;
	unsigned char record;
};

struct device{
	unsigned long long ieeeaddr;
	unsigned short shortaddr;
	char devicename[MAXNAMELEN];
	unsigned int status;
	unsigned char zclversion;
	unsigned char applicationversion;
	unsigned char stackversion;
	unsigned char hwversion;
	char manufacturername[33];
	char modelidentifier[33];
	char datecode[17];
	unsigned char powersource;

	unsigned int endpoint_zonetype[8];

	unsigned char epcursor;
	ActiveEpRspFormat_t activeep;
	struct list_head eplisthead;
	struct list_head list;
	time_t timestamp;
	int accesscnt;
};

struct gateway{
	unsigned long long gatewayid;
	char gatewayname[MAXNAMELEN];
	unsigned short devicecount;
	unsigned char boxversion;
	unsigned char protocolversion;
	struct list_head head;
};

// endpoint 
struct endpoint * endpoint_create(struct simpledesc * simpledesc);
void endpoint_destroy(struct endpoint * ep);
unsigned char endpoint_check_arm(struct endpoint * ep, unsigned char hour, unsigned char minute);

// device
struct device * device_create(unsigned long long deviceieee, unsigned short shortaddr);
struct device * device_create2(unsigned long long ieee, unsigned short shortaddr,char * name, unsigned char status,
		unsigned char zclversion, unsigned char applicationversion, 
		unsigned char stackversion, unsigned char hwversion,
		char * manufacturername, char * modelidentifier, char * datecode);

void device_addendpoint(struct device * d, struct endpoint * ep);
unsigned char device_getepcount(struct device * d);
void device_destroy(struct device * d);
void device_setep(struct device * d, ActiveEpRspFormat_t * activeep);
static inline void device_increase(struct device * d){
	d->epcursor++;
}

void device_clear_status(struct device * d, unsigned int status);

void device_set_status(struct device * d, unsigned int status);

static inline int device_check_status(struct device * d, unsigned int status){
	return d->status & status;
}

void device_set_zonetype(struct device *d, unsigned char endpoint, unsigned short zonetype);

int device_get_index(struct device *d, unsigned char endpoint);
int device_has_enpoint(struct device * d, unsigned char endpoint);

struct endpoint * device_get_endpoint(struct device * d, unsigned char endpoint);

// gateway
struct gateway * getgateway();
void gateway_init(struct gateway * gw,unsigned long long gatewayid, char * gatewayname, unsigned char boxversion, unsigned char protocolversion);
void gateway_adddevice(struct gateway * gw, struct device * d);
void gateway_deldevice(struct gateway * gw, struct device *d);
struct device * gateway_getdevice(struct gateway * gw, unsigned long long ieee);
struct device * gateway_getdevice_shortaddr(unsigned short shortaddr);


int gateway_update_device_networkaddr(unsigned long long ieee, unsigned short shortaddr);
//struct endpoint * gateway_get_endpoint_incluster(unsigned long long ieee, unsigned short clusterid);
struct endpoint * gateway_get_endpoint(unsigned long long ieee, unsigned char endpoint);

struct device * gateway_get_warning_device();
struct endpoint * gateway_get_warning_device_endpoint();
unsigned short gateway_get_active_device_count();
int is_device_deleted(unsigned long long ieee);
#endif
