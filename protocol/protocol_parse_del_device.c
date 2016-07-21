#include "bytebuffer.h"
#include "protocol_datatype.h"

//删除设备
//-------
//标识位 1 byte "0xCE"
//消息长度 2 bytes 标识位(含)字节长度长度
//消息ID 2 bytes 0x80 0x0A
//
//SerialID 4 bytes 序列号
//DeviceID 8 bytes 设备ID(IEEE)
//校验码 (从开头到校验位前一位的^)
//标识位 1 byte
//-------
//
void protocol_parse_del_device( unsigned char * buf, unsigned short len, struct protocol_datatype_del_device * del){
	const unsigned char * p = buf;
	bytebuffer_skipbytes(&p, 5);
	bytebuffer_readdword(&p, &del->serialnum);
	bytebuffer_readquadword(&p, &del->ieee);
}
