#include <stdlib.h>
#include "gateway.h"
#include "list.h"
#include "protocol.h"
#include "zcl_ha.h"
#include "bytebuffer.h"
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
unsigned int protocol_encode_add_del_device(unsigned char * buf, unsigned long long ieeeaddr, unsigned char add){
	unsigned char * p = buf;
	bytebuffer_writebyte(&p,PROTOCOL_START_FLAG);
	bytebuffer_writeword(&p,0x0000); 
	bytebuffer_writeword(&p,ADDDELDEVICE); 
	bytebuffer_writebyte(&p,add); 
	bytebuffer_writequadword(&p, ieeeaddr);

	struct device *d;
	d = gateway_getdevice(getgateway(), ieeeaddr);

	unsigned char devicetypeidcount = device_getepcount(d);
	bytebuffer_writebyte(&p, devicetypeidcount);

	struct list_head *pos, *n;
	struct endpoint *e;
	list_for_each_safe(pos, n,&d->eplisthead) {
		e=list_entry(pos, struct endpoint, list);
		bytebuffer_writebyte(&p, e->simpledesc.simpledesc.Endpoint);
		bytebuffer_writeword(&p,e->simpledesc.simpledesc.DeviceID);
		if(e->simpledesc.simpledesc.DeviceID == ZCL_HA_DEVICEID_IAS_ZONE){
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

