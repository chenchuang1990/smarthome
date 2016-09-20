#ifndef __ZCL_DOWN_CMD_H_H
#define __ZCL_DOWN_CMD_H_H

#include "protocol_cmdtype.h"
#define ZCL_DOWN_IDENTIFY 0x800C

//unsigned char trans_seq_num;

unsigned char get_sequence(void);

void zcl_down_cmd_identify(unsigned long long ieee,struct protocol_cmdtype_identify *  identify);

void zcl_down_cmd_warning(unsigned long long ieee, struct protocol_cmdtype_warning * warning);

void zcl_down_cmd_onoff(struct protocol_cmdtype_onoff_ieee * onoff);

void zcl_down_cmd_level_ctrl(struct protocol_cmdtype_level_ctrl_ieee * level_ctrl);
void zcl_down_cmd_permit_joining(struct protocol_cmdtype_permit_joining* permit_joining);
void zcl_down_cmd_config_reporting(struct protocol_cmdtype_config_reporting* config_reporting);
//void zcl_down_cmd_get_dstatus(struct protocol_cmdtype_get_device_status* get_dstatus);
//void zcl_down_cmd_get_onoff(struct protocol_cmdtype_get_onoff_state* onoff_attr);

#endif
