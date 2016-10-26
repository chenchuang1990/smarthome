#include <stdio.h>
#include <stdlib.h>
#include "zcl_ha.h"
#include "protocol.h"
#include "gateway.h"
#include "bytebuffer.h"
//登录
//ce 00 21 00 01 00 0c 29 a5 26 48 00 00 00 13 7a 00 00 01 56 d3 00 02 01 00 00 02 04 02 00 15 ff ce 
//-------
//标识位 1 byte "0xCE"
//消息长度 2 bytes 标识位(含)字节长度长度
//消息ID 2 bytes 0x00 0x01
//GatewayID 6 bytes
//GatewayNameLength 1 byte
//GatewayName xbytes
//BoxVersion 1 byte
//ProtocolVersion 1 byte
//DeviceCount 2 bytes    // 设备列表
//DeviceOne:
//	DeviceID 8 bytes 设备ID(IEEE)
//	DeviceNameLen 1 byte 设备名称长度
//	DeviceName x bytes 设备名称
//	DeviceTypeIDCount 1 byte
//	EndPoint 1 byte
//	DeviceTypeID1 2 bytes[zonetype 2bytes]
//	...        
//校验码 (从开头到校验位前一位的^)
//标识位 1 byte
//
//-------
unsigned int protocol_encode_login(unsigned char *buf, int match) {
	struct gateway * gw = getgateway();
	unsigned char *p = buf;
	
	bytebuffer_writebyte(&p,PROTOCOL_START_FLAG);
	bytebuffer_writeword(&p,0x0000);
	bytebuffer_writeword(&p,LOGIN);
	if(match) {
		bytebuffer_writemac(&p, gw->gatewayid); 
		unsigned char gateway_name_len = strlen(gw->gatewayname);
		bytebuffer_writebyte(&p, gateway_name_len);
		bytebuffer_writebytes(&p, (unsigned char *)gw->gatewayname, gateway_name_len);
		bytebuffer_writebyte(&p, gw->boxversion);
		bytebuffer_writebyte(&p, gw->protocolversion);
		unsigned short active_device_count = gateway_get_active_device_count();
		bytebuffer_writeword(&p,active_device_count);

		struct list_head *pos, *n;
		struct device *d;
		list_for_each_safe(pos, n,&gw->head)
		{
			d=list_entry(pos, struct device, list);
			//if((!device_check_status(d, DEVICE_APP_DEL)) && (!device_check_status(d, DEVICE_LEAVE_NET))){
			if(device_check_status(d, DEVICE_APP_ADD) && (!device_check_status(d, DEVICE_LEAVE_NET))){
				bytebuffer_writequadword(&p, d->ieeeaddr);
				unsigned char devicenamelen = strlen(d->devicename);
				bytebuffer_writebyte(&p, devicenamelen);
				bytebuffer_writebytes(&p, (unsigned char *)d->devicename,devicenamelen);
				//unsigned char modelidlen = strlen(d->modelidentifier);
				//bytebuffer_writebyte(&p, modelidlen);
				//bytebuffer_writebytes(&p, (unsigned char *)d->modelidentifier, modelidlen);
				unsigned char devicetypeidcount = device_getepcount(d);
				bytebuffer_writebyte(&p, devicetypeidcount);

				struct list_head *pos1, *n1;
				struct endpoint *e;
				list_for_each_safe(pos1, n1,&d->eplisthead) {
					e=list_entry(pos1, struct endpoint, list);
					bytebuffer_writebyte(&p, e->simpledesc.simpledesc.Endpoint);
					bytebuffer_writeword(&p,e->simpledesc.simpledesc.DeviceID);
					if(e->simpledesc.simpledesc.DeviceID == ZCL_HA_DEVICEID_IAS_ZONE) {
						bytebuffer_writeword(&p,e->simpledesc.zonetype);
					}
					else if((e->simpledesc.simpledesc.DeviceID == ZCL_HA_DEVICEID_MAINS_POWER_OUTLET) || 
							(e->simpledesc.simpledesc.DeviceID == ZCL_HA_DEVICEID_ON_OFF_OUTPUT) ||
										(e->simpledesc.simpledesc.DeviceID == ZCL_HA_DEVICEID_SHADE)) {
						//printf("endpoint:%d\n", e->simpledesc.simpledesc.Endpoint);
						//printf("device state:%d\n", e->simpledesc.device_state);
						bytebuffer_writebyte(&p, e->simpledesc.device_state);
					}
				}
			}
		}	
	}
	else {
		bytebuffer_writebyte(&p, match);
	}
	
	unsigned int templen = p-buf;
	unsigned char *p1=buf+1;
	bytebuffer_writeword(&p1,templen+2);

	unsigned checksum = protocol_checksum(buf,templen);
	bytebuffer_writebyte(&p,checksum);
	bytebuffer_writebyte(&p,PROTOCOL_END_FLAG);

	return p-buf;
}
