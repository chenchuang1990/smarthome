#include "protocol.h"
#include "bytebuffer.h"
#include "protocol_cmdtype.h"
//设备布防/撤防 反馈
//-------
//标识位 1 byte "0xCE"
//消息长度 2 bytes 标识位(含)字节长度长度
//消息ID 2 bytes 0x00 0x0F
//DeviceID 8 bytes  设备ID
//Result 1 byte 0 fail 1 success
//校验码 (从开头到校验位前一位的^)
//标识位 1 byte
//-------
//
unsigned int protocol_encode_arm_feedback(unsigned char * buf, struct protocol_cmdtype_setarm *setarm, unsigned char result)
{
	unsigned char *p = buf;
	bytebuffer_writebyte(&p,PROTOCOL_START_FLAG);
	bytebuffer_writeword(&p,0x0000);
	//bytebuffer_writeword(&p,20);
	bytebuffer_writeword(&p,DEVICE_ARM_FEEDBACK);
	bytebuffer_writedword(&p, setarm->serialnum);
	bytebuffer_writequadword(&p, setarm->ieee);

	bytebuffer_writebyte(&p, setarm->endpoint);
	bytebuffer_writebyte(&p, setarm->arm.armmodel);
	bytebuffer_writebyte(&p, setarm->arm.starthour);
	bytebuffer_writebyte(&p, setarm->arm.startminute);
	bytebuffer_writebyte(&p, setarm->arm.endhour);
	bytebuffer_writebyte(&p, setarm->arm.endminute);
	
	bytebuffer_writebyte(&p, result);

	unsigned char *p1 =  buf + 1;
	bytebuffer_writeword(&p1,p-buf+2);
	
	/*
	unsigned int templen = p-buf;
	unsigned char *p1=buf+1;
	bytebuffer_writeword(&p1,templen+2);
	*/
	
	unsigned char checksum = protocol_checksum(buf,p-buf);
	bytebuffer_writebyte(&p,checksum);
	bytebuffer_writebyte(&p,PROTOCOL_END_FLAG);

	return p-buf;
}
