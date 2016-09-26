#include <stdlib.h>
#include <string.h>
#include "mtAf.h"
#include "commands.h"
#include "zcl.h"
#include "zcl_register_cluster.h"

int zcl_register_cluster(unsigned char in_cluster_count, unsigned short * in_cluster,unsigned char out_cluster_count, unsigned short * out_cluster, unsigned char endpoint, unsigned short appdevicetypeid);


int zcl_register_cluster_ss(){
	unsigned char in_cluster_count = 12;
	unsigned short in_cluster[MAX_CLUSTER_COUNT];
	memset(in_cluster, 0, MAX_CLUSTER_COUNT * sizeof(unsigned short));
	in_cluster[0] = ZCL_CLUSTER_ID_GEN_BASIC;
	in_cluster[1] = ZCL_CLUSTER_ID_GEN_POWER_CFG;
	in_cluster[2] = ZCL_CLUSTER_ID_GEN_IDENTIFY;
	in_cluster[3] = ZCL_CLUSTER_ID_GEN_ON_OFF;
	in_cluster[4] = ZCL_CLUSTER_ID_GEN_COMMISSIONING;
	in_cluster[5] = ZCL_CLUSTER_ID_GEN_POLL_CONTROL;
	in_cluster[6] = ZCL_CLUSTER_ID_HA_DIAGNOSTIC;
	in_cluster[7] = ZCL_CLUSTER_ID_SS_IAS_ZONE;	
	in_cluster[8] = ZCL_CLUSTER_ID_SS_IAS_ACE;
	in_cluster[9] = ZCL_CLUSTER_ID_SS_IAS_WD;
	in_cluster[10] = ZCL_CLUSTER_ID_GEN_ON_OFF_SWITCH_CONFIG;
	in_cluster[11] = ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT;

	unsigned char out_cluster_count = 13;
	unsigned short out_cluster[MAX_CLUSTER_COUNT];
	memset(out_cluster, 0, MAX_CLUSTER_COUNT * sizeof(unsigned short));
	out_cluster[0] = ZCL_CLUSTER_ID_GEN_BASIC;
	out_cluster[1] = ZCL_CLUSTER_ID_GEN_POWER_CFG;
	out_cluster[2] = ZCL_CLUSTER_ID_GEN_IDENTIFY;
	out_cluster[3] = ZCL_CLUSTER_ID_GEN_ON_OFF;
	out_cluster[4] = ZCL_CLUSTER_ID_GEN_COMMISSIONING;
	out_cluster[5] = ZCL_CLUSTER_ID_GEN_POLL_CONTROL;
	out_cluster[6] = ZCL_CLUSTER_ID_HA_DIAGNOSTIC;
	out_cluster[7] = ZCL_CLUSTER_ID_SS_IAS_ZONE;	
	out_cluster[8] = ZCL_CLUSTER_ID_SS_IAS_ACE;
	out_cluster[9] = ZCL_CLUSTER_ID_SS_IAS_WD;
	out_cluster[10] = ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT;
	out_cluster[11] = ZCL_CLUSTER_ID_GEN_ON_OFF_SWITCH_CONFIG;
	out_cluster[12] = ZCL_CLUSTER_ID_GEN_SCENES;


	zcl_register_cluster(in_cluster_count, in_cluster, out_cluster_count, out_cluster, APP_DEVICETYPEID_SS_ENDPOINT, APP_DEVICETYPEID_SS);

	return APP_DEVICETYPEID_SS_ENDPOINT;
}


int zcl_register_cluster(unsigned char in_cluster_count, unsigned short * in_cluster,unsigned char out_cluster_count, unsigned short * out_cluster, unsigned char endpoint, unsigned short appdevicetypeid)
{ 
	if(in_cluster_count > MAX_CLUSTER_COUNT || out_cluster_count > MAX_CLUSTER_COUNT){
		return 1;
	} 
	RegisterFormat_t req;
	memset(&req, 0, sizeof(RegisterFormat_t)); 
	req.EndPoint= endpoint;
	req.AppProfId = 0x0104;
	req.AppDeviceId = appdevicetypeid;
	req.AppNumInClusters = in_cluster_count;
	memcpy(req.AppInClusterList, in_cluster, sizeof(unsigned short)*in_cluster_count);
	req.AppNumOutClusters = out_cluster_count;
	memcpy(req.AppOutClusterList, out_cluster, sizeof(unsigned short)*out_cluster_count);
	sendcmd((unsigned char *)&req, AF_REGISTER);

	return 0;
}

int zcl_register_cluster_closures()
{
	unsigned char in_cluster_count = 0;
	unsigned short in_cluster[MAX_CLUSTER_COUNT] = {0};

	unsigned char out_cluster_count = 11;
	unsigned short out_cluster[MAX_CLUSTER_COUNT];
	memset(out_cluster, 0, MAX_CLUSTER_COUNT * sizeof(unsigned short));
	out_cluster[0] = ZCL_CLUSTER_ID_GEN_BASIC;
	out_cluster[1] = ZCL_CLUSTER_ID_GEN_IDENTIFY;
	out_cluster[2] = ZCL_CLUSTER_ID_GEN_GROUPS;
	out_cluster[3] = ZCL_CLUSTER_ID_GEN_SCENES;
	out_cluster[4] = ZCL_CLUSTER_ID_GEN_ON_OFF;
	out_cluster[5] = ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL;
	out_cluster[6] = ZCL_CLUSTER_ID_GEN_COMMISSIONING;
	out_cluster[7] = ZCL_CLUSTER_ID_CLOSURES_SHADE_CONFIG;
	out_cluster[8] = ZCL_CLUSTER_ID_HA_DIAGNOSTIC;
	out_cluster[9] = ZCL_CLUSTER_ID_SE_METERING;
	out_cluster[10] = ZCL_CLUSTER_ID_HA_ELECTRICAL_MEASUREMENT;

	zcl_register_cluster(in_cluster_count, in_cluster, out_cluster_count, out_cluster, APP_DEVICETYPEID_CLOSURES_ENDPOINT, APP_DEVICETYPEID_CLOSURES);

	return APP_DEVICETYPEID_CLOSURES_ENDPOINT;
}

