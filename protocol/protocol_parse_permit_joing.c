#include "bytebuffer.h"
#include "protocol_cmdtype.h"
#include "protocol_datatype.h"

//允许入网
//-------
//标识位 1 byte "0xCE"
//消息长度 2 bytes 标识位(含)字节长度长度
//消息ID 2 bytes 0x80 0x10
//SerialNum 4 bytes 序列号
//允许入网时间 1 byte (单位：s)
//校验码 (从开头到校验位前一位的^)
//标识位 1 byte
//-------
//
//

void protocol_parse_permit_joining(unsigned char * buf, unsigned short len, struct protocol_cmdtype_permit_joining * joining){
	const unsigned char * p = buf; 
	bytebuffer_skipbytes(&p, 5);
	bytebuffer_readdword(&p, &joining->serialnum);
	bytebuffer_readbyte(&p, &joining->joing_time);
}

#define SEC_KEY_LEN 16
unsigned char *recv_reportable_change( uint8 dataType, unsigned char *s, unsigned char *d )
{
	unsigned short len;
	unsigned long long temp64;

	if ( s == NULL )
	{
		return NULL;
	}

	switch ( dataType )
	{
		case ZCL_DATATYPE_DATA8:
		case ZCL_DATATYPE_BOOLEAN:
		case ZCL_DATATYPE_BITMAP8:
		case ZCL_DATATYPE_INT8:
		case ZCL_DATATYPE_UINT8:
		case ZCL_DATATYPE_ENUM8:
			//*buf++ = *((uint8 *)attrData);
			//bytebuffer_readbyte(&s, (unsigned char *)d);
			*(unsigned char *)d = *s;
			break;

		case ZCL_DATATYPE_DATA16:
		case ZCL_DATATYPE_BITMAP16:
		case ZCL_DATATYPE_UINT16:
		case ZCL_DATATYPE_INT16:
		case ZCL_DATATYPE_ENUM16:
		case ZCL_DATATYPE_SEMI_PREC:
		case ZCL_DATATYPE_CLUSTER_ID:
		case ZCL_DATATYPE_ATTR_ID:
			//*buf++ = LO_UINT16( *((uint16*)attrData) );
			//*buf++ = HI_UINT16( *((uint16*)attrData) );
			//bytebuffer_readword(&s, (unsigned short *)d);
			*(unsigned short *)d = bytebuffer_getword(s);
			break;

		case ZCL_DATATYPE_DATA24:
		case ZCL_DATATYPE_BITMAP24:
		case ZCL_DATATYPE_UINT24:
		case ZCL_DATATYPE_INT24:
			//*buf++ = BREAK_UINT32( *((uint32*)attrData), 0 );
			//*buf++ = BREAK_UINT32( *((uint32*)attrData), 1 );
			//*buf++ = BREAK_UINT32( *((uint32*)attrData), 2 );
			//bytebuffer_readdword(&s, (unsigned int *)d);
			*(unsigned int *)d = bytebuffer_getdword(s);
			
			break;

		case ZCL_DATATYPE_DATA32:
		case ZCL_DATATYPE_BITMAP32:
		case ZCL_DATATYPE_UINT32:
		case ZCL_DATATYPE_INT32:
		case ZCL_DATATYPE_SINGLE_PREC:
		case ZCL_DATATYPE_TOD:
		case ZCL_DATATYPE_DATE:
		case ZCL_DATATYPE_UTC:
		case ZCL_DATATYPE_BAC_OID:
			//buf = zcl_buffer_uint32( buf, *((uint32*)attrData) );
			//bytebuffer_readdword(&s, (unsigned int *)d);
			*(unsigned int *)d = bytebuffer_getdword(s);
			break;

		case ZCL_DATATYPE_UINT40:
		case ZCL_DATATYPE_INT40:
			//pStr = (uint8*)attrData;
			//buf = zcl_memcpy( buf, pStr, 5 );
			//bytebuffer_readquadword(&s, &temp64);
			temp64 = bytebuffer_getquadword(s);
			zcl_memcpy(d, (void *)&temp64, 5);
			break;

		case ZCL_DATATYPE_UINT48:
		case ZCL_DATATYPE_INT48:
			//pStr = (uint8*)attrData;
			//buf = zcl_memcpy( buf, pStr, 6 );
			//bytebuffer_readquadword(&s, &temp64);
			temp64 = bytebuffer_getquadword(s);
			zcl_memcpy(d, (void *)&temp64, 6);
			break;

		case ZCL_DATATYPE_UINT56:
		case ZCL_DATATYPE_INT56:
			//pStr = (uint8*)attrData;
			//buf = zcl_memcpy( buf, pStr, 7 );
			//bytebuffer_readquadword(&s, &temp64);
			temp64 = bytebuffer_getquadword(s);
			zcl_memcpy(d, (void *)&temp64, 7);
			break;

		case ZCL_DATATYPE_DOUBLE_PREC:
		case ZCL_DATATYPE_IEEE_ADDR:
		case ZCL_DATATYPE_UINT64:
		case ZCL_DATATYPE_INT64:
			//pStr = (uint8*)attrData;
			//buf = zcl_memcpy( buf, pStr, 8 );
			//bytebuffer_readquadword(&s, (unsigned long long *)d);
			*(unsigned long long *)d = bytebuffer_getquadword(s);
			break;

		case ZCL_DATATYPE_CHAR_STR:
		case ZCL_DATATYPE_OCTET_STR:
			//pStr = (uint8*)attrData;
			//len = *pStr;
			//buf = zcl_memcpy( buf, pStr, len+1 ); // Including length field
			len = *s;
			zcl_memcpy(d, s, len+1);
			break;

		case ZCL_DATATYPE_LONG_CHAR_STR:
		case ZCL_DATATYPE_LONG_OCTET_STR:
			//pStr = (uint8*)attrData;
			//len = BUILD_UINT16( pStr[0], pStr[1] );
			//buf = zcl_memcpy( buf, pStr, len+2 ); // Including length field
			//bytebuffer_readword(&s, &len);
			len = bytebuffer_getword(s);
			*(unsigned char *)(d)++ = LO_UINT16(len);
			*(unsigned char *)(d)++ = HI_UINT16(len);
			zcl_memcpy(d, s+2, len);
			break;

		case ZCL_DATATYPE_128_BIT_SEC_KEY:
			//pStr = (uint8*)attrData;
			//buf = zcl_memcpy( buf, pStr, SEC_KEY_LEN );
			zcl_memcpy(d, s, SEC_KEY_LEN);
			break;

		case ZCL_DATATYPE_NO_DATA:
		case ZCL_DATATYPE_UNKNOWN:
			// Fall through

		default:
			break;
	}

	return d;
}

