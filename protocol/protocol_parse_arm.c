#include "bytebuffer.h"
#include "protocol_cmdtype.h"

//设备布防/撤防
//-------
//标识位 1 byte "0xCE"
//消息长度 2 bytes 标识位(含)字节长度长度
//消息ID 2 bytes 0x80 0x0F
//DeviceID 8 bytes  设备ID
//SerialNum 4 bytes 序列号
//ArmModel 1 byte 0 arm 1 disarm 2 armtime
//ArmStartTimeHour 1 byte 
//ArmStartTimeMinute 1 byte
//ArmEndTimeHour 1 byte
//ArmEndTimeMinute 1 byte
//校验码 (从开头到校验位前一位的^)
//标识位 1 byte
//-------
unsigned long long protocol_parse_arm(unsigned char * buf, unsigned short len, struct protocol_cmdtype_arm * arm, unsigned int * serialnum, unsigned char * endpoint ){
	const unsigned char * p = buf;
	bytebuffer_skipbytes(&p, 5);
	bytebuffer_readdword(&p, serialnum);
	unsigned long long ieee;
	bytebuffer_readquadword(&p, &ieee);
	bytebuffer_readbyte(&p, endpoint);
	//bytebuffer_readdword(&p, serialnum);
	bytebuffer_readbyte(&p, &arm->armmodel);
	bytebuffer_readbyte(&p, &arm->starthour);
	bytebuffer_readbyte(&p, &arm->startminute);
	bytebuffer_readbyte(&p, &arm->endhour);
	bytebuffer_readbyte(&p, &arm->endminute); 

	return ieee;
}
