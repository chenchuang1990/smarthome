#include <stdio.h>
#include "sqlitedb.h"
#include "bytebuffer.h"
#include "protocol.h"
#include "protocol_datatype.h"
#include "gateway.h"

unsigned int protocol_encode_deviceattr(unsigned char * buf, struct protocol_datatype_get_device_attr * get_attr){
	unsigned char * p = buf;
	bytebuffer_writebyte(&p,PROTOCOL_START_FLAG);                
	bytebuffer_writeword(&p,0x0000);              
	bytebuffer_writeword(&p,DEVICEATTRIBUTE);
	bytebuffer_writedword(&p,get_attr->serialnum);
	bytebuffer_writequadword(&p, get_attr->ieee);       
	bytebuffer_writebyte(&p, get_attr->endpoint);       
	struct device * d = gateway_getdevice(getgateway(), get_attr->ieee);
	if(d){
		bytebuffer_writeword(&p, d->shortaddr);
		struct endpoint * ep = device_get_endpoint(d, get_attr->endpoint);
		if(ep){ 
			bytebuffer_writeword(&p, ep->simpledesc.simpledesc.ProfileID);
			bytebuffer_writebyte(&p,d->zclversion);
			bytebuffer_writebyte(&p,d->applicationversion);
			bytebuffer_writebyte(&p,d->stackversion);
			bytebuffer_writebyte(&p,d->hwversion);
			unsigned char manufacturernamelen = strlen(d->manufacturername);
			bytebuffer_writebyte(&p, manufacturernamelen);
			bytebuffer_writebytes(&p, (unsigned char *)d->manufacturername,manufacturernamelen);
			unsigned char modelidentifierlen = strlen(d->modelidentifier);
			bytebuffer_writebyte(&p, modelidentifierlen);
			bytebuffer_writebytes(&p, (unsigned char *)d->modelidentifier,modelidentifierlen);
			unsigned char datecodelen = strlen(d->datecode);
			bytebuffer_writebyte(&p, datecodelen);        
			bytebuffer_writebytes(&p, (unsigned char *)d->datecode,datecodelen);
			bytebuffer_writebyte(&p, d->powersource);
		}

	}
	unsigned char *p1=buf+1;
	bytebuffer_writeword(&p1,p-buf+2);

	unsigned checksum = protocol_checksum(buf,p-buf);
	bytebuffer_writebyte(&p,checksum);
	bytebuffer_writebyte(&p,PROTOCOL_END_FLAG);

	return p-buf;
}
