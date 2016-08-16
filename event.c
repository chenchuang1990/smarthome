#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>

#include "connection.h"
#include "protocol.h"
#include "toolkit.h"
#include "eventhub.h" 
#include "cetimer.h"
#include "socket.h"
#include "termcontrol.h" 
#include "gateway.h"
#include "list.h"
#include "zcl_datatype.h"
#include "innercmd.h"
#include "bytebuffer.h" 
#include "protocol_cmd_header.h"
#include "protocol_cmdtype.h"
#include "protocol_datatype.h"
#include "zcl_down_cmd.h"
#include "sqlitedb.h"
#include "zcl_ss.h"
#include "zcl_general.h"

//#define DEBUG_EP 1
//#define DEBUG

extern int g_main_to_znp_write_fd;
extern struct connection * g_serverconn;
time_t login_time;

void event_send_warning(struct endpoint * wd_ep, unsigned long long warning_device_ieee, unsigned char cmdid,unsigned char warning_mode);

void event_accept(int fd){
	struct connection * c = freeconnlist_getconn();
	connection_init(c, fd, CONNSOCKETCLIENT);
	connrbtree_insert(c);
}


int _check_command(unsigned char * buffer, int buflen, unsigned char command){
	int i = 0;
	for(; i < buflen; i++){
		if(buffer[i] == command){
			return 1;
		}
	}

	return 0;
}

static void print_hex(unsigned char *addr, int len)
{
#ifdef DEBUG
	int i;
	for(i = 0; i < len; i++) 
		printf("%02x ", addr[i]);
	printf("\n");
#endif
}

extern int access_period;

