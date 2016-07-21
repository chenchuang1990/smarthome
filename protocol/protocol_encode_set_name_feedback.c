#include <stdio.h>
#include <time.h>
#include <assert.h>
#include "bytebuffer.h"
#include "gateway.h"
#include "protocol.h"
#include "protocol_datatype.h"

unsigned int protocol_encode_set_name_feedback(unsigned char * buf, struct protocol_datatype_set_device_name * device_name ,unsigned char result){
	unsigned char * p = buf;
	bytebuffer_writebyte(&p,PROTOCOL_START_FLAG);                  
	bytebuffer_writeword(&p,0);                
	bytebuffer_writeword(&p,DEVICE_SET_NAME_FEEDBACK);   
	bytebuffer_writedword(&p,device_name->serialnum);
	bytebuffer_writequadword(&p, device_name->ieee);
	bytebuffer_writebyte(&p,result);        
	unsigned char devicenamelen = strlen(device_name->name);
	bytebuffer_writebyte(&p, devicenamelen);
	bytebuffer_writebytes(&p, (unsigned char *)device_name->name, devicenamelen);
	unsigned char *p1=buf+1;                
	bytebuffer_writeword(&p1,p-buf+2);    
	unsigned checksum = protocol_checksum(buf,p-buf);
	bytebuffer_writebyte(&p,checksum);
	bytebuffer_writebyte(&p,PROTOCOL_END_FLAG);  

	return p-buf;           
}

