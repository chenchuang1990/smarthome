#include <stdio.h>
#include "bytebuffer.h"
#include "gateway.h"
#include "protocol_datatype.h"
#include "protocol.h"

unsigned int protocol_encode_del_device_feedback(unsigned char * buf, struct protocol_datatype_del_device * del_device,unsigned char  result){
	unsigned char * p = buf;
	bytebuffer_writebyte(&p,PROTOCOL_START_FLAG);
	bytebuffer_writeword(&p,0);
	bytebuffer_writeword(&p,DEL_DEVICE_FEEDBACK);
	bytebuffer_writedword(&p,del_device->serialnum);
	bytebuffer_writequadword(&p, del_device->ieee);
	bytebuffer_writebyte(&p,result);

	unsigned char *p1 = buf + 1;				
	bytebuffer_writeword(&p1,p-buf+2);

	unsigned checksum = protocol_checksum(buf,p-buf);
	bytebuffer_writebyte(&p,checksum);
	bytebuffer_writebyte(&p,PROTOCOL_END_FLAG);

	return p-buf;
}

