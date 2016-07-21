#include "bytebuffer.h"
#include "protocol_datatype.h"

//查询设备属性
//-------
//标识位 1 byte "0xCE"
//消息长度 2 bytes 标识位(含)字节长度长度
//消息ID 2 bytes 0x80 0x0B
//
//SerialID 4 bytes 序列号
//DeviceID 8 bytes 设备ID(IEEE)
//校验码 (从开头到校验位前一位的^)
//标识位 1 byte
//-------
//
void protocol_parse_get_device_attr(unsigned char * buf, unsigned short len, struct protocol_datatype_get_device_attr * get_attr){
	const unsigned char * p = buf;
	bytebuffer_skipbytes(&p, 5);
	bytebuffer_readdword(&p, &get_attr->serialnum);
	bytebuffer_readquadword(&p, &get_attr->ieee);
	bytebuffer_readbyte(&p, &get_attr->endpoint);
}

