#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include "zcl_down_cmd.h"
#include "zcl_general.h"
#include "gateway.h"
#include "zcl_ss.h"
#include "zcl_register_cluster.h"
#include "mtSapi.h"
#include "key.h"
#include "commands.h"



#define IDENTYFYTIME 30

unsigned char trans_seq_num;

unsigned char get_sequence(void)
{
	return trans_seq_num++;
}

void zcl_down_cmd_identify(unsigned long long ieee, struct protocol_cmdtype_identify * identify){ 
	struct device * d = gateway_getdevice(getgateway(), ieee);
	if(d){
		fprintf(stdout, " **identify endpoint %d\n", identify->endpoint);
		fprintf(stdout, " **identify nwkwork %d\n", d->shortaddr);
		zclGeneral_SendIdentify(APP_DEVICETYPEID_SS_ENDPOINT, 
				identify->endpoint,
				d->shortaddr,
				IDENTYFYTIME, 0, identify->serialnum);
	}
}

void zcl_down_cmd_warning(unsigned long long ieee, struct protocol_cmdtype_warning* warning){ 
	struct endpoint * dstep = gateway_get_endpoint(ieee, warning->endpoint);

	if(dstep){
		/*fprintf(stdout, " **warning endpoint %d\n", warning->endpoint);
		fprintf(stdout, " **warning warnmode %d\n", warning->start_warning.warningmessage.warningbits.warnMode);
		fprintf(stdout, " **warning warnstrobe %d\n", warning->start_warning.warningmessage.warningbits.warnStrobe);
		fprintf(stdout, " **warning warnSirenlevel%d\n", warning->start_warning.warningmessage.warningbits.warnSirenLevel);
		fprintf(stdout, " **warning warningDuration %d\n", warning->start_warning.warningDuration);
		fprintf(stdout, " **warning strobeDutyCycle%d\n", warning->start_warning.strobeDutyCycle);
		fprintf(stdout, " **warning strobeLevel%d\n", warning->start_warning.strobeLevel);*/
		zclss_send_ias_wd_start_warning_cmd(APP_DEVICETYPEID_SS_ENDPOINT, 
				warning->endpoint,
				dstep->simpledesc.simpledesc.NwkAddr,
				&warning->start_warning,
				0, warning->serialnum);
	}
}

void zcl_down_cmd_onoff(struct protocol_cmdtype_onoff_ieee * onoff){
	struct device * d = gateway_getdevice(getgateway(), onoff->ieee);

	afAddrType_t addrtype;
	if(d){
		fprintf(stdout, "send onoff command\n");
		addrtype.addr.shortAddr = d->shortaddr;
		addrtype.endPoint = onoff->endpoint;
		if(onoff->onoff){
			//zclGeneral_SendOnOff_CmdOn(APP_DEVICETYPEID_SS_ENDPOINT, &addrtype,0,0);
			//zclGeneral_SendOnOff_CmdOn(APP_DEVICETYPEID_SS_ENDPOINT, &addrtype,0,get_sequence());
			zclGeneral_SendOnOff_CmdOn(APP_DEVICETYPEID_SS_ENDPOINT, &addrtype,0,(unsigned char)onoff->serialnum);
		}else{
			//zclGeneral_SendOnOff_CmdOff(APP_DEVICETYPEID_SS_ENDPOINT, &addrtype,0,0);
			//zclGeneral_SendOnOff_CmdOff(APP_DEVICETYPEID_SS_ENDPOINT, &addrtype,0,get_sequence());
			zclGeneral_SendOnOff_CmdOff(APP_DEVICETYPEID_SS_ENDPOINT, &addrtype,0,(unsigned char)onoff->serialnum);
		}

	}

}

