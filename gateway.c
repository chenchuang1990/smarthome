#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "gateway.h"
#include "sqlitedb.h"
#include "toolkit.h"
#include "zcl_ha.h" 

#define min(a,b) a>b?b:a

//static struct gateway gatewayinstance;
struct gateway gatewayinstance;

// ---------------endpoint---------------
struct endpoint * endpoint_create(struct simpledesc * simpledesc){
	struct endpoint * endpoint = (struct endpoint *)malloc(sizeof(struct endpoint));
	memset(endpoint, 0, sizeof(struct endpoint));
	memcpy(&endpoint->simpledesc, simpledesc, sizeof(struct simpledesc));
	INIT_LIST_HEAD(&endpoint->list);

	return endpoint;
}

void endpoint_destroy(struct endpoint * ep){
	list_del(&ep->list);
	free(ep);
}

unsigned char endpoint_check_arm(struct endpoint * ep, unsigned char hour, unsigned char minute){
	//printf("ep->simpledesc.arm.armmodel = %d\n", ep->simpledesc.arm.armmodel);
	if(ep->simpledesc.arm.armmodel == ARM){
		return 1;
	}
	if(ep->simpledesc.arm.armmodel == ARMTIME){ 
		if(toolkit_in_period(ep->simpledesc.arm.starthour, ep->simpledesc.arm.startminute, ep->simpledesc.arm.endhour, ep->simpledesc.arm.endminute, hour, minute)){ 
			return 1;
		}else{
			return 0;
		}
	}
	if(ep->simpledesc.arm.armmodel == DISARM){
		return 0;
	}

	return 0;
}
// ---------------endpoint---------------
//
// ---------------device---------------
struct device * device_create(unsigned long long deviceieee, unsigned short shortaddr){
	struct device * device = (struct device *)malloc(sizeof(struct device)); 
	memset(device, 0, sizeof(struct device));
	INIT_LIST_HEAD(&device->list);
	INIT_LIST_HEAD(&device->eplisthead);
	device->ieeeaddr = deviceieee;
	device->shortaddr = shortaddr;

	return device; 
}

void device_addendpoint(struct device * d, struct endpoint * ep){ 
	unsigned int i;
	for(i = 0; i < 8; i++){ 
		if(d->endpoint_zonetype[i] != 0){
			if((d->endpoint_zonetype[i] >> 16) == ep->simpledesc.simpledesc.Endpoint){
				ep->simpledesc.zonetype = d->endpoint_zonetype[i]&0x00FF;
				d->endpoint_zonetype[i] = 0;
			}
		}
	}

	list_add_tail(&ep->list, &d->eplisthead);
}

void device_setep(struct device * d, ActiveEpRspFormat_t * activeep){
	memcpy(&d->activeep, activeep, sizeof(ActiveEpRspFormat_t));
}

void device_destroy(struct device * d){ 
	struct endpoint * ep;
	struct list_head * pos, *n;
	list_for_each_safe(pos, n, &d->eplisthead){ 
		ep = list_entry(pos, struct endpoint, list); 
		endpoint_destroy(ep);
	}
	list_del(&d->list);
	free(d);
}

struct device * device_create2(unsigned long long ieee,unsigned short shortaddr, char * name, unsigned char status,
		unsigned char zclversion, unsigned char applicationversion, 
		unsigned char stackversion, unsigned char hwversion,
		char * manufacturername, char * modelidentifier, char * datecode){
	struct device * d = device_create(ieee, shortaddr);
	d->status = status;
	d->zclversion = zclversion;
	d->applicationversion = applicationversion;
	d->stackversion = stackversion;
	d->hwversion = hwversion;
	unsigned char len = strlen(manufacturername);
	memcpy(d->manufacturername, manufacturername, len);
	d->manufacturername[len] = 0;

	len = strlen(modelidentifier);
	memcpy(d->modelidentifier, modelidentifier, len);
	d->modelidentifier[len] = 0;

	len = strlen(datecode);
	memcpy(d->datecode, datecode, len);
	d->datecode[len] = 0;

	len = strlen(name);
	memcpy(d->devicename, name, len);
	d->devicename[len] = 0;

	return d;
}