void protocol_parse_config_reporting(unsigned char * buf, unsigned short len, struct protocol_cmdtype_config_reporting * cfg_report){
	const unsigned char * p = buf; 
	int i;
	bytebuffer_skipbytes(&p, 5);
	bytebuffer_readdword(&p, &cfg_report->serialnum);
	bytebuffer_readquadword(&p, &cfg_report->ieee);
	bytebuffer_readword(&p, &cfg_report->clusterid);
	bytebuffer_readbyte(&p, &cfg_report->endpoint);
	bytebuffer_readbyte(&p, &cfg_report->attr_num);
	for(i = 0; i < cfg_report->attr_num; i++) {
		bytebuffer_readbyte(&p, &cfg_report->attr_list[i].direction);
		bytebuffer_readword(&p, &cfg_report->attr_list[i].attr_id);
		bytebuffer_readbyte(&p, &cfg_report->attr_list[i].data_type);		
		bytebuffer_readword(&p, &cfg_report->attr_list[i].min_interval);
		bytebuffer_readword(&p, &cfg_report->attr_list[i].max_interval);
		bytebuffer_readword(&p, &cfg_report->attr_list[i].timeout_period);
		if(zclAnalogDataType(cfg_report->attr_list[i].data_type)) {
			int left_len = len - (p + 2 - buf);
			cfg_report->attr_list[i].reprotable_change = (unsigned char *)malloc(left_len);
			recv_reportable_change(cfg_report->attr_list[i].data_type, (unsigned char *)p, cfg_report->attr_list[i].reprotable_change);
		}
	}
}

void protocol_parse_get_device_status(unsigned char * buf, unsigned short len, struct protocol_datatype_online_status *get_status){
	const unsigned char * p = buf;
	bytebuffer_skipbytes(&p, 5);
	bytebuffer_readdword(&p, &get_status->serialnum);
	bytebuffer_readquadword(&p, &get_status->ieee);
	bytebuffer_readbyte(&p, &get_status->period);
}
void protocol_parse_read_state_cmd(unsigned char *buf, unsigned short len, struct protocol_cmdtype_read_state *read_state)
{
	const unsigned char * p = buf;
	bytebuffer_skipbytes(&p, 5);
	bytebuffer_readdword(&p, &read_state->serialnum);
	bytebuffer_readquadword(&p, &read_state->ieee);
	bytebuffer_readbyte(&p, &read_state->endpoint);
}


void protocol_parse_login_feedback(unsigned char *buf, unsigned short len, struct protocol_datatype_login_feedback *feedback)
{
	const unsigned char * p = buf;
	bytebuffer_skipbytes(&p, 11);
	bytebuffer_readdword(&p, &feedback->timestamp);
	bytebuffer_readbyte(&p, &feedback->result);
}

void protocol_parse_app_login(unsigned char *buf, unsigned short len, unsigned long long *mac)
{
	const unsigned char * p = buf;
	bytebuffer_skipbytes(&p, 5);
	bytebuffer_readmac(&p, mac);
}


