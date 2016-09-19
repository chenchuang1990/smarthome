#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include "protocol.h"
#include "zcl_datatype.h"
#include "gateway.h"
#include "zcl_ha.h"
#include "bytebuffer.h"



unsigned int protocol_encode_general_response(unsigned char *buf, struct zclgeneraldefaultresponse * response) {
	unsigned char *p = buf;
	bytebuffer_writebyte(&p,PROTOCOL_START_FLAG);
	bytebuffer_writeword(&p,0);
	bytebuffer_writeword(&p,DEVICE_RESPONSE);
	bytebuffer_writequadword(&p,response->ieeeaddr);
	bytebuffer_writeword(&p, response->clusterid);
	bytebuffer_writebyte(&p,response->endpoint);
	bytebuffer_writebyte(&p,response->cmd_ind);
	bytebuffer_writebyte(&p,response->status);

	unsigned char * len = buf + 1;
	bytebuffer_writeword(&len, p - buf + 2);
	unsigned char checksum = protocol_checksum(buf,p-buf);
	bytebuffer_writebyte(&p,checksum);
	bytebuffer_writebyte(&p,PROTOCOL_END_FLAG);

	return p-buf;
}

unsigned int protocol_encode_onoff_response(unsigned char *buf, struct zclgeneraldefaultresponse *response) 
{
	unsigned char *p = buf;
	bytebuffer_writebyte(&p,PROTOCOL_START_FLAG);
	bytebuffer_writeword(&p,0);
	bytebuffer_writeword(&p,DEVICE_ONOFF_RSP);
	bytebuffer_writedword(&p, response->serialnum);
	bytebuffer_writequadword(&p,response->ieeeaddr);
	bytebuffer_writebyte(&p,response->endpoint);
	bytebuffer_writebyte(&p,response->cmd_ind);
	bytebuffer_writebyte(&p,response->status);

	unsigned char * len = buf + 1;
	bytebuffer_writeword(&len, p - buf + 2);
	unsigned char checksum = protocol_checksum(buf,p-buf);
	bytebuffer_writebyte(&p,checksum);
	bytebuffer_writebyte(&p,PROTOCOL_END_FLAG);

	return p-buf;
}

unsigned int protocol_encode_readonoff_response(unsigned char *buf, struct zclreadonoffrsp * response) 
{
	unsigned char *p = buf;
	bytebuffer_writebyte(&p,PROTOCOL_START_FLAG);
	bytebuffer_writeword(&p,0);
	bytebuffer_writeword(&p,READ_ONOFF_RSP);
	bytebuffer_writedword(&p, response->serialnum);
	bytebuffer_writequadword(&p,response->ieeeaddr);
	bytebuffer_writebyte(&p, response->endpoint);
	bytebuffer_writebyte(&p,response->state);
	
	unsigned char * len = buf + 1;
	bytebuffer_writeword(&len, p - buf + 2);
	unsigned char checksum = protocol_checksum(buf,p-buf);
	bytebuffer_writebyte(&p,checksum);
	bytebuffer_writebyte(&p,PROTOCOL_END_FLAG);

	return p-buf;
}

unsigned int protocol_encode_level_response(unsigned char *buf, struct zcllevlctldefaultresponse *response) 
{
	unsigned char *p = buf;
	bytebuffer_writebyte(&p,PROTOCOL_START_FLAG);
	bytebuffer_writeword(&p,0);
	bytebuffer_writeword(&p,DEVICE_LEVEL_RSP);
	bytebuffer_writedword(&p, response->serialnum);
	bytebuffer_writequadword(&p,response->ieeeaddr);
	bytebuffer_writebyte(&p,response->endpoint);
	bytebuffer_writebyte(&p,response->device_state);
	bytebuffer_writebyte(&p,response->status);

	unsigned char * len = buf + 1;
	bytebuffer_writeword(&len, p - buf + 2);
	unsigned char checksum = protocol_checksum(buf,p-buf);
	bytebuffer_writebyte(&p,checksum);
	bytebuffer_writebyte(&p,PROTOCOL_END_FLAG);

	return p-buf;
}

unsigned int protocol_encode_readlevel_response(unsigned char *buf, struct zclreadlevelctlrsp * response) 
{
	unsigned char *p = buf;
	bytebuffer_writebyte(&p,PROTOCOL_START_FLAG);
	bytebuffer_writeword(&p,0);
	bytebuffer_writeword(&p,READ_LEVEL_RSP);
	bytebuffer_writedword(&p, response->serialnum);
	bytebuffer_writequadword(&p,response->ieeeaddr);
	bytebuffer_writebyte(&p, response->endpoint);
	bytebuffer_writebyte(&p,response->cur_level);
	
	unsigned char * len = buf + 1;
	bytebuffer_writeword(&len, p - buf + 2);
	unsigned char checksum = protocol_checksum(buf,p-buf);
	bytebuffer_writebyte(&p,checksum);
	bytebuffer_writebyte(&p,PROTOCOL_END_FLAG);

	return p-buf;
}

unsigned int protocol_encode_warning_response(unsigned char *buf, struct zclgeneraldefaultresponse *response) 
{
	unsigned char *p = buf;
	bytebuffer_writebyte(&p,PROTOCOL_START_FLAG);
	bytebuffer_writeword(&p,0);
	bytebuffer_writeword(&p,DEVICE_WARNING_RSP);
	bytebuffer_writedword(&p, response->serialnum);
	bytebuffer_writequadword(&p,response->ieeeaddr);
	bytebuffer_writebyte(&p,response->endpoint);
	bytebuffer_writebyte(&p,response->cmd_ind);
	bytebuffer_writebyte(&p,response->status);

	unsigned char * len = buf + 1;
	bytebuffer_writeword(&len, p - buf + 2);
	unsigned char checksum = protocol_checksum(buf,p-buf);
	bytebuffer_writebyte(&p,checksum);
	bytebuffer_writebyte(&p,PROTOCOL_END_FLAG);

	return p-buf;
}

unsigned int protocol_encode_report_status(unsigned char *buf, struct zclbasicstatus* status) {
	unsigned char *p = buf;
	bytebuffer_writebyte(&p,PROTOCOL_START_FLAG);
	bytebuffer_writeword(&p,0);
	bytebuffer_writeword(&p,STATUS_REPORT);
	bytebuffer_writequadword(&p,status->ieeeaddr);
	bytebuffer_writebyte(&p,status->status);

	unsigned char * len = buf + 1;
	bytebuffer_writeword(&len, p - buf + 2);
	unsigned char checksum = protocol_checksum(buf,p-buf);
	bytebuffer_writebyte(&p,checksum);
	bytebuffer_writebyte(&p,PROTOCOL_END_FLAG);

	return p-buf;
}

#if 0
unsigned int protocol_encode_onoff_report(unsigned char *buf, struct zclonoffreport* status) {
	unsigned char *p = buf;
	bytebuffer_writebyte(&p,PROTOCOL_START_FLAG);
	bytebuffer_writeword(&p,0);
	bytebuffer_writeword(&p,ONOFF_REPORT);
	bytebuffer_writequadword(&p,status->ieeeaddr);
	bytebuffer_writebyte(&p,status->endpoint);
	bytebuffer_writebyte(&p,status->state);

	unsigned char * len = buf + 1;
	bytebuffer_writeword(&len, p - buf + 2);
	unsigned char checksum = protocol_checksum(buf,p-buf);
	bytebuffer_writebyte(&p,checksum);
	bytebuffer_writebyte(&p,PROTOCOL_END_FLAG);

	return p-buf;
}
#endif

