#ifndef __PROTOCOL_CMD_HEADER_H_H_
#define __PROTOCOL_CMD_HEADER_H_H_ 
#include "protocol_cmdtype.h"
#include "protocol_datatype.h"

unsigned long long protocol_parse_arm(unsigned char * buf, unsigned short len, struct protocol_cmdtype_arm * arm, unsigned int * serialnum, unsigned char * endpoint); 
unsigned long long protocol_parse_identify(unsigned char * buf, unsigned short len, struct protocol_cmdtype_identify * identify); 
unsigned long long protocol_parse_warning(unsigned char * buf, unsigned short len, struct protocol_cmdtype_warning * warning);
struct protocol_datatype_del_device;
void protocol_parse_del_device( unsigned char * buf, unsigned short len, struct protocol_datatype_del_device * del);
struct protocol_datatype_get_device_attr;
void protocol_parse_get_device_attr(unsigned char * buf, unsigned short len, struct protocol_datatype_get_device_attr * get_attr);
struct protocol_datatype_set_device_name;
void protocol_parse_set_device_name( unsigned char * buf, unsigned short len, struct protocol_datatype_set_device_name * name);
void protocol_parse_onoff(unsigned char * buf, unsigned short len, struct protocol_cmdtype_onoff_ieee * onoff);
void protocol_parse_level_ctrl(unsigned char * buf, unsigned short len, struct protocol_cmdtype_level_ctrl_ieee * level_ctrl_ieee);
void protocol_parse_permit_joining(unsigned char * buf, unsigned short len, struct protocol_cmdtype_permit_joining * joining);
void protocol_parse_config_reporting(unsigned char * buf, unsigned short len, struct protocol_cmdtype_config_reporting * cfg_report);
void protocol_parse_get_device_status(unsigned char * buf, unsigned short len, struct protocol_datatype_online_status * get_status);
void protocol_parse_get_onoff_state(unsigned char *buf, unsigned short len, struct protocol_cmdtype_get_onoff_state *onoff_state);
void protocol_parse_login_feedback(unsigned char *buf, unsigned short len, struct protocol_datatype_login_feedback *feedback);

unsigned int protocol_encode_add_del_device(unsigned char * buf, unsigned long long ieeeaddr, unsigned char add);
struct zclzonechangenotification;
unsigned int protocol_encode_alarm(unsigned char *buf, struct zclzonechangenotification * notification);
unsigned int protocol_encode_heart(unsigned char *buf);
unsigned int protocol_encode_login(unsigned char *buf);
unsigned int protocol_encode_arm_feedback(unsigned char * buf, unsigned int serial_num, unsigned long long ieee, unsigned char result);
unsigned int protocol_encode_add_del_device(unsigned char * buf, unsigned long long ieeeaddr, unsigned char add);
unsigned int protocol_encode_del_device_feedback(unsigned char * buf, struct protocol_datatype_del_device * del_device,unsigned char  result);
unsigned int protocol_encode_deviceattr(unsigned char * buf, struct protocol_datatype_get_device_attr * get_attr);
unsigned int protocol_encode_set_name_feedback(unsigned char * buf, struct protocol_datatype_set_device_name * device_name ,unsigned char result);
unsigned int protocol_encode_general_response(unsigned char *buf, struct zclgeneraldefaultresponse * response);
unsigned int protocol_encode_onoff_response(unsigned char *buf, struct zclgeneraldefaultresponse * response);
unsigned int protocol_encode_readonoff_response(unsigned char *buf, struct zclreadonoffrsp * response); 
unsigned int protocol_encode_level_response(unsigned char *buf, struct zclgeneraldefaultresponse *response);
unsigned int protocol_encode_warning_response(unsigned char *buf, struct zclgeneraldefaultresponse *response);
unsigned int protocol_encode_report_status(unsigned char *buf, struct zclbasicstatus* status);
//unsigned int protocol_encode_onoff_report(unsigned char *buf, struct zclonoffreport* status);
#endif
