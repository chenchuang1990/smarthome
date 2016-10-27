#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#include <pthread.h>

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
#include "zcl_ha.h"

//#define DEBUG_EP 1
//#define DEBUG
#define TESTMODE
#ifdef TESTMODE
#define TEST_ID	0x80ff
struct test_header {
	unsigned int serialnum;
	unsigned long long ieee;
	unsigned char endpoint;
};
#endif

extern int g_main_to_znp_write_fd;
extern struct connection * g_serverconn;
extern pthread_mutex_t big_mutex;
time_t login_time;


void event_send_warning(struct endpoint * wd_ep, unsigned long long warning_device_ieee, unsigned char cmdid,unsigned char warning_mode);

void event_accept(int fd){
	struct connection * c = freeconnlist_getconn();
	if(c) {
		//connection_init(c, fd, CONNSOCKETCLIENT);
		connection_init(c, fd, CONNFREE);
		connrbtree_insert(c);
	}
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
			connlist_checkstatus(hub, t);
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
				unsigned int buflen = protocol_encode_login(buf, 1); 
				print_hex(buf, buflen);
				sendnonblocking(serverfd, buf, buflen);				
				login_time = time(NULL);
				printf("[event_recvmsg]login_time:%lx\n", login_time);
			}
		}
	}
	else if(c && (connection_gettype(c) == CONNSOCKETCLIENT || 
					connection_gettype(c) == CONNSOCKETSERVER || 
					connection_gettype(c) == CONNFREE)){ 
		c->timestamp = time(NULL);
		connection_put(c, buf, buflen); 

		for(;;) { // in case of receive two packets one time.
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

						broadcast(sbuf, slen);

						//sendnonblocking(fd, sbuf, slen);
						//toolkit_printbytes(sbuf, slen);
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
							//device_set_status(d, DEVICE_APP_DEL);
							device_clear_status(d, DEVICE_APP_ADD);
							pthread_mutex_lock(&big_mutex);
							printf("delete lock\n");
							sqlitedb_delete_device(del_device.ieee);							
							gateway_deldevice(getgateway(), d);	
							d = NULL;
							printf("delete unlock\n");
							pthread_mutex_unlock(&big_mutex);
							result = 0;
						}
						unsigned char sbuf[128] = {0}; 
						unsigned int slen = protocol_encode_del_device_feedback(sbuf, &del_device, result);

						//sendnonblocking(fd, sbuf, slen);
						broadcast(sbuf, slen);
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
					 	 printf("APP_LOGIN:");
						 unsigned long long gateway_id;						 
					     unsigned char sbuf[2048] = {0}; 
						 int match = 1;

						 protocol_parse_app_login(buffer, messagelen, &gateway_id);
						 if(getgateway()->gatewayid != gateway_id) {
						 	match = 0;
						 }
						 
					     unsigned int sbuflen = protocol_encode_login(sbuf, match); 

					     int login_len = sendnonblocking(fd, sbuf, sbuflen);
						 printf("login_len:%d\n", login_len);
						 print_hex(sbuf, sbuflen);
						 login_time = time(NULL);
						 c->type = CONNSOCKETCLIENT;
				     }
					 break;
				case DEVICE_SETARM:
				     {
					 	printf("DEVICE_SETARM\n");
					     struct protocol_cmdtype_setarm setarm;
						 unsigned char result;
					     //unsigned int serialnum = 0;
					     //unsigned char endpoint = 0;
					     //unsigned long long ieee = protocol_parse_arm(buffer, messagelen, &arm);
					     protocol_parse_arm(buffer, messagelen, &setarm);
						 
						 if(setarm.arm.starthour < 24 && setarm.arm.startminute < 60 && setarm.arm.endhour < 24 && setarm.arm.endminute < 60) {
						     result = (unsigned char)sqlitedb_update_device_arm(setarm.ieee, setarm.endpoint, &setarm.arm);
						     //result = (result == 0)?1:0;
						     if(result == 0){ 
							     struct endpoint * ep = gateway_get_endpoint(setarm.ieee, setarm.endpoint);
								 if(ep) {
							     	memcpy(&ep->simpledesc.arm, &setarm.arm, sizeof(struct protocol_cmdtype_arm));
								 }
								 else {
								 	result = 1;
								 }	
						     }
						 }
						 else 
						 	result = 1;
						 
					     unsigned char sbuf[128] = {0};
					     unsigned int sbuflen = protocol_encode_arm_feedback(sbuf, &setarm, result);
					     //sendnonblocking(fd, sbuf, sbuflen);						 
						 //toolkit_printbytes(sbuf, sbuflen);
						 broadcast(sbuf, sbuflen);
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
				
				case READ_ALARM_STAUS:
					{
					     unsigned char result;
					     struct protocol_cmdtype_setarm armset;
						 memset(&armset, 0, sizeof(armset));
					     protocol_parse_get_alarm_cmd(buffer, messagelen, &armset); 
						 
						 struct endpoint * ep = gateway_get_endpoint(armset.ieee, armset.endpoint);
						 if(ep) {
					     	memcpy(&armset.arm, &ep->simpledesc.arm, sizeof(struct protocol_cmdtype_arm));
							result = 0;
						 }
						 else {
						 	result = 1;
						 }	
					     unsigned char sbuf[512] = {0};
					     unsigned int slen = protocol_encode_arm_feedback(sbuf, &armset, result);
					     broadcast(sbuf, slen); 
				     }
				     break;
				case READ_ONOFF_CMD:
					{
						//unsigned char onoff = 0;
						struct protocol_cmdtype_read_state onoff_state;
						//struct list_head *pos, *n;
						//struct device *other_dev;
						
						protocol_parse_read_state_cmd(buffer, messagelen, &onoff_state);
						/*if(is_device_deleted(onoff_state.onoff_state.ieee)) {
						 	fprintf(stdout, "device has been deleted\n");
						 	break;
						}*/
						

						struct device * d = gateway_getdevice(getgateway(), onoff_state.ieee);
						//if(d && !(d->status & DEVICE_APP_DEL)) {
						if(d && (d->status & DEVICE_APP_ADD)) {
							/*if endpoint number is 0, then set the noneedcheck member of struct device*/
							if(0 == onoff_state.endpoint) {
								if(d->accesscnt-- <= 0)
									d->accesscnt = 0;
								printf("exit count:%d\n", d->accesscnt);
								break;
							}
							//else if (0xff == onoff_state.endpoint && 1 == d->noneedcheck) {
							else if (0xff == onoff_state.endpoint) {
								d->accesscnt++;
								printf("enter count:%d\n", d->accesscnt);
								c->cur_dev = d;
								d->timestamp = time(NULL);
								#if 0
								list_for_each_safe(pos, n, &getgateway()->head){ 
									other_dev = list_entry(pos, struct device, list); 
									if(other_dev->noneedcheck == 0 && other_dev->ieeeaddr != d->ieeeaddr){
										other_dev->noneedcheck = 1;
									}
								}
								#endif
							}
							#if 0
							struct endpoint *ep = device_get_endpoint(d, onoff_state.endpoint);					
							if(ep) {
						    	onoff = ep->simpledesc.device_state;
								result = 0;
							}
							#endif
						}
						else {
							fprintf(stdout, "READ_ONOFF_CMD:device has been deleted\n");
						 	break;
						}

						/*unsigned char sbuf[512] = {0};
						unsigned int slen = protocol_encode_state_feedback(sbuf, &onoff_state, READ_ONOFF_RSP, onoff);
						sendnonblocking(fd, sbuf, slen);
						toolkit_printbytes(sbuf, slen);*/
						//onoff_state_cmd.cmdid = PROTOCOL_READ_ONOFF;
						//sendnonblocking(g_main_to_znp_write_fd, &onoff_state_cmd, sizeof(struct protocol_cmdtype_get_onoff_state_cmd));
					}
					break;
				case READ_LEVEL_CMD:
					{
						//unsigned char result = 1, level = 0;
						struct protocol_cmdtype_read_state level_state;
						//struct list_head *pos, *n;
						//struct device *other_dev;
						
						protocol_parse_read_state_cmd(buffer, messagelen, &level_state);
						/*if(is_device_deleted(onoff_state.onoff_state.ieee)) {
						 	fprintf(stdout, "device has been deleted\n");
						 	break;
						}*/

						struct device * d = gateway_getdevice(getgateway(), level_state.ieee);
						//if(d && !(d->status & DEVICE_APP_DEL)) {
						if(d && (d->status & DEVICE_APP_ADD)) {
							/*if endpoint number is 0, then set the noneedcheck member of struct device*/
							if(0 == level_state.endpoint) {
								if(d->accesscnt-- <= 0)
									d->accesscnt = 0;
								printf("exit count:%d\n", d->accesscnt);
								break;
							}
							else if (0xff == level_state.endpoint) {
								d->accesscnt++;
								printf("enter count:%d\n", d->accesscnt);
								d->timestamp = time(NULL);
								#if 0
								list_for_each_safe(pos, n, &getgateway()->head){ 
									other_dev = list_entry(pos, struct device, list); 
									if(other_dev->noneedcheck == 0 && other_dev->ieeeaddr != d->ieeeaddr){
										other_dev->noneedcheck = 1;
									}
								}
								#endif
							}
							#if 0
							struct endpoint *ep = device_get_endpoint(d, level_state.endpoint);					
							if(ep) {
						    	onoff = ep->simpledesc.device_state;
								result = 0;
							}
							#endif
						}
						else {
							fprintf(stdout, "READ_LEVEL_CMD:device has been deleted\n");
						 	break;
						}

						/*unsigned char sbuf[512] = {0};
						unsigned int slen = protocol_encode_state_feedback(sbuf, &level_state, READ_LEVEL_RSP, level);
						sendnonblocking(fd, sbuf, slen);
						toolkit_printbytes(sbuf, slen);*/
						//onoff_state_cmd.cmdid = PROTOCOL_READ_ONOFF;
						//sendnonblocking(g_main_to_znp_write_fd, &onoff_state_cmd, sizeof(struct protocol_cmdtype_get_onoff_state_cmd));
					}
					break;
				case TEST_ID:
					{
						const unsigned char *p = buffer;
						struct test_header test;
						zclCfgReportCmd_t CfgReportCmd;
						
						bytebuffer_skipbytes(&p, 5);
						bytebuffer_readdword(&p, &test.serialnum);
						bytebuffer_readquadword(&p, &test.ieee);
						bytebuffer_readbyte(&p, &test.endpoint);
						printf("serialnum:%d ieee:%llx endpoint:%x\n", test.serialnum, test.ieee, test.endpoint);
						//unsigned char change = 0;
						memset(&CfgReportCmd, 0, sizeof(CfgReportCmd));
						CfgReportCmd.numAttr = 1;
						CfgReportCmd.attrList[0].direction = 0;
						CfgReportCmd.attrList[0].attrID = 0x0000;
						CfgReportCmd.attrList[0].dataType = 0x10;
						CfgReportCmd.attrList[0].minReportInt = 0;
						CfgReportCmd.attrList[0].maxReportInt = 30;
						CfgReportCmd.attrList[0].timeoutPeriod = 0;
						CfgReportCmd.attrList[0].reportableChange = NULL;

						struct device *d = gateway_getdevice(getgateway(), test.ieee);
						if(d) {
							printf("gateway_get_endpoint\n");
							zcl_SendConfigReportCmd(1, 1, d->shortaddr, 0x0006, &CfgReportCmd, 0, 1, get_sequence());
						}
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

#define seq_after(d, a, b) \
	(((char)b - (char)a < 0) || \
	(char)b - (char)a > 32)

#define min(a,b) a>b?b:a

/*add the code that report fucking devicename*/

enum dn {
	SWITCH,
	SOCKET,
	CONTACT,
	MOTION,
	GAS,
	FIRE,
	KEY,
	WARN,
	SHADE,
	BELL,
	WATER,
	UNKNOWN
};

char utf8_table[][32] = {{0xe9, 0x94, 0xae, 0xe5, 0xbc, 0x80, 0xe5, 0x85, 0xb3, 0},
								{0xe6, 0x8f, 0x92, 0xe5, 0xba, 0xa7, 0},
								{0xe9, 0x97, 0xa8, 0xe7, 0xa3, 0x81, 0},
								{0xe7, 0xba, 0xa2, 0xe5, 0xa4, 0x96, 0xe7, 0xa7, 0xbb, 0xe5, 0x8a, 0xa8, 0xe6, 0x8a, 0xa5, 0xe8, 0xad, 0xa6, 0},
								{0xe7, 0x93, 0xa6, 0xe6, 0x96, 0xaf, 0xe4, 0xbc, 0xa0, 0xe6, 0x84, 0x9f, 0xe5, 0x99, 0xa8, 0},
								{0xe7, 0x83, 0x9f, 0xe9, 0x9b, 0xbe, 0xe4, 0xbc, 0xa0, 0xe6, 0x84, 0x9f, 0xe5, 0x99, 0xa8, 0},
								{0xe7, 0xb4, 0xa7, 0xe6, 0x80, 0xa5, 0xe6, 0x8c, 0x89, 0xe9, 0x92, 0xae, 0},
								{0xe6, 0x8a, 0xa5, 0xe8, 0xad, 0xa6, 0xe5, 0x99, 0xa8, 0},
								{0xe7, 0xaa, 0x97, 0xe5, 0xb8, 0x98, 0},
								{0xe9, 0x97, 0xa8, 0xe9, 0x93, 0x83, 0},
								{0xe6, 0xb0, 0xb4, 0xe6, 0xb5, 0xb8, 0xe4, 0xbc, 0xa0, 0xe6, 0x84, 0x9f, 0xe5, 0x99, 0xa8, 0},
								{0xe6, 0x9c, 0xaa, 0xe5, 0x91, 0xbd, 0xe5, 0x90, 0x8d, 0}
						};

int devicetype_valid(unsigned short devicetype)
{
	if(ZCL_HA_DEVICEID_IAS_ANCILLARY_CONTROL_EQUIPMENT == devicetype || 
		ZCL_HA_DEVICEID_IAS_ZONE == devicetype || 
		ZCL_HA_DEVICEID_IAS_WARNING_DEVICE == devicetype || 
		ZCL_HA_DEVICEID_SHADE == devicetype ||
		ZCL_HA_DEVICEID_WINDOW_COVERING_DEVICE == devicetype) 
		//|| ZCL_HA_DEVICEID_MAINS_POWER_OUTLET == devicetype)
		return 1;
	else 
		return 0;
}

void set_devicename(struct device *d, unsigned short devicetypeid, unsigned short zonetype, unsigned char epcnt)
{
	char name[MAXNAMELEN] = {0};
	int slen = 0;
	
	switch(devicetypeid) {
	#if 0
	case ZCL_HA_DEVICEID_MAINS_POWER_OUTLET:
		
		printf("ZCL_HA_DEVICEID_MAINS_POWER_OUTLET:%s", d->modelidentifier);
		if(!strncasecmp(d->modelidentifier, "Z-809", 5)) {
			snprintf(name, sizeof(name), "%s", utf8_table[SOCKET]);
		}
		else if('F' == d->modelidentifier[0]) {
			int i;
			char *substrp[2];
			char *out_ptr = NULL;
			char sa[33] = {0};
			char *str = sa;			
			slen = strlen(d->modelidentifier);
			memcpy(str, d->modelidentifier, min(slen, 33));
			str[32] = 0;
			
			for(i = 0; i < 2; str = NULL, i++) {
				substrp[i] = strtok_r(str, "-", &out_ptr);
				if(NULL == substrp[i])
					break;
				printf("--> %s\n", substrp[i]);
			}
			if(!strncasecmp(substrp[1], "SKT", 3))
				snprintf(name, sizeof(name), "%s", utf8_table[SOCKET]);
		}
		else
			snprintf(name, sizeof(name), "%d%s", epcnt, utf8_table[SWITCH]);
		
		break;
	#endif
	case ZCL_HA_DEVICEID_IAS_ZONE:
		switch(zonetype) {
		case SS_IAS_ZONE_TYPE_CONTACT_SWITCH:
			snprintf(name, sizeof(name), "%s", utf8_table[CONTACT]);
			break;
		case SS_IAS_ZONE_TYPE_MOTION_SENSOR:
			snprintf(name, sizeof(name), "%s", utf8_table[MOTION]);
			break;
		case SS_IAS_ZONE_TYPE_GAS_SENSOR:
			snprintf(name, sizeof(name), "%s", utf8_table[GAS]);
			break;
		case SS_IAS_ZONE_TYPE_FIRE_SENSOR:
			snprintf(name, sizeof(name), "%s", utf8_table[FIRE]);
			break;
		case SS_IAS_ZONE_TYPE_KEY_FOB:
			snprintf(name, sizeof(name), "%s", utf8_table[KEY]);
			break;
		case SS_IAS_ZONE_TYPE_WATER_SENSOR:
			snprintf(name, sizeof(name), "%s", utf8_table[WATER]);
		}
		break;
	case ZCL_HA_DEVICEID_IAS_WARNING_DEVICE:
		snprintf(name, sizeof(name), "%s", utf8_table[WARN]);
		break;
	case ZCL_HA_DEVICEID_SHADE:
		snprintf(name, sizeof(name), "%s", utf8_table[SHADE]);
		break;
	case ZCL_HA_DEVICEID_IAS_ANCILLARY_CONTROL_EQUIPMENT:
		snprintf(name, sizeof(name), "%s", utf8_table[BELL]);
		break;
	default:
		snprintf(name, sizeof(name), "%s", utf8_table[UNKNOWN]);
	}
	slen = strlen(name); 
	memcpy(d->devicename, name, min(slen, MAXNAMELEN-1));
	//toolkit_printbytes((unsigned char *)d->devicename, slen);
	d->devicename[MAXNAMELEN-1] = 0;
	sqlitedb_update_devicename(d->ieeeaddr, d->devicename);
}

void init_devicename(struct device *d)
{
	//printf("init_devicename\n");
	struct list_head *pos, *n;
	struct endpoint *ep;
	if(0 == strlen(d->devicename)) {
		list_for_each_safe(pos, n,&d->eplisthead) {
			ep = list_entry(pos, struct endpoint, list);
			if(devicetype_valid(ep->simpledesc.simpledesc.DeviceID)) {
				printf("devicetype is valid\n");
				if(ZCL_HA_DEVICEID_IAS_ZONE == ep->simpledesc.simpledesc.DeviceID && 
						0 == ep->simpledesc.zonetype)
					break;
				set_devicename(d, ep->simpledesc.simpledesc.DeviceID,
					ep->simpledesc.zonetype, d->activeep.ActiveEPCount);
			}
		}
	}
}

static int should_not_report(struct device *d)
{
	struct list_head *pos, *n;
	struct endpoint *ep;

	list_for_each_safe(pos, n, &d->eplisthead) {
		ep = list_entry(pos, struct endpoint, list);
		if(ZCL_HA_DEVICEID_IAS_ZONE == ep->simpledesc.simpledesc.DeviceID &&
			0 == ep->simpledesc.zonetype)
			return 1;
	}
	return 0;
}

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
				//if(!d  || d->status & DEVICE_APP_DEL) {
				//if(!d) {
				if(!d || should_not_report(d)) {
					return;
				}
				//d->status &= ~DEVICE_APP_DEL;
				//sqlitedb_update_device_status(d);
				device_set_status(d, DEVICE_APP_ADD);

				//add the devicename field
				if(0 == strlen(d->devicename)) {
					init_devicename(d);
					sqlitedb_update_devicename(d->ieeeaddr, d->devicename);
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
				//if(!d || d->status & DEVICE_APP_DEL){
				if(!d || !(d->status & DEVICE_APP_ADD)) {
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
							if(0 == (req.ext_status & 0x80)) {	//not heartbeat
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
									case SS_IAS_ZONE_TYPE_WATER_SENSOR:
										warn_mode = SS_IAS_START_WARNING_WARNING_MODE_FIRE;
										break;
									case SS_IAS_ZONE_TYPE_KEY_FOB:
										warn_mode = SS_IAS_START_WARNING_WARNING_MODE_EMERGENCY;
										break;
									default:
										warn_mode = SS_IAS_START_WARNING_WARNING_MODE_STOP;
										break;
									}
									//if(wd_device && !device_check_status(wd_device, DEVICE_APP_DEL)) {
									if(wd_device && device_check_status(wd_device, DEVICE_APP_ADD)) {
										printf("wd_device shortaddr is 0x%04x\n", wd_device->shortaddr);
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
					else {
						printf("warning:[event_recvznp] current seq(%d) is before last seq(%d)\n", req.trans_num, ep->simpledesc.zcl_transnum);
					}
				}
				else {
					printf("error:[event_recvznp] no ep\n");
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
				sqlitedb_update_device_state(readonoff_rsp.ieeeaddr, readonoff_rsp.endpoint, readonoff_rsp.state);
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
		case ZCLREADLEVELCTLRSP:
			{
				struct zclreadlevelctlrsp readlevel_rsp;
				readnonblocking(fd, &readlevel_rsp, sizeof(readlevel_rsp));
				
				buflen = protocol_encode_readlevel_response(buf, &readlevel_rsp);
				sqlitedb_update_device_state(readlevel_rsp.ieeeaddr, readlevel_rsp.endpoint, readlevel_rsp.cur_level);
				broadcast(buf, buflen);				
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

