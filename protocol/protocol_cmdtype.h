#ifndef __PROTOCOL_CMDTYPE_H_H_
#define __PROTOCOL_CMDTYPE_H_H_
#include "zcl_ss.h" 

#define ARM 0
#define DISARM 1
#define ARMTIME 2

#define ARMTIMECOUNT 4

#define PROTOCOL_IDENTIFY 1
#define PROTOCOL_WARNING 2
#define PROTOCOL_ONOFF 3
#define PROTOCOL_LEVEL_CTRL 4
#define PROTOCOL_PERMIT_JOING 5
#define PROTOCOL_CONFIG_REPORT 6
//#define PROTOCOL_GET_STATUS 7
#define PROTOCOL_READ_ONOFF  8



struct protocol_cmdtype_arm{
	unsigned char armmodel;
	unsigned char starthour;
	unsigned char startminute;
	unsigned char endhour;
	unsigned char endminute; 
};

struct protocol_cmdtype_setarm {
		unsigned int serialnum;
		unsigned long long ieee;	
		unsigned char endpoint;
		struct protocol_cmdtype_arm arm;
};

struct __attribute__((packed))protocol_cmdtype_identify{ 
	unsigned char endpoint;
	unsigned int serialnum;
};

struct __attribute__((packed))protocol_cmdtype_identify_ieee{
	unsigned long long ieee;
	struct protocol_cmdtype_identify identify;
};

struct __attribute__((packed))protocol_cmdtype_identify_ieee_cmd{
	int cmdid;
	struct protocol_cmdtype_identify_ieee identify_ieee;
};

struct __attribute__((packed))protocol_cmdtype_warning{
	unsigned int serialnum;
	unsigned char endpoint;
	zclWDStartWarning_t start_warning;
};

struct __attribute__((packed))protocol_cmdtype_warning_ieee{
	unsigned long long ieee;
	struct protocol_cmdtype_warning warning;
};

struct __attribute__((packed))protocol_cmdtype_warning_ieee_cmd{
	int cmdid;
	struct protocol_cmdtype_warning_ieee warning_ieee;
};

struct __attribute__((packed))protocol_cmdtype_onoff_ieee{
	unsigned long long ieee;
	unsigned int serialnum;
	unsigned char endpoint;
	unsigned char onoff;
};

struct __attribute__((packed))protocol_cmdtype_onoff_ieee_cmd{
	int cmdid; 
	struct protocol_cmdtype_onoff_ieee onoff_ieee;
};

struct __attribute__((packed))protocol_move2level_cmd{
	unsigned char level;
	unsigned short trans_time;
};

struct __attribute__((packed))protocol_move_cmd{
	unsigned char move_mode;
	unsigned char rate;
};

struct __attribute__((packed))protocol_step_cmd{
	unsigned char step_mode;
	unsigned char step_size;
	unsigned short trans_time;
};

struct __attribute__((packed))protocol_cmdtype_level_ctrl_ieee{
	unsigned long long ieee;
	unsigned int serialnum;
	unsigned char endpoint;
	unsigned char zcl_cmdid;
	union __attribute__((packed)){
		struct protocol_move2level_cmd move2level;
		struct protocol_move_cmd	move;
		struct protocol_step_cmd	step;
	}level_ctrl_cmd;
};

struct __attribute__((packed))protocol_cmdtype_level_ctrl_ieee_cmd{
	int cmdid;
	struct protocol_cmdtype_level_ctrl_ieee level_ctrl_ieee;
};

struct __attribute__((packed))protocol_cmdtype_permit_joining {
	unsigned int serialnum;
	unsigned char joing_time;
};

struct __attribute__((packed))protocol_cmdtype_permit_joining_cmd {
	int cmdid;
	struct protocol_cmdtype_permit_joining permit_joining;
};

struct __attribute__((packed))cfg_report_recod {	
	unsigned char direction;
	unsigned short attr_id;
	unsigned char data_type;
	unsigned short min_interval;
	unsigned short max_interval;
	unsigned short timeout_period;
	unsigned char *reprotable_change;
};

struct __attribute__((packed))protocol_cmdtype_config_reporting {
	unsigned long long ieee;
	unsigned int serialnum;
	unsigned short clusterid;
	unsigned char endpoint;
	unsigned char attr_num;
	struct cfg_report_recod attr_list[10];
};

struct __attribute__((packed))protocol_cmdtype_config_reporting_cmd {
	int cmdid;
	struct protocol_cmdtype_config_reporting config_reporting;
};

struct __attribute__((packed))protocol_cmdtype_get_device_status {
	unsigned long long ieee;
	unsigned int serialnum;
	unsigned char period;
};

struct __attribute__((packed))protocol_cmdtype_get_device_status_cmd{
	int cmdid;
	struct protocol_cmdtype_get_device_status get_device_status;
};

struct __attribute__((packed))protocol_cmdtype_read_state {
	unsigned long long ieee;
	unsigned int serialnum;
	unsigned char endpoint;
};

struct __attribute__((packed))protocol_cmdtype_report_online {
	unsigned long long ieee;
	unsigned int serialnum;
	unsigned char endpoint;
	unsigned char on;
};

/*
struct __attribute__((packed))protocol_cmdtype_get_onoff_state_cmd{
	int cmdid;
	struct protocol_cmdtype_get_onoff_state onoff_state;
};
*/

/*
struct __attribute__((packed))protocol_cmdtype_get_alarm_state {
	unsigned long long ieee;
	unsigned int serialnum;
	unsigned char endpoint;
};

struct __attribute__((packed))protocol_cmdtype_get_alarm_state_cmd{
	int cmdid;
	struct protocol_cmdtype_get_alarm_state alarm_state;
};
*/

#endif
