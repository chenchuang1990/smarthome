
#include "bytebuffer.h"
#include "protocol_cmdtype.h"

//设备点名
//-------
//标识位 1 byte "0xCE"
//消息长度 2 bytes 标识位(含)字节长度长度
//消息ID 2 bytes 0x80 0x0C
//
//SerialID 4 bytes 序列号
//DeviceID 8 bytes 设备ID(IEEE)
//	校验码 (从开头到校验位前一位的^)
//	标识位 1 byte
unsigned long long protocol_parse_identify(unsigned char * buf, unsigned short len, struct protocol_cmdtype_identify * identify){ 
	const unsigned char * p = buf;
	bytebuffer_skipbytes(&p, 5);
	bytebuffer_readdword(&p, &identify->serialnum);
	unsigned long long ieee;
	bytebuffer_readquadword(&p, &ieee);
	unsigned char endpoint;
	bytebuffer_readbyte(&p, &endpoint);
	identify->endpoint = endpoint;

	return ieee;
}
