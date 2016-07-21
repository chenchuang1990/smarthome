
#include "protocol.h"
#include "gateway.h"
#include "bytebuffer.h"
//心跳
//CE 
//00 0D 
//00 02 
//AC 00 00 00 00 CA 
//A7 
//CE
//-------
//标识位 1 byte "0xCE"
//消息长度 2 bytes 标识位(含)字节长度长度
//消息ID 2 bytes  0x00 0x02
//DeviceID 8 bytes 设备ID(IEEE)
//校验码 (从开头到校验位前一位的^)
//标识位 1 byte
//
//-------
unsigned int protocol_encode_heart(unsigned char *buf) {
	struct gateway * gw = getgateway();
	unsigned char *p = buf;
	bytebuffer_writebyte(&p,PROTOCOL_START_FLAG);
	bytebuffer_writeword(&p,0x0D);
	bytebuffer_writeword(&p,HEART);
	bytebuffer_writemac(&p,gw->gatewayid);

	unsigned int templen = p-buf;
	unsigned char checksum = protocol_checksum(buf,templen);
	bytebuffer_writebyte(&p,checksum);
	bytebuffer_writebyte(&p,PROTOCOL_END_FLAG);

	return p-buf;
} 
