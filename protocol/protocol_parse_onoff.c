#include "bytebuffer.h"
#include "protocol_cmdtype.h"

//设备开关
//-------
//标识位 1 byte "0xCE"
//消息长度 2 bytes 标识位(含)字节长度长度
//消息ID 2 bytes 0x80 0x10
//SerialNum 4 bytes 序列号
//DeviceID 8 bytes 设备ID
//Endpoint 1 byte 
//OnOff 1 byte (0 off 1 on)
//校验码 (从开头到校验位前一位的^)
//标识位 1 byte
//-------
//
//

void protocol_parse_onoff(unsigned char * buf, unsigned short len, struct protocol_cmdtype_onoff_ieee * onoff){
	const unsigned char * p = buf; 
	bytebuffer_skipbytes(&p, 5);
	bytebuffer_readdword(&p, &onoff->serialnum);
	bytebuffer_readquadword(&p, &onoff->ieee); 
	bytebuffer_readbyte(&p, &onoff->endpoint);
	bytebuffer_readbyte(&p, &onoff->onoff);
}