void zcl_down_cmd_level_ctrl(struct protocol_cmdtype_level_ctrl_ieee * level_ctrl)
{

	struct device * d = gateway_getdevice(getgateway(), level_ctrl->ieee);

	if(d){
		fprintf(stdout, "send level ctrl cmd\n");
		printf("level_ctrl->serialnum:%x\n", level_ctrl->serialnum);
		afAddrType_t addrtype;
		addrtype.addr.shortAddr = d->shortaddr;
		addrtype.endPoint = level_ctrl->endpoint;
		switch(level_ctrl->zcl_cmdid) {
		case COMMAND_LEVEL_MOVE_TO_LEVEL:
			zclGeneral_SendLevelControlMoveToLevel(APP_DEVICETYPEID_CLOSURES_ENDPOINT, &addrtype, 
				level_ctrl->level_ctrl_cmd.move2level.level, level_ctrl->level_ctrl_cmd.move2level.trans_time,0, level_ctrl->serialnum);
			break;
		case COMMAND_LEVEL_MOVE:
			zclGeneral_SendLevelControlMove(APP_DEVICETYPEID_CLOSURES_ENDPOINT, &addrtype, 
				level_ctrl->level_ctrl_cmd.move.move_mode, level_ctrl->level_ctrl_cmd.move.rate,0, level_ctrl->serialnum);
			break;
		case COMMAND_LEVEL_STEP:
			zclGeneral_SendLevelControlStep(APP_DEVICETYPEID_CLOSURES_ENDPOINT, &addrtype, 
				level_ctrl->level_ctrl_cmd.step.step_mode, level_ctrl->level_ctrl_cmd.step.step_size, level_ctrl->level_ctrl_cmd.step.trans_time, 0, level_ctrl->serialnum);
			break;
		case COMMAND_LEVEL_STOP:
			zclGeneral_SendLevelControlStop(APP_DEVICETYPEID_CLOSURES_ENDPOINT, &addrtype, 0, level_ctrl->serialnum);
			break;
		case COMMAND_LEVEL_MOVE_TO_LEVEL_WITH_ON_OFF:
			zclGeneral_SendLevelControlMoveToLevelWithOnOff(APP_DEVICETYPEID_CLOSURES_ENDPOINT, &addrtype, 
				level_ctrl->level_ctrl_cmd.move2level.level, level_ctrl->level_ctrl_cmd.move2level.trans_time,0, level_ctrl->serialnum);
			break;
		case COMMAND_LEVEL_MOVE_WITH_ON_OFF:
			zclGeneral_SendLevelControlMoveWithOnOff(APP_DEVICETYPEID_CLOSURES_ENDPOINT, &addrtype, 
				level_ctrl->level_ctrl_cmd.move.move_mode, level_ctrl->level_ctrl_cmd.move.rate,0, level_ctrl->serialnum);
			break;
		case COMMAND_LEVEL_STEP_WITH_ON_OFF:
			zclGeneral_SendLevelControlStepWithOnOff(APP_DEVICETYPEID_CLOSURES_ENDPOINT, &addrtype, 
				level_ctrl->level_ctrl_cmd.step.step_mode, level_ctrl->level_ctrl_cmd.step.step_size, level_ctrl->level_ctrl_cmd.step.trans_time, 0, level_ctrl->serialnum);
			break;
		case COMMAND_LEVEL_STOP_WITH_ON_OFF:
			zclGeneral_SendLevelControlStopWithOnOff(APP_DEVICETYPEID_CLOSURES_ENDPOINT, &addrtype, 0, level_ctrl->serialnum);
			break;
		}
		
	}
}

void zcl_down_cmd_permit_joining(struct protocol_cmdtype_permit_joining* permit_joining)
{
	PermitJoiningReqFormat_t request;

	printf("zcl_down_cmd_permit_joining\n");
	request.Destination = 0x0000;
	request.Timeout = permit_joining->joing_time; 

	start_led_timer(permit_joining->joing_time);
	sendcmd((unsigned char *)&request, ZB_PERMIT_JOINING_REQ);
}