unsigned char device_getepcount(struct device * d){
	unsigned char count = 0;
	struct list_head * pos, *n;
	list_for_each_safe(pos, n, &d->eplisthead){ 
		count++;
	}

	return count;
}

/*added by cc*/
int is_device_deleted(unsigned long long ieee)
{
	struct device *dev = gateway_getdevice(getgateway(), ieee);
	if(dev && (!device_check_status(dev, DEVICE_APP_DEL)))
		return 0;
	else
		return 1;
}

void device_clear_status(struct device * d, unsigned int status) { 
	d->status &= ~status;
	sqlitedb_update_device_status(d);
}

void device_set_status(struct device * d, unsigned int status) { 
	d->status |= status;
	sqlitedb_update_device_status(d);
}

struct endpoint * _device_get_enpint(struct device *d,  unsigned char endpoint){ 
	struct list_head *pos, *n;
	struct endpoint * ep;
	list_for_each_safe(pos, n, &d->eplisthead){
		ep = list_entry(pos, struct endpoint, list); 
		if(ep && (ep->simpledesc.simpledesc.Endpoint == endpoint)){
			return ep;
		}
	}

	return NULL;
}

void device_set_zonetype(struct device *d, unsigned char endpoint, unsigned short zonetype){
	struct endpoint * ep = _device_get_enpint(d, endpoint);
	if(ep){
		ep->simpledesc.zonetype = zonetype;
		sqlitedb_update_device_endpoint_zonetype(d, endpoint, zonetype);
	}else{ 
		unsigned char i;
		for(i = 0; i < 8; i++){
			if(d->endpoint_zonetype[i]==0){
				d->endpoint_zonetype[i] = (endpoint << 16)+zonetype;
				break;
			}
		}
	}
}

int device_get_index(struct device *d, unsigned char endpoint){
	int result = -1;
	struct list_head *pos, *n;
	struct endpoint * ep;
	list_for_each_safe(pos, n, &d->eplisthead){
		result++;
		ep = list_entry(pos, struct endpoint, list); 
		if(ep && (ep->simpledesc.simpledesc.Endpoint == endpoint)){
			return result;
		}
	}

	return -1;
}

int device_has_enpoint(struct device * d, unsigned char endpoint){
	struct list_head *pos, *n;
	struct endpoint * ep;
	list_for_each_safe(pos, n, &d->eplisthead){
		ep = list_entry(pos, struct endpoint, list);
		if(ep && (ep->simpledesc.simpledesc.Endpoint == endpoint)){
			return 1;
		}
	}

	return 0;
}

struct endpoint * device_get_endpoint(struct device * d, unsigned char endpoint) {
	struct list_head *pos, *n; 
	struct endpoint * ep;
	list_for_each_safe(pos, n, &d->eplisthead){
		ep = list_entry(pos, struct endpoint, list);
		if(ep && (ep->simpledesc.simpledesc.Endpoint == endpoint)){
			return ep;
		}
	}

	return NULL;
}

void device_set_short_addr(struct device * d, unsigned short shortaddr){
	struct endpoint * ep;
	struct list_head * pos, * n;
	list_for_each_safe(pos, n, &d->eplisthead){ 
		ep = list_entry(pos, struct endpoint, list);
		if(ep)
			ep->simpledesc.simpledesc.NwkAddr = shortaddr;
	}
}

// ---------------device---------------
//
// ---------------gateway---------------

struct gateway * getgateway(){
	return &gatewayinstance;
}

void gateway_init(struct gateway * gw,unsigned long long gatewayid, char * gatewayname, unsigned char boxversion, unsigned char protocolversion){ 
	gw->gatewayid = gatewayid;
	memset(gw->gatewayname, 0, MAXNAMELEN);
	memcpy(gw->gatewayname, gatewayname, min(strlen(gatewayname), MAXNAMELEN-1));
	gw->boxversion = boxversion;
	gw->protocolversion = protocolversion;
	INIT_LIST_HEAD(&gw->head);
}