void event_recvmsg(struct eventhub * hub, int fd, unsigned char * buf, int buflen)
{
	//	fprintf(stdout, "recv ");
	//	toolkit_printbytes(buf, buflen);
	struct connection * c = connrbtree_getconn(fd);
	if(c && connection_gettype(c) == CONNSOCKETCMD) { 
		if( _check_command(buf, buflen, CECHECK[0])){ //maybe "C" connect "B"
			time_t t = time(NULL);
			connlist_checkstatus(t);
		}
		if( _check_command(buf, buflen, HEARTBEAT[0])){
			unsigned char heart_buf[255];
			unsigned int hbuflen;
			hbuflen = protocol_encode_heart(heart_buf);
			broadcast(heart_buf, hbuflen);
		}
		if( _check_command(buf, buflen, CESEND[0])){
			unsigned char buf[2048] = {0}; 
			int serverfd = connlist_getserverfd();
			if(serverfd != -1){
				unsigned int buflen = protocol_encode_login(buf); 
				print_hex(buf, buflen);
				sendnonblocking(serverfd, buf, buflen);				
				login_time = time(NULL);
				printf("[event_recvmsg]login_time:%lx\n", login_time);
			}
		}
	} else if(c && (connection_gettype(c) == CONNSOCKETCLIENT || connection_gettype(c) == CONNSOCKETSERVER)){ 
		c->timestamp = time(NULL);
		connection_put(c, buf, buflen); 

		for(;;){ // in case of receive two packets one time.
			unsigned short messageid = 0;
			int messagelen = protocol_check(c, &messageid);
			if(messageid == ILLEGAL || messageid == HALFPACK){
				break;
			}
			unsigned char buffer[1024] = {0};
			connection_get(c,buffer, messagelen);
			toolkit_printbytes(buffer, messagelen);
			//static time_t login_time;
			switch(messageid) {
				case LOGINFEEDBACK:
					{
						printf("LOGINFEEDBACK\n");
						struct protocol_datatype_login_feedback feedback;
						protocol_parse_login_feedback(buffer, messagelen, &feedback);
						//time_t feed_time = time(NULL);
						struct timeval feed_tv;
						if(-1 == gettimeofday(&feed_tv, NULL)) {
							perror("gettimeofday");
							break;
						}
						printf("feed_tv.tv_sec:%lx\n", feed_tv.tv_sec);
						printf("feedback.timestamp:%x\n", feedback.timestamp);
						printf("login_time:%lx\n", login_time);
						if((feed_tv.tv_sec - login_time < 60) && !feedback.result) {
							feed_tv.tv_sec = (time_t)(feedback.timestamp);
							printf("feed_tv.tv_sec:%lx\n", feed_tv.tv_sec);
							if(-1 == settimeofday(&feed_tv, NULL)) {
								perror("settimeofday");
							}
							system("sync");
						}
					}					
					break;
				case HEARTFEEDBACK:
					break;
				case REQDEVICELIST:
					break;
				case REQOPERATE:
					break;
				case DEVICE_SET_NAME:
					{ 
						struct protocol_datatype_set_device_name set_device_name;
						protocol_parse_set_device_name(buffer, messagelen, &set_device_name);
						unsigned char result = 1;
						if(getgateway()->gatewayid == set_device_name.ieee){ 
							//result = (sqlitedb_update_gatewayname(set_device_name.ieee, set_device_name.name) == 0)?1:0;
							result = sqlitedb_update_gatewayname(set_device_name.ieee, set_device_name.name);
							if(0 == result){
								struct gateway * gw = getgateway();
								if(gw){
									memcpy(gw->gatewayname,set_device_name.name, strlen(set_device_name.name));
									gw->gatewayname[strlen(set_device_name.name)] = 0;
								}
							}
						}else{
							//result = (sqlitedb_update_devicename(set_device_name.ieee, set_device_name.name) == 0)?1:0;
							result = sqlitedb_update_devicename(set_device_name.ieee, set_device_name.name);
							if(0 == result){
								struct device * d = gateway_getdevice(getgateway(), set_device_name.ieee);
								if(d){
									memcpy(d->devicename,set_device_name.name, strlen(set_device_name.name));
									d->devicename[strlen(set_device_name.name)] = 0;
								}
							}
							
						}
						unsigned char sbuf[512] = {0};
						unsigned int slen = protocol_encode_set_name_feedback(sbuf, &set_device_name, result);

						sendnonblocking(fd, sbuf, slen);
						toolkit_printbytes(sbuf, slen);
					}
					break;
				/*case DEVICE_DEL:
					{ 
						printf("DEVICE_DEL\n");
						unsigned char result = 0;
						struct protocol_datatype_del_device del_device;
						protocol_parse_del_device(buffer, messagelen, &del_device);
						struct device * d = gateway_getdevice(getgateway(), del_device.ieee);
						if(d){
							device_set_status(d, DEVICE_APP_DEL);
							result = 1;
						}
						unsigned char buf[128] = {0}; 
						unsigned int len = protocol_encode_del_device_feedback(buf, &del_device, result);

						sendnonblocking(fd, buf,len);
					}
					break;*/
				case DEVICE_DEL:
					{ 
						printf("DEVICE_DEL\n");
						unsigned char result = 1;
						struct protocol_datatype_del_device del_device;
						protocol_parse_del_device(buffer, messagelen, &del_device);
						struct device * d = gateway_getdevice(getgateway(), del_device.ieee);
						if(d){
							device_set_status(d, DEVICE_APP_DEL);
							sqlitedb_delete_device(del_device.ieee);							
							gateway_deldevice(getgateway(), d);	
							result = 0;
						}
						unsigned char sbuf[128] = {0}; 
						unsigned int slen = protocol_encode_del_device_feedback(sbuf, &del_device, result);

						sendnonblocking(fd, sbuf, slen);
					}
					break;
				case DEVICE_ATTR:
					{
					     struct protocol_datatype_get_device_attr get_attr;
					     protocol_parse_get_device_attr(buffer, messagelen, &get_attr); 
						 if(is_device_deleted(get_attr.ieee))
						 	break;
					     unsigned char sbuf[512] = {0};
					     unsigned int slen = protocol_encode_deviceattr(sbuf, &get_attr);

					     sendnonblocking(fd, sbuf, slen); 
				     }
				     break;
				case DEVICE_IDENTIFY:
				     {
					     struct protocol_cmdtype_identify_ieee_cmd identify_ieee_cmd;
					     identify_ieee_cmd.cmdid = PROTOCOL_IDENTIFY;
					     identify_ieee_cmd.identify_ieee.ieee = protocol_parse_identify(buffer, messagelen,&identify_ieee_cmd.identify_ieee.identify); 
					  	 if(is_device_deleted(identify_ieee_cmd.identify_ieee.ieee)) {
							fprintf(stdout, "device has been deleted\n");
							break;
						 }
						 int n = sendnonblocking(g_main_to_znp_write_fd, &identify_ieee_cmd, sizeof(struct protocol_cmdtype_identify_ieee_cmd));
					     fprintf(stdout, "send identify %d %d\n", n, sizeof(struct protocol_cmdtype_identify_ieee_cmd));
				     }
					 break;
				case DEVICE_WARNING:
				     {
					 	 printf("DEVICE_WARNING\n");
					     struct protocol_cmdtype_warning_ieee_cmd warning_ieee_cmd;
					     warning_ieee_cmd.cmdid = PROTOCOL_WARNING;
					     warning_ieee_cmd.warning_ieee.ieee = protocol_parse_warning(buffer, messagelen, &warning_ieee_cmd.warning_ieee.warning);
						 if(is_device_deleted(warning_ieee_cmd.warning_ieee.ieee)) {
							fprintf(stdout, "device has been deleted\n");
							break;
						 }
					     sendnonblocking(g_main_to_znp_write_fd, &warning_ieee_cmd, sizeof(struct protocol_cmdtype_warning_ieee_cmd)); 
						 /*cc's idea*/
						 struct zclgeneraldefaultresponse warning_rsp;

						warning_rsp.cmd_ind = 0;
						warning_rsp.endpoint = warning_ieee_cmd.warning_ieee.warning.endpoint;
						warning_rsp.ieeeaddr = warning_ieee_cmd.warning_ieee.ieee;
						warning_rsp.serialnum = warning_ieee_cmd.warning_ieee.warning.serialnum;
						warning_rsp.status = 0;
						unsigned char sbuf[128] = {0};
						unsigned int sbuflen = protocol_encode_warning_response(sbuf, &warning_rsp);
						 //broadcast(sbuf, sbuflen);
						sendnonblocking(fd, sbuf, sbuflen);
				     }
					 break;
				case APP_LOGIN:
				     {
					     unsigned char sbuf[2048] = {0}; 
					     unsigned int sbuflen = protocol_encode_login(sbuf); 

					     sendnonblocking(fd, sbuf, sbuflen);
						 print_hex(sbuf, sbuflen);
						 login_time = time(NULL);
						 
				     }
					 break;
				case DEVICE_SETARM:
				     {
					 	printf("DEVICE_SETARM\n");
					     struct protocol_cmdtype_arm arm;
					     unsigned int serialnum = 0;
					     unsigned char endpoint = 0;
					     unsigned long long ieee = protocol_parse_arm(buffer, messagelen, &arm, &serialnum, &endpoint);
					     unsigned char result = (unsigned char)sqlitedb_update_device_arm(ieee, endpoint, &arm);
					     //result = (result == 0)?1:0;
					     if(result == 0){ 
						     struct endpoint * ep = gateway_get_endpoint(ieee, endpoint);
							 if(ep) {
						     	memcpy(&ep->simpledesc.arm, &arm, sizeof(struct protocol_cmdtype_arm));
							 }
							 else {
							 	result = 1;
							 }	
					     }
						 //else 
						 //	result = 1;
					     unsigned char sbuf[128] = {0};
					     unsigned int sbuflen = protocol_encode_arm_feedback(sbuf, serialnum, ieee, result);
					     sendnonblocking(fd, sbuf, sbuflen);						 
						 toolkit_printbytes(sbuf, sbuflen);
				     }
					 break;
				case DEVICE_ONOFF:
				     {
					     struct protocol_cmdtype_onoff_ieee_cmd onoff;
					     onoff.cmdid = PROTOCOL_ONOFF;
					     protocol_parse_onoff(buffer, messagelen, &onoff.onoff_ieee);
						 printf("onoff.onoff_ieee.serialnum:%x\n", onoff.onoff_ieee.serialnum);
						 if(is_device_deleted(onoff.onoff_ieee.ieee)) {
							fprintf(stdout, "device has been deleted\n");
							break;
						 }
					     sendnonblocking(g_main_to_znp_write_fd, &onoff, sizeof(struct protocol_cmdtype_onoff_ieee_cmd));
				     }
					 break;
				case DEVICE_LEVEL_CTRL:
				     {
					 	printf("DEVICE_LEVEL_CTRL\n");
					     struct protocol_cmdtype_level_ctrl_ieee_cmd level_ctrl_ieee_cmd;
					     level_ctrl_ieee_cmd.cmdid = PROTOCOL_LEVEL_CTRL;
					     protocol_parse_level_ctrl(buffer, messagelen, &level_ctrl_ieee_cmd.level_ctrl_ieee);
						 if(is_device_deleted(level_ctrl_ieee_cmd.level_ctrl_ieee.ieee)) {
							fprintf(stdout, "device has been deleted\n");
							break;
						 }
						 struct endpoint * ep = gateway_get_endpoint(level_ctrl_ieee_cmd.level_ctrl_ieee.ieee, level_ctrl_ieee_cmd.level_ctrl_ieee.endpoint);
						 if(!ep) {
							 printf("event_recvmsg::no ep\n");
							 break;
						 } 
						 ep->simpledesc.device_state = level_ctrl_ieee_cmd.level_ctrl_ieee.level_ctrl_cmd.move2level.level;
					     sendnonblocking(g_main_to_znp_write_fd, &level_ctrl_ieee_cmd, sizeof(struct protocol_cmdtype_level_ctrl_ieee_cmd));
				     }
					 break;
				case PERMIT_JOINING:
				     {
					     struct protocol_cmdtype_permit_joining_cmd permit_joining_ieee_cmd;
					     permit_joining_ieee_cmd.cmdid = PROTOCOL_PERMIT_JOING;
					     protocol_parse_permit_joining(buffer, messagelen, &permit_joining_ieee_cmd.permit_joining);
						 
					     sendnonblocking(g_main_to_znp_write_fd, &permit_joining_ieee_cmd, sizeof(struct protocol_cmdtype_permit_joining_cmd));
				     }
					 break;
				case CONFIG_REPORT:
					{
						printf("PROTOCOL_CONFIG_REPORT\n");
						struct protocol_cmdtype_config_reporting_cmd cfg_rpt_cmd;
						memset(&cfg_rpt_cmd, 0, sizeof(cfg_rpt_cmd));
						cfg_rpt_cmd.cmdid = PROTOCOL_CONFIG_REPORT;
						protocol_parse_config_reporting(buffer, messagelen, &cfg_rpt_cmd.config_reporting);
						if(is_device_deleted(cfg_rpt_cmd.config_reporting.ieee)) {
						 	fprintf(stdout, "device has been deleted\n");
						 	break;
						}
						sendnonblocking(g_main_to_znp_write_fd, &cfg_rpt_cmd, sizeof(struct protocol_cmdtype_config_reporting_cmd));
					}
					break;
				#if 0
				case DEVICE_STATUS:
					{
					     //struct protocol_cmdtype_get_device_status_cmd get_status_cmd;
					     struct protocol_datatype_online_status online_status;
    					 struct zclbasicstatus basic_status;
						 unsigned char sbuf[512] = {0};
						
					     protocol_parse_get_device_status(buffer, messagelen, &online_status); 

						 struct device *d = gateway_getdevice(getgateway(), online_status.ieee);
						 if(!d) {
							fprintf(stdout, "device is NULL\n");
						 	break;
						 }
						 basic_status.ieeeaddr = online_status.ieee;
						 basic_status.status = (d->status & DEVICE_APP_DEL) ? 0 : 1;
						 if(online_status.period > 0)
						 	access_period = online_status.period * 60;
						 int len = protocol_encode_report_status(sbuf, &basic_status);
						 sendnonblocking(fd, sbuf, len); 
						// broadcast(sbuf, len);	
						 //get_status_cmd.cmdid = PROTOCOL_GET_STATUS;
						 //sendnonblocking(g_main_to_znp_write_fd, &get_status_cmd, sizeof(struct protocol_cmdtype_get_device_status_cmd));
						 //unsigned char buf[512] = {0};
					     //unsigned int len = protocol_encode_deviceattr(buf, &device_onoff);

					     //sendnonblocking(fd, buf, len); 
				     }
				     break;
				#endif
					
				case READ_ONOFF_CMD:
					{
						struct protocol_cmdtype_get_onoff_state_cmd onoff_state_cmd;
						protocol_parse_get_onoff_state(buffer, messagelen, &onoff_state_cmd.onoff_state);
						if(is_device_deleted(onoff_state_cmd.onoff_state.ieee)) {
						 	fprintf(stdout, "device has been deleted\n");
						 	break;
						}
						onoff_state_cmd.cmdid = PROTOCOL_READ_ONOFF;
						sendnonblocking(g_main_to_znp_write_fd, &onoff_state_cmd, sizeof(struct protocol_cmdtype_get_onoff_state_cmd));
					}
					break;
				case ILLEGAL:
					break;
			}
		}
	}
}