void zcl_down_cmd_config_reporting(struct protocol_cmdtype_config_reporting* config_reporting)
{
	int i;
	zclCfgReportCmd_t CfgReportCmd;
	//unsigned short temp;

	printf("zcl_down_cmd_config_reporting\n");
	printf("ieee:0x%016llx\n",config_reporting->ieee);
	printf("clusterid:0x%04x\n",config_reporting->clusterid);
	printf("endpoint:0x%02x\n",config_reporting->endpoint);
	printf("attr_num:0x%02x\n",config_reporting->attr_num);
	CfgReportCmd.numAttr = config_reporting->attr_num;
	for(i = 0; i < config_reporting->attr_num; i++) {
		CfgReportCmd.attrList[i].direction = config_reporting->attr_list[i].direction;
		printf("direction:0x%02x\n", CfgReportCmd.attrList[i].direction);
		CfgReportCmd.attrList[i].attrID= config_reporting->attr_list[i].attr_id;
		printf("attr_id:0x%04x\n",CfgReportCmd.attrList[i].attrID);
		CfgReportCmd.attrList[i].dataType= config_reporting->attr_list[i].data_type;
		printf("data_type:0x%02x\n",CfgReportCmd.attrList[i].dataType);
		CfgReportCmd.attrList[i].minReportInt= config_reporting->attr_list[i].min_interval;
		printf("min_interval:0x%04x\n",CfgReportCmd.attrList[i].minReportInt);
		CfgReportCmd.attrList[i].maxReportInt= config_reporting->attr_list[i].max_interval;
		printf("max_interval:0x%04x\n",CfgReportCmd.attrList[i].maxReportInt);
		CfgReportCmd.attrList[i].timeoutPeriod = config_reporting->attr_list[i].timeout_period;
		printf("timeout_period:0x%04x\n",CfgReportCmd.attrList[i].timeoutPeriod);	
		if(zclAnalogDataType(config_reporting->attr_list[i].data_type)) {
			CfgReportCmd.attrList[i].reportableChange= config_reporting->attr_list[i].reprotable_change;
			printf("temp is 0x%02x%02x\n", config_reporting->attr_list[i].reprotable_change[1], config_reporting->attr_list[i].reprotable_change[0]);
		}
	}
	
	struct device *d = gateway_getdevice(getgateway(),config_reporting->ieee);
	if(d) {
		zcl_SendConfigReportCmd(2, config_reporting->endpoint, d->shortaddr,config_reporting->clusterid, &CfgReportCmd,
                               ZCL_FRAME_CLIENT_SERVER_DIR, 0, config_reporting->serialnum);
	}
	for(i = 0; i < config_reporting->attr_num; i++) {
		if(config_reporting->attr_list[i].reprotable_change)
			free(config_reporting->attr_list[i].reprotable_change);
	}
}

#if 0
extern volatile int access_period;

void zcl_down_cmd_get_dstatus(struct protocol_cmdtype_get_device_status* get_dstatus)
{
	zclReadCmd_t readcmd; 
	struct device *d = gateway_getdevice(getgateway(), get_dstatus->ieee);
	if(d) {
		readcmd.numAttr = 1;
		readcmd.attrID[0] = ATTRID_BASIC_ZCL_VERSION;
		//printf("endpoint:%02x\n", get_dstatus->endpoint);
		access_period = get_dstatus->period;
		if(0 == access_period)
			zcl_SendRead(1, 1, d->shortaddr, ZCL_CLUSTER_ID_GEN_BASIC, &readcmd, ZCL_CLUSTER_ID_GEN_BASIC,0, get_dstatus->serialnum);
	}
}
#endif

void zcl_down_cmd_get_onoff(struct protocol_cmdtype_get_onoff_state* onoff_attr)
{
	zclReadCmd_t readcmd; 
	struct device *d = gateway_getdevice(getgateway(), onoff_attr->ieee);
	if(d) {
		readcmd.numAttr = 1;
		readcmd.attrID[0] = ATTRID_BASIC_ZCL_VERSION;
		//printf("endpoint:%02x\n", get_dstatus->endpoint);
		//zcl_SendRead(1, onoff_attr->endpoint, d->shortaddr, ZCL_CLUSTER_ID_GEN_ON_OFF, &readcmd, ZCL_CLUSTER_ID_GEN_BASIC,0,get_sequence());
		zcl_SendRead(1, onoff_attr->endpoint, d->shortaddr, ZCL_CLUSTER_ID_GEN_ON_OFF, &readcmd, ZCL_CLUSTER_ID_GEN_BASIC,0,onoff_attr->serialnum);
	}
}


