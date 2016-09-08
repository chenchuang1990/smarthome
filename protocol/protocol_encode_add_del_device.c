#include <stdio.h>
#include <stdlib.h>
#include "gateway.h"
#include "list.h"
#include "protocol.h"
#include "zcl_ha.h"
#include "bytebuffer.h"

unsigned short find_zonetye_from_device(struct device *d, char endpoint) 
{
#define EPMASK 0xff0000
	int i;
	unsigned short zonetype;
	
	for(i = 0; i < 8; i++) {
		if((d->endpoint_zonetype[i] & EPMASK) >> 16 == endpoint) {
			printf("find_zonetye_from_device::endpoint_zonetype:%x\n", d->endpoint_zonetype[i]);
			break;
		}
	}
	if(i < 8) {
		zonetype = (d->endpoint_zonetype[i] & ~EPMASK);
		d->endpoint_zonetype[i] = 0;
		printf("find_zonetye_from_device::%x\n", zonetype);
		return zonetype;
	}
	printf("find_zonetye_from_device::no match\n");
	return 0;
}

//添加 设备
//ce 
//00 19 
//00 05 
//01 
//00 13 7a 00 00 01 56 d3 
//02 
//01 00 00 
//02 04 02 00 15 
//2c 
//ce 
//-------
//标识位 1 byte "0xCE"
//消息长度 2 bytes 标识位(含)字节长度长度
//消息ID 2 bytes 0x00 0x05
//Action 1 bytes (1 add 0 del)
//DeviceID 8 bytes 设备ID ieee
//DeviceTypeIDCount 1 byte
//	EndPoint 1 byte
//	DeviceTypeID1(2bytes) [zonetype 2 bytes]
//	...
//
//校验码 (从开头到校验位前一位的^)
//标识位 1 byte
//-------
unsigned int protocol_encode_add_del_device(unsigned char * buf, unsigned long long ieeeaddr, unsigned char add) {
	unsigned char * p = buf;
	bytebuffer_writebyte(&p,PROTOCOL_START_FLAG);
	bytebuffer_writeword(&p,0x0000); 
	bytebuffer_writeword(&p,ADDDELDEVICE); 
	bytebuffer_writebyte(&p,add); 
	bytebuffer_writequadword(&p, ieeeaddr);

	

	struct device *d;
	d = gateway_getdevice(getgateway(), ieeeaddr);

	//add report devciename
	unsigned char dname_len = strlen(d->devicename);
	bytebuffer_writebyte(&p, dname_len);
	bytebuffer_writebytes(&p, (unsigned char *)d->devicename, dname_len);

	unsigned char devicetypeidcount = device_getepcount(d);
	bytebuffer_writebyte(&p, devicetypeidcount);

	struct list_head *pos, *n;
	struct endpoint *e;
	list_for_each_safe(pos, n,&d->eplisthead) {
		e=list_entry(pos, struct endpoint, list);
		bytebuffer_writebyte(&p, e->simpledesc.simpledesc.Endpoint);
		bytebuffer_writeword(&p,e->simpledesc.simpledesc.DeviceID);
		if(e->simpledesc.simpledesc.DeviceID == ZCL_HA_DEVICEID_IAS_ZONE){
			/*modifid by cc start*/
			if(0 == e->simpledesc.zonetype) {
				e->simpledesc.zonetype = find_zonetye_from_device(d, e->simpledesc.simpledesc.Endpoint);
			}
			/*modifid by cc end*/
			bytebuffer_writeword(&p, e->simpledesc.zonetype);
		}	
	}
	unsigned int templen = p-buf;
	unsigned char *p1=buf+1;
	bytebuffer_writeword(&p1,templen+2);

	unsigned checksum = protocol_checksum(buf,templen);
	bytebuffer_writebyte(&p,checksum);
	bytebuffer_writebyte(&p,PROTOCOL_END_FLAG);

	return p-buf;
}