void send_config_report_cmd(struct device *d)
{
	struct list_head * pos, *n;
	struct endpoint * ep;
	unsigned char i;
	zclCfgReportCmd_t cfg_report;
	//int size = sizeof(cfg_report);
	//cfg_report.attrList = malloc(size);
	memset((char *)&cfg_report, 0, sizeof(zclCfgReportCmd_t));
	//memset(cfg_report.attrList, 0, cfg_report.numAttr * sizeof(zclCfgReportRec_t));
	cfg_report.numAttr = 1;
	cfg_report.attrList[0].attrID = 0x0000;
	cfg_report.attrList[0].dataType = ZCL_DATATYPE_BOOLEAN;
	cfg_report.attrList[0].direction = 0;
	cfg_report.attrList[0].maxReportInt = 7;
	cfg_report.attrList[0].minReportInt = 6;
	
	list_for_each_safe(pos, n, &d->eplisthead){
		ep = list_entry(pos, struct endpoint, list); 
		for(i = 0; i < ep->simpledesc.simpledesc.NumInClusters; i++){ 
			if(ep->simpledesc.simpledesc.InClusterList[i] == ZCL_CLUSTER_ID_GEN_ON_OFF) {
				//SimpleDescRspFormat_t sim_desc = ep->simpledesc.simpledesc;
				printf("zcl_SendConfigReportCmd\n");
				zcl_SendConfigReportCmd(2, ep->simpledesc.simpledesc.Endpoint, d->shortaddr, ZCL_CLUSTER_ID_GEN_ON_OFF, &cfg_report, ZCL_FRAME_CLIENT_SERVER_DIR, 0, 0);
			}
		}

	}
}

