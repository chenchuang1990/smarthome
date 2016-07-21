#include "bytebuffer.h"
#include "protocol_datatype.h"

//设置设备名称
//-------
//标识位 1 byte "0xCE"
//消息长度 2 bytes 标识位(含)字节长度长度
//消息ID 2 bytes 0x80 0x08
//
//SerialID 4 bytes 序列号
//DeviceID 8 bytes 设备ID(IEEE)
//DeviceNameLen 1 byte
//DeviceName x bytes
//校验码 (从开头到校验位前一位的^)
//标识位 1 byte
//
//-------
//
void protocol_parse_set_device_name( unsigned char * buf, unsigned short len, struct protocol_datatype_set_device_name * name){
	const unsigned char * p = buf;
	bytebuffer_skipbytes(&p, 5);
	bytebuffer_readdword(&p, &name->serialnum);
	bytebuffer_readquadword(&p, &name->ieee);
	unsigned char namelen;
	bytebuffer_readbyte(&p, &namelen);
	bytebuffer_readbytes(&p, name->name, namelen);
	name->name[namelen] = 0;
}
