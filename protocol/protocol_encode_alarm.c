#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include "protocol.h"
#include "zcl_datatype.h"
#include "gateway.h"
#include "zcl_ha.h"
#include "bytebuffer.h"

/*
 * Attribute Value       | Zone Type                  | Alarm1                                    Alarm2
 * 0x0000                | Standard CIE               | System Alarm                              -
 * 0x000d                | Motion sensor              | Intrusion indication Presence indication 
 * 0x0015                | Contact switch             | 1 st portal Open-Close                    2 nd portal Open-Close 
 * 0x0028                | Fire sensor                | Fire indication                           -
 * 0x002a                | Water sensor               | Water overflow indication                 -
 * 0x002b                | Gas sensor                 | CO indication                             Cooking indication 
 * 0x002c                | Personal emergency device  | Fall / Concussion                         Emergency button 
 * 0x002d                | Vibration / Movement sensor| Movement indication                       Vibration 
 * 0x010f                | Remote Control             | Panic                                     Emergency 
 * 0x0115                | Key fob                    | Panic                                     Emergency 
 * 0x021d                | Keypad                     | Panic                                     Emergency 
 * 0x0225                | Standard Warning Device    | -                                         -
 *                       | (see [B5] part 3)          | 
 * Other values < 0x7fff | Reserved                   | 
 * 0x8000-0xfffe         | Reserved for manufacturer  |
 *                       | specific types             |
 * 0xffff                | Invalid Zone Type          | 
 *---------------------------------------------------------------------------------------------------------------
 */
#define STANDARDCIE             0x0000
#define MOTIONSENSOR            0x000d
#define CONTACTSWITCH           0x0015
#define FIRESENSOR              0x0028
#define WATERSENSOR             0x002a
#define GASSENSOR               0x002b
#define PERSONALEMERGENCYDEVICE 0x002c
#define VIBRATIONMOVEMENTSENSOR 0x002d
#define REMOTECONTROL           0x010f
#define KEYFOB                  0x0115
#define KEYPAD                  0x021d
#define STANDARDWARNINGDEVICE   0x0225
#define INVALIDZONETYPE         0xffff

//报警(目前仅针对门磁)
//ce 
//00 1b 
//00 06 
//00 13 7a 00 00 01 56 d3 
//00 00 00 00 56 e1 28 6b 
//02 
//15 00 00 dd ce 
//-------
//标识位 1 byte "0xCE"
//消息长度 2 bytes 标识位(含)字节长度
//消息ID 2 bytes 0x00 0x06
//DeviceID 8 bytes 设备ID(IEEE)
//WarnTime 8 bytes // the number of seconds elapsed since January 1, 1970 UTC.
//EndPoint 1 byte
//DeviceTypeID 2 bytes  // must be 0402 for now  ---目前针对安防门窗磁
//	zonetype 2bytes
//	Alarm1 1 byte
//	Alarm2 1 byte
//校验码 (从开头到校验位前一位的^)
//标识位 1 byte
//
//-------

unsigned int protocol_encode_alarm(unsigned char *buf, struct zclzonechangenotification * notification) {
	unsigned char *p = buf;
	unsigned long long ctime;
	bytebuffer_writebyte(&p,PROTOCOL_START_FLAG);
	bytebuffer_writeword(&p,0);
	bytebuffer_writeword(&p,ALARM);
	bytebuffer_writequadword(&p,notification->ieeeaddr);

	ctime = time(NULL);
	bytebuffer_writequadword(&p,ctime);
	bytebuffer_writebyte(&p, notification->endpoint);
	struct endpoint * ep = gateway_get_endpoint(notification->ieeeaddr, notification->endpoint); 
	assert(ep);
	if(ep){
		bytebuffer_writeword(&p, ep->simpledesc.simpledesc.DeviceID);
		if(ep->simpledesc.simpledesc.DeviceID == ZCL_HA_DEVICEID_IAS_ZONE){
			bytebuffer_writeword(&p, ep->simpledesc.zonetype);
		}
	}


	//bytebuffer_writebyte(&p,notification->zonechangenotification.zonestatus.alarm1);
	//bytebuffer_writebyte(&p,notification->zonechangenotification.zonestatus.alarm2);
	bytebuffer_writeword(&p,notification->zonechangenotification.uszonestatus);

	unsigned char * len = buf + 1;
	bytebuffer_writeword(&len, p - buf + 2);
	unsigned char checksum = protocol_checksum(buf,p-buf);
	bytebuffer_writebyte(&p,checksum);
	bytebuffer_writebyte(&p,PROTOCOL_END_FLAG);

	return p-buf;
}