#define WARNING_INTERVAL	5
#define STATUS_MASK			0xcf

extern struct gateway gatewayinstance;

#if 1
#define seq_after(d, a, b) \
	(((char)b - (char)a < 0) || \
	(char)b - (char)a > 64)
#else

int seq_after(struct device *d, unsigned char a, unsigned char b) 
{
	if(strncasecmp(6, d->manufacturername, "feibit"))
		return (((char)b - (char)a < 0) || 	(char)b - (char)a > 64);
	else
		return ((char)b - (char)a < 0);
}
#endif


void event_recvznp(struct eventhub * hub, int fd){ 
	unsigned char buf[128] = {0};
	unsigned int buflen = 0;

	int znpdatatype = 0;
	readnonblocking(fd, &znpdatatype, sizeof(int));
	switch(znpdatatype){
		case ZCLZONEENROLLREQ: 
			{ 
				struct zclzoneenrollreq req;
				readnonblocking(fd, &req, sizeof(struct zclzoneenrollreq));
				fprintf(stdout, "********event recv znp enroll ieee %llX \n", req.ieeeaddr);
				struct device * d = gateway_getdevice(getgateway(), req.ieeeaddr);
				if(!d  || d->status & DEVICE_APP_DEL){
					return;
				}
				buflen = protocol_encode_add_del_device(buf, req.ieeeaddr, 1);
				broadcast(buf, buflen);
				
				/*send_config_report_cmd(d);*/
			}
			break;
		case ZCLZONECHANGENOTIFICATION:
			{
				struct zclzonechangenotification req;
				readnonblocking(fd, &req, sizeof(struct zclzonechangenotification));
				struct device * d = gateway_getdevice(getgateway(), req.ieeeaddr);
				if(!d || d->status & DEVICE_APP_DEL){
					return;
				}
				fprintf(stdout, "********event recv znp notification %llX \n", req.ieeeaddr);
				time_t curtime = time(NULL);
				//struct tm * tm = localtime(&curtime);
				struct tm * tm = gmtime(&curtime);
				//unsigned char hour = (unsigned char )tm->tm_hour + 8;
				unsigned char hour = (unsigned char )tm->tm_hour;
				unsigned char minute = (unsigned char )tm->tm_min; 
				struct endpoint * wd_ep = gateway_get_warning_device_endpoint();
				struct device * wd_device = gateway_get_warning_device();
				struct device * sensor = gateway_getdevice(getgateway(), req.ieeeaddr);
				if(!sensor) {
					fprintf(stdout, "**error: no sensor**\n");
					return;
				}
				struct endpoint * ep = gateway_get_endpoint(req.ieeeaddr, req.endpoint); 
			#ifdef DEBUG_EP
				if(!ep) {
					printf("error:ieee:%llx, endpoint:%d\n", req.ieeeaddr, req.endpoint);
					struct device * errdev = gateway_getdevice(&gatewayinstance, req.ieeeaddr);
					if(!errdev) 
						printf("no fucking device!\n");
					
					
					struct list_head *errpos, *errn; 
					struct endpoint *errep;
					list_for_each_safe(errpos, errn, &errdev->eplisthead){
						errep = list_entry(errpos, struct endpoint, list);
						printf("endpoint is %d\n", errep->simpledesc.simpledesc.Endpoint);						
					}
					if (list_empty(&errdev->eplisthead))
						printf("fuck!! no active ep!\n");
				}
			#endif
				if(ep) {
					printf("current seqence num:%d\n", req.trans_num);
					printf("last seqence num:%d\n", ep->simpledesc.zcl_transnum);
					if(seq_after(sensor, req.trans_num, ep->simpledesc.zcl_transnum)) {
						if(endpoint_check_arm(ep, hour, minute)) {
							if(0 == (req.ext_status & 0x80)) {
								printf("simpedisc.zonetype is %d\n", ep->simpledesc.zonetype);
								if(req.zonechangenotification.zonestatus.alarm1 || req.zonechangenotification.zonestatus.alarm2) {
									fprintf(stdout, "------start alarm\n");
									int warn_mode;
									switch(ep->simpledesc.zonetype) {
									case SS_IAS_ZONE_TYPE_CONTACT_SWITCH:
									case SS_IAS_ZONE_TYPE_MOTION_SENSOR:
										warn_mode = SS_IAS_START_WARNING_WARNING_MODE_BURGLAR;
										break;
									case SS_IAS_ZONE_TYPE_GAS_SENSOR:
									case SS_IAS_ZONE_TYPE_FIRE_SENSOR:
										warn_mode = SS_IAS_START_WARNING_WARNING_MODE_FIRE;
										break;
									case SS_IAS_ZONE_TYPE_KEY_FOB:
										warn_mode = SS_IAS_START_WARNING_WARNING_MODE_EMERGENCY;
										break;
									default:
										warn_mode = SS_IAS_START_WARNING_WARNING_MODE_STOP;
										break;
									}
									if(wd_device && !device_check_status(wd_device, DEVICE_APP_DEL)) {
										event_send_warning(wd_ep, wd_device->ieeeaddr,PROTOCOL_WARNING, warn_mode);
									}
									
								}
								buflen = protocol_encode_alarm(buf, &req);
								broadcast(buf, buflen);
							}
						}	
						ep->simpledesc.zcl_transnum = req.trans_num;
						sqlitedb_update_device_seq(req.ieeeaddr, req.endpoint, req.trans_num);
					}
				}
			}
			break;
		case ZCLGENERALDEFAULTRSP:
			{
				struct zclgeneraldefaultresponse dft_rsp;
				readnonblocking(fd, &dft_rsp, sizeof(dft_rsp));
				
				buflen = protocol_encode_general_response(buf, &dft_rsp);
				broadcast(buf, buflen);				
			}	
			break;
		case ZCLGENONOFFRSP:
			{
				struct zclgeneraldefaultresponse onoff_rsp;
				readnonblocking(fd, &onoff_rsp, sizeof(onoff_rsp));
				
				buflen = protocol_encode_onoff_response(buf, &onoff_rsp);
				int ret = sqlitedb_update_device_state(onoff_rsp.ieeeaddr, onoff_rsp.endpoint, onoff_rsp.cmd_ind);
				printf("event_recvznp::ZCLGENONOFFRSP:%d\n", ret);
				broadcast(buf, buflen);				
			}	
			break;			
		case ZCLREADONOFFRSP:
			{
				struct zclreadonoffrsp readonoff_rsp;
				readnonblocking(fd, &readonoff_rsp, sizeof(readonoff_rsp));
				
				buflen = protocol_encode_readonoff_response(buf, &readonoff_rsp);
				broadcast(buf, buflen);				
			}	
			break;
		case ZCLGENLEVELCTLRSP:
			{
				struct zcllevlctldefaultresponse levelctl_rsp;
				readnonblocking(fd, &levelctl_rsp, sizeof(levelctl_rsp));
				
				struct endpoint *ep = gateway_get_endpoint(levelctl_rsp.ieeeaddr, levelctl_rsp.endpoint); 
				if(ep) {
					levelctl_rsp.device_state = ep->simpledesc.device_state;
					if(0 == levelctl_rsp.status)
						sqlitedb_update_device_state(levelctl_rsp.ieeeaddr, levelctl_rsp.endpoint, levelctl_rsp.device_state);
				
					buflen = protocol_encode_level_response(buf, &levelctl_rsp);
					broadcast(buf, buflen);
				}
			}
			break;
		case ZCLWARNINGRSP:
			{
				struct zclgeneraldefaultresponse warning_rsp;
				readnonblocking(fd, &warning_rsp, sizeof(warning_rsp));
				
				buflen = protocol_encode_warning_response(buf, &warning_rsp);
				broadcast(buf, buflen);
			}
			break;
		case ZCLBASICSTATUS:
			{
				struct zclbasicstatus basic_status;
				readnonblocking(fd, &basic_status, sizeof(basic_status));
				
				buflen = protocol_encode_report_status(buf, &basic_status);
				broadcast(buf, buflen);			
			}
			break;
		#if 0
		case ZCLONOFFREPORT:
			{
				struct zclonoffreport onoff_report;
				readnonblocking(fd, &onoff_report, sizeof(onoff_report));
				printf("ieee:%llx\n", onoff_report.ieeeaddr);
				printf("endpoint:%x\n", onoff_report.endpoint);
				printf("state:%x\n", onoff_report.state);
				buflen = protocol_encode_onoff_report(buf, &onoff_report);
				broadcast(buf, buflen);	
			}
			break;
		#endif
	}
}