void gateway_adddevice(struct gateway * gw, struct device * d){
	list_add_tail(&d->list, &gw->head);
}

void gateway_deldevice(struct gateway * gw, struct device *d){
	//list_del(&d->list);
	device_destroy(d);
}

struct device * gateway_getdevice(struct gateway * gw, unsigned long long ieee){
	struct device * d;
	struct list_head * pos, *n;
	list_for_each_safe(pos, n, &gw->head){ 
		d = list_entry(pos, struct device, list); 
		if(d->ieeeaddr == ieee){
			return d;
		}
	}

	return NULL;
}

struct device * gateway_getdevice_shortaddr(unsigned short shortaddr){
	struct device * d;
	struct list_head * pos, *n;
	list_for_each_safe(pos, n, &gatewayinstance.head){ 
		d = list_entry(pos, struct device, list); 
		if(d->shortaddr == shortaddr){
			return d;
		}
	}

	return NULL;
}

int gateway_update_device_networkaddr(unsigned long long ieee, unsigned short shortaddr){
	struct device * d = gateway_getdevice(&gatewayinstance, ieee);
	if(d){
		device_set_short_addr( d, shortaddr);
	}

	return 0;
}

struct endpoint * gateway_get_endpoint_incluster(unsigned long long ieee, unsigned short clusterid){
	struct device * d = gateway_getdevice(&gatewayinstance, ieee);
	if(d){
		struct list_head * pos, *n;
		struct endpoint * ep;
		unsigned char i;
		list_for_each_safe(pos, n, &d->eplisthead){
			ep = list_entry(pos, struct endpoint, list); 
			if(ep) {
				for(i = 0; i < ep->simpledesc.simpledesc.NumInClusters; i++){ 
					if(ep->simpledesc.simpledesc.InClusterList[i] == clusterid){
						return ep;
					}
				}
			}

		}
	}

	return NULL;
}

struct endpoint * gateway_get_endpoint(unsigned long long ieee, unsigned char endpoint){
	struct device * d = gateway_getdevice(&gatewayinstance, ieee);
	struct endpoint * dstep = NULL;
	if(d){
		dstep = device_get_endpoint(d, endpoint);
	}

	return dstep;
}

struct endpoint * gateway_get_warning_device_endpoint(){ 
	struct endpoint * ep;
	struct list_head *eppos, *epn;
	struct device * d;
	struct list_head *pos, *n;
	list_for_each_safe(pos, n, &gatewayinstance.head){
		d = list_entry(pos, struct device, list); 
		list_for_each_safe(eppos, epn, &d->eplisthead){
			ep = list_entry(eppos, struct endpoint, list);
			if(ep && (ep->simpledesc.simpledesc.DeviceID == ZCL_HA_DEVICEID_IAS_WARNING_DEVICE)){
				return ep;
			}
		}
	}

	return NULL;
}

struct device * gateway_get_warning_device(){
	struct endpoint * ep;
	struct list_head *eppos, *epn;
	struct device * d;
	struct list_head *pos, *n;
	list_for_each_safe(pos, n, &gatewayinstance.head){
		d = list_entry(pos, struct device, list); 
		list_for_each_safe(eppos, epn, &d->eplisthead){
			ep = list_entry(eppos, struct endpoint, list);
			if(ep && (ep->simpledesc.simpledesc.DeviceID == ZCL_HA_DEVICEID_IAS_WARNING_DEVICE)){
				return d;
			}
		}
	}

	return NULL;
}

unsigned short gateway_get_active_device_count(){
	unsigned short active_device_count = 0;
	struct device * d;
	struct list_head *pos, *n;
	list_for_each_safe(pos, n, &gatewayinstance.head){
		d = list_entry(pos, struct device, list); 
		if((!device_check_status(d, DEVICE_APP_DEL)) && (!device_check_status(d, DEVICE_LEAVE_NET))){
			active_device_count++;
		}
	}

	return active_device_count;
}

// ---------------gateway---------------