void event_close(int fd){
	struct connection * c = connrbtree_getconn(fd);
	if(c){ 
		connrbtree_del(c);
	}
}


void event_send_warning(struct endpoint * wd_ep, unsigned long long warning_device_ieee, unsigned char cmdid,unsigned char warning_mode){
	struct protocol_cmdtype_warning_ieee_cmd warning_ieee_cmd;
	warning_ieee_cmd.cmdid = cmdid;
	warning_ieee_cmd.warning_ieee.ieee = warning_device_ieee;
	warning_ieee_cmd.warning_ieee.warning.serialnum = 0;
	warning_ieee_cmd.warning_ieee.warning.endpoint = wd_ep->simpledesc.simpledesc.Endpoint;
	warning_ieee_cmd.warning_ieee.warning.start_warning.warningmessage.warningbits.warnStrobe = SS_IAS_START_WARNING_STROBE_USE_STPOBE_IN_PARALLEL_TO_WARNING;
	//warning_ieee_cmd.warning_ieee.warning.start_warning.warningmessage.warningbits.warnSirenLevel = SS_IAS_SIREN_LEVEL_MEDIUM_LEVEL_SOUND;
	warning_ieee_cmd.warning_ieee.warning.start_warning.warningmessage.warningbits.warnSirenLevel = SS_IAS_SIREN_LEVEL_LOW_LEVEL_SOUND;
	//warning_ieee_cmd.warning_ieee.warning.start_warning.warningDuration = 0xffff;
	warning_ieee_cmd.warning_ieee.warning.start_warning.warningDuration = 2;
	warning_ieee_cmd.warning_ieee.warning.start_warning.strobeDutyCycle = 1; // magic number
	//warning_ieee_cmd.warning_ieee.warning.start_warning.strobeLevel = SS_IAS_STROBE_LEVEL_MEDIUM_LEVEL_STROBE;
	warning_ieee_cmd.warning_ieee.warning.start_warning.strobeLevel = SS_IAS_STROBE_LEVEL_LOW_LEVEL_STROBE;
	warning_ieee_cmd.warning_ieee.warning.start_warning.warningmessage.warningbits.warnMode = warning_mode;

	sendnonblocking(g_main_to_znp_write_fd, &warning_ieee_cmd, sizeof(struct protocol_cmdtype_warning_ieee_cmd)); 
}

