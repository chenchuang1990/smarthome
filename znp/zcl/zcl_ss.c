#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

#include "sqlitedb.h"
#include "zcl_ss.h" 
#include "zcl_general.h" 
#include "zcl.h"
#include "mtParser.h"
#include "zcl_datatype.h"
#include "socket.h"
#include "gateway.h" 

extern int g_znpwfd;

#define NEW_PROTOCOL


/*******************************************************************************
 * MACROS
 */
#define zclSS_ZoneTypeSupported( a ) ( (a) == SS_IAS_ZONE_TYPE_STANDARD_CIE              || \
		(a) == SS_IAS_ZONE_TYPE_MOTION_SENSOR             || \
		(a) == SS_IAS_ZONE_TYPE_CONTACT_SWITCH            || \
		(a) == SS_IAS_ZONE_TYPE_FIRE_SENSOR               || \
		(a) == SS_IAS_ZONE_TYPE_WATER_SENSOR              || \
		(a) == SS_IAS_ZONE_TYPE_GAS_SENSOR                || \
		(a) == SS_IAS_ZONE_TYPE_PERSONAL_EMERGENCY_DEVICE || \
		(a) == SS_IAS_ZONE_TYPE_VIBRATION_MOVEMENT_SENSOR || \
		(a) == SS_IAS_ZONE_TYPE_REMOTE_CONTROL            || \
		(a) == SS_IAS_ZONE_TYPE_KEY_FOB                   || \
		(a) == SS_IAS_ZONE_TYPE_KEYPAD                    || \
		(a) == SS_IAS_ZONE_TYPE_STANDARD_WARNING_DEVICE   || \
		(a) == SS_IAS_ZONE_TYPE_GLASS_BREAK_SENSOR        || \
		(a) == SS_IAS_ZONE_TYPE_SECURITY_REPEATER         )

/*******************************************************************************
 * TYPEDEFS
 */
typedef struct zclSS_ZoneItem
{
	struct zclSS_ZoneItem   *next;
	uint8                   endpoint; // Used to link it into the endpoint descriptor
	IAS_ACE_ZoneTable_t     zone;     // Zone info
} zclSS_ZoneItem_t;

/*******************************************************************************
 * GLOBAL VARIABLES
 */
const uint8 zclSS_UknownIeeeAddress[8] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

/*******************************************************************************
 * GLOBAL FUNCTIONS
 */

/*******************************************************************************
 * LOCAL VARIABLES
 */
static zclSS_ZoneItem_t *zclSS_ZoneTable = (zclSS_ZoneItem_t *)NULL;

/*********************************************************************
 * @fn      zclSS_AddZone
 *
 * @brief   Add a zone for an endpoint
 *
 * @param   endpoint - endpoint of new zone
 * @param   zone - new zone item
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclSS_AddZone( uint8 endpoint, IAS_ACE_ZoneTable_t *zone )
{
	zclSS_ZoneItem_t *pNewItem;
	zclSS_ZoneItem_t *pLoop;

	// Fill in the new profile list
	pNewItem = (zclSS_ZoneItem_t *)malloc( sizeof( zclSS_ZoneItem_t ) );
	memset(pNewItem, 0, sizeof(zclSS_ZoneItem_t));
	if ( pNewItem == NULL )
	{
		return ( ZMemError );
	}

	// Fill in the plugin record.
	pNewItem->next = (zclSS_ZoneItem_t *)NULL;
	pNewItem->endpoint = endpoint;
	memcpy( (uint8*)&(pNewItem->zone), (uint8*)zone, sizeof ( IAS_ACE_ZoneTable_t ));

	// Find spot in list
	if (  zclSS_ZoneTable == NULL )
	{
		zclSS_ZoneTable = pNewItem;
	}
	else
	{
		// Look for end of list
		pLoop = zclSS_ZoneTable;
		while ( pLoop->next != NULL )
		{
			pLoop = pLoop->next;
		}

		// Put new item at end of list
		pLoop->next = pNewItem;
	}

	return ( 0 );
}

/*********************************************************************
 * @fn      zclSS_CountAllZones
 *
 * @brief   Count the total number of zones
 *
 * @param   none
 *
 * @return  number of zones
 */
uint8 zclSS_CountAllZones( void )
{
	zclSS_ZoneItem_t *pLoop;
	uint8 cnt = 0;

	// Look for end of list
	pLoop = zclSS_ZoneTable;
	while ( pLoop )
	{
		cnt++;
		pLoop = pLoop->next;
	}
	return ( cnt );
}

/*********************************************************************
 * @fn      zclSS_ZoneIDAvailable
 *
 * @brief   Check to see whether zoneID is available for use
 *
 * @param   zoneID - ID to look for zone
 *
 * @return  TRUE if zoneID is available, FALSE otherwise
 */
static uint8 zclSS_ZoneIDAvailable( uint8 zoneID )
{
	zclSS_ZoneItem_t *pLoop;

	if ( zoneID < ZCL_SS_MAX_ZONE_ID )
	{
		pLoop = zclSS_ZoneTable;
		while ( pLoop )
		{
			if ( pLoop->zone.zoneID == zoneID  )
			{
				return ( FALSE );
			}
			pLoop = pLoop->next;
		}

		// Zone ID not in use
		return ( TRUE );
	}

	return ( FALSE );
}
/*********************************************************************
 * @fn      zclSS_GetNextFreeZoneID
 *
 * @brief   Get the next free zone ID
 *
 * @param   none
 *
 * @return  free zone ID (0-ZCL_SS_MAX_ZONE_ID) ,
 *          (ZCL_SS_MAX_ZONE_ID + 1) if none is found (0xFF)
 */
static uint8 zclSS_GetNextFreeZoneID( void )
{
	static uint8 nextAvailZoneID = 0;

	if ( zclSS_ZoneIDAvailable( nextAvailZoneID ) == FALSE )
	{
		uint8 zoneID = nextAvailZoneID;

		// Look for next available zone ID
		do
		{
			if ( ++zoneID > ZCL_SS_MAX_ZONE_ID )
			{
				zoneID = 0; // roll over
			}
		} while ( (zoneID != nextAvailZoneID) && (zclSS_ZoneIDAvailable( zoneID ) == FALSE) );

		// Did we found a free zone ID?
		if ( zoneID != nextAvailZoneID )
		{
			nextAvailZoneID = zoneID;
		}
		else
		{
			return ( ZCL_SS_MAX_ZONE_ID + 1 );
		}
	}

	return ( nextAvailZoneID );
}



/*******************************************************************************
 * @fn      zclSS_IAS_Send_ZoneStatusEnrollResponseCmd
 *
 * @brief   Call to send out a Enroll Response Command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   responseCode -  value of  response Code
 * @param   zoneID  - index to the zone table of the CIE
 * @param   disableDefaultRsp - toggle for enabling/disabling default response
 * @param   seqNum - command sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSS_IAS_Send_ZoneStatusEnrollResponseCmd( uint8 srcEP, uint8 dstEp, uint16 dstaddr,
		uint8 responseCode, uint8 zoneID,
		uint8 disableDefaultRsp, uint8 seqNum )
{
	uint8 buf[PAYLOAD_LEN_ZONE_STATUS_ENROLL_RSP];

	buf[0] = responseCode;
	buf[1] = zoneID;

	return zcl_sendcommand( srcEP, dstEp, dstaddr, ZCL_CLUSTER_ID_SS_IAS_ZONE,
			COMMAND_SS_IAS_ZONE_STATUS_ENROLL_RESPONSE, TRUE,
			ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
			seqNum, PAYLOAD_LEN_ZONE_STATUS_ENROLL_RSP, buf );
}

int zclss_processincmd_zonestatus_enrollrequest(struct zclincomingmsg * pInMsg){
	IAS_ACE_ZoneTable_t zone;
	ZStatus_t stat = -1;
	uint16 zoneType;
	//uint16 manuCode;
	uint8 responseCode;
	uint8 zoneID;

	printf("zclss_processincmd_zonestatus_enrollrequest\n");
	zoneType = BUILD_UINT16( pInMsg->data[0], pInMsg->data[1] );
	//manuCode = BUILD_UINT16( pInMsg->data[2], pInMsg->data[3] );
	if ( zclSS_ZoneTypeSupported( zoneType ) )
	{
		// Add zone to the table if space is available
		if ( ( zclSS_CountAllZones() < ZCL_SS_MAX_ZONES-1 ) &&
				( ( zoneID = zclSS_GetNextFreeZoneID() ) <= ZCL_SS_MAX_ZONE_ID ) )
		{
			zone.zoneID = zoneID;
			zone.zoneType = zoneType;

			// The application will fill in the right IEEE Address later
			//     zcl_cpyExtAddr( zone.zoneAddress, (void *)zclSS_UknownIeeeAddress );
			memcpy(zone.zoneAddress, &zclSS_UknownIeeeAddress, sizeof(zclSS_UknownIeeeAddress));

			if ( zclSS_AddZone( pInMsg->message->DstEndpoint, &zone ) == ZSuccess )
			{
				responseCode = ZSuccess;
			}
			else
			{
				// CIE does not permit new zones to enroll at this time
				responseCode = SS_IAS_ZONE_STATUS_ENROLL_RESPONSE_CODE_NO_ENROLL_PERMIT;
			}
		}
		else
		{
			// CIE reached its limit of number of enrolled zones
			responseCode = SS_IAS_ZONE_STATUS_ENROLL_RESPONSE_CODE_TOO_MANY_ZONES;
		}
	}
	else
	{
		// Zone type is not known to CIE and is not supported
		responseCode = SS_IAS_ZONE_STATUS_ENROLL_RESPONSE_CODE_NOT_SUPPORTED;
	}


	struct device * d = gateway_getdevice_shortaddr(pInMsg->message->SrcAddr);
	assert(d);
	device_set_zonetype(d, pInMsg->message->SrcEndpoint, zoneType); 

	/*暂时bu去掉上报enroll*/
	//#if 0
	struct zcl_zone_enroll_req_cmd enroll_cmd;
	memset(&enroll_cmd, 0, sizeof(struct zcl_zone_enroll_req_cmd));
	enroll_cmd.cmdid = ZCLZONEENROLLREQ;
	enroll_cmd.req.ieeeaddr = d->ieeeaddr;
	enroll_cmd.req.groupid = pInMsg->message->GroupId;
	enroll_cmd.req.clusterid = pInMsg->message->ClusterId;
	enroll_cmd.req.zonetype = zoneType;
	enroll_cmd.req.zoneid = zoneID;
	enroll_cmd.req.endpoint = pInMsg->message->SrcEndpoint;
	int n = write(g_znpwfd, &enroll_cmd, sizeof(struct zcl_zone_enroll_req_cmd));
	fprintf(stdout, "********send add new device %llX %d %d\n", enroll_cmd.req.ieeeaddr, n, sizeof(struct zcl_zone_enroll_req_cmd));
	assert(n == sizeof(struct zcl_zone_enroll_req_cmd));
	//#endif
	
	// if ( stat == ZSuccess )
	{
		// Send a response back
		stat = zclSS_IAS_Send_ZoneStatusEnrollResponseCmd( pInMsg->message->DstEndpoint, pInMsg->message->SrcEndpoint, pInMsg->message->SrcAddr, responseCode, zoneID, TRUE, pInMsg->zclframehdr.transseqnum );

		//return ( ZCL_STATUS_CMD_HAS_RSP );
	}
	// else
	{
		return ( stat );
	}
}

int zclss_processincmd_zonestatus_changenotification(struct zclincomingmsg * msg){

	struct zcl_zone_change_notification_cmd cmd;
	memset(&cmd, 0, sizeof(struct zcl_zone_change_notification_cmd));
	cmd.cmdid = ZCLZONECHANGENOTIFICATION;
	cmd.req.zonechangenotification.uszonestatus = BUILD_UINT16(msg->data[0],msg->data[1]);
	cmd.req.ext_status = msg->data[2];
	cmd.req.clusterid = msg->message->ClusterId;
	cmd.req.endpoint = msg->message->SrcEndpoint;
	cmd.req.trans_num = msg->zclframehdr.transseqnum;
	
	struct device * d = gateway_getdevice_shortaddr(msg->message->SrcAddr);
	if(!d){
		printf("no fucking device\n");
		return -1;
	}
	cmd.req.ieeeaddr = d->ieeeaddr;
			
	write(g_znpwfd, &cmd, sizeof(struct zcl_zone_change_notification_cmd));

	return 0;
}


/*
int zclss_processincmd_zonestatus_changenotification(struct zclincomingmsg * msg){

	struct zcl_zone_change_notification_cmd cmd;
	memset(&cmd, 0, sizeof(struct zcl_zone_change_notification_cmd));
	cmd.cmdid = ZCLZONECHANGENOTIFICATION;
	cmd.req.zonechangenotification.uszonestatus = BUILD_UINT16(msg->data[0],msg->data[1]);
	cmd.req.ext_status = msg->data[2];
	struct device * d = gateway_getdevice_shortaddr(msg->message->SrcAddr);
	struct endpoint *ep;
	if(d){
		cmd.req.ieeeaddr = d->ieeeaddr;
		cmd.req.clusterid = msg->message->ClusterId;
		cmd.req.endpoint = msg->message->SrcEndpoint;
		ep = gateway_get_endpoint(d->ieeeaddr, msg->message->SrcEndpoint);
		if(!ep) {
			printf("no fucking endpoint\n");
			return -1;
		}
		ep->zcl_transnum = msg->zclframehdr.transseqnum;
		printf("zclss_processincmd_zonestatus_changenotification:%llx\n", cmd.req.ieeeaddr);
		printf("zcl trans_num:%d\n", msg->zclframehdr.transseqnum);
	}

	write(g_znpwfd, &cmd, sizeof(struct zcl_zone_change_notification_cmd));

	return 0;
}
*/

int zclss_processinzonestatuscmdsserver(struct zclincomingmsg * msg){ 
	int result = -1;
	switch(msg->zclframehdr.commandid){
		case COMMAND_SS_IAS_ZONE_STATUS_ENROLL_RESPONSE:
			break;
		case COMMAND_SS_IAS_ZONE_STATUS_INIT_NORMAL_OP_MODE:
			break;
		case COMMAND_SS_IAS_ZONE_STATUS_INIT_TEST_MODE:
			break; 
		default:
			result = -1;
			break;
	}

	return result;
}

//for test
int zclgeneral_send_identify_query(struct zclincomingmsg *msg)
{
	int result = -1;
	IncomingMsgFormat_t *message = (IncomingMsgFormat_t *)msg->message;
	uint8 srcEP = message->DstEndpoint;
	uint8 dstEP = message->SrcEndpoint;
	uint16 dstAddr = message->SrcAddr;
	zclGeneral_SendIdentifyQuery(srcEP, dstEP, dstAddr,0, 0);
	return result;
}

int zclss_processinzonestatuscmdsclient(struct zclincomingmsg * msg){
	int result = -1;

	switch(msg->zclframehdr.commandid){
		case COMMAND_SS_IAS_ZONE_STATUS_CHANGE_NOTIFICATION:
			result = zclss_processincmd_zonestatus_changenotification(msg);
			break;
		case COMMAND_SS_IAS_ZONE_STATUS_ENROLL_REQUEST:
			result = zclss_processincmd_zonestatus_enrollrequest(msg);
			//IncomingMsgFormat_t *message = (IncomingMsgFormat_t *)msg->message;
			//            uint8 srcEP = message->DstEndpoint;
			//            uint8 dstEP = message->SrcEndpoint;
			//            uint16 dstAddr = message->SrcAddr;
			//            uint16 identifyTime = 0x0014;
			//                        zclGeneral_SendIdentify(srcEP, dstEP, dstAddr, identifyTime, 0, 0); 
			//            zclReadCmd_t cmd;
			//            cmd.numAttr = 1;
			//            cmd.attrID[0] = 0x0005;
			//            zcl_SendRead(srcEP,dstEP,dstAddr,ZCL_CLUSTER_ID_GEN_BASIC, &cmd, ZCL_FRAME_CLIENT_SERVER_DIR, 0,0 );
			break; 
		default:
			result = -1;
	}

	return result;
}

int zcl_pross_onoff_response(struct zclincomingmsg *msg)
{
	struct zcl_general_default_response_cmd cmd;
	cmd.cmdid = ZCLGENONOFFRSP;
	cmd.req.cmd_ind = msg->data[0];
	cmd.req.status= msg->data[1];
	struct device * d = gateway_getdevice_shortaddr(msg->message->SrcAddr);
	if(d){
		cmd.req.ieeeaddr = d->ieeeaddr;
		cmd.req.clusterid = msg->message->ClusterId;
		cmd.req.endpoint = msg->message->SrcEndpoint;
	}
	cmd.req.serialnum = msg->zclframehdr.transseqnum;

	write(g_znpwfd, &cmd, sizeof(struct zcl_general_default_response_cmd));

	return 0;
}

int zcl_pross_read_onoff_rsp(struct zclincomingmsg *msg)
{
	struct zcl_read_onoff_rsp_cmd cmd;
	
	cmd.cmdid = ZCLREADONOFFRSP;	
	cmd.req.state = msg->data[4];
	
	
	struct device * d = gateway_getdevice_shortaddr(msg->message->SrcAddr);
	if(d){
		cmd.req.ieeeaddr = d->ieeeaddr;
		cmd.req.endpoint = msg->message->SrcEndpoint;
	}
	cmd.req.serialnum = msg->zclframehdr.transseqnum;
	
	write(g_znpwfd, &cmd, sizeof(struct zcl_read_onoff_rsp_cmd));
	
	return 0;
}


int zcl_pross_default_response(struct zclincomingmsg *msg)
{
	struct zcl_general_default_response_cmd cmd;
	cmd.cmdid = ZCLGENERALDEFAULTRSP;
	cmd.req.cmd_ind = msg->data[0];
	cmd.req.status= msg->data[1];
	struct device * d = gateway_getdevice_shortaddr(msg->message->SrcAddr);
	if(d){
		cmd.req.ieeeaddr = d->ieeeaddr;
		cmd.req.clusterid = msg->message->ClusterId;
		cmd.req.endpoint = msg->message->SrcEndpoint;
	}

	write(g_znpwfd, &cmd, sizeof(struct zcl_general_default_response_cmd));

	return 0;
}

int zcl_processingeneralcmdsclient(struct zclincomingmsg *msg)
{
	int result = -1;
	switch(msg->zclframehdr.commandid) {
	case ZCL_CMD_DEFAULT_RSP:
		result = zcl_pross_default_response(msg);
		break;
	}

	return 0;
}

int zclss_handlespecificcommands( struct zclincomingmsg * msg){ 
	int result = -1;
	//printf("zclss_handlespecificcommands\n");
	switch(msg->message->ClusterId){
		case ZCL_CLUSTER_ID_SS_IAS_ZONE:
			if(zcl_ServerCmd(msg->zclframehdr.control.direction)){
				result = zclss_processinzonestatuscmdsserver(msg);
			}else{
				result = zclss_processinzonestatuscmdsclient(msg);
			}
			break;
	}
	return result;
}

int zclss_handlegeneralcommands( struct zclincomingmsg * msg){ 
	int result = -1;
	if(zcl_ServerCmd(msg->zclframehdr.control.direction)){
		//result = zclss_processingeneralcmdsserver(msg);
	}else{
		result = zcl_processingeneralcmdsclient(msg);
	}
	return result;
}


int zclss_handleincoming( struct zclincomingmsg * zclincomingmsg){
	int result = -1;
#if defined ( INTER_PAN )
	//todo
#endif
	if (zcl_ClusterCmd(zclincomingmsg->zclframehdr.control.type)){ 
		if(zclincomingmsg->zclframehdr.control.manuspecific == 0){ 
			zclss_handlespecificcommands(zclincomingmsg);
		}else{
			// We don't support any manufacturer specific command -- ignore it.  
			result = -1;
		}
	}else{
		result = -1;
	}

	return result;
}

int zclss_handle_default( struct zclincomingmsg * zclincomingmsg){
	int result = -1;
#if defined ( INTER_PAN )
	//todo
#endif
	if (zcl_ProfileCmd(zclincomingmsg->zclframehdr.control.type)){ 
		if(zclincomingmsg->zclframehdr.control.manuspecific == 0){ 
			zclss_handlegeneralcommands(zclincomingmsg);
		}else{
			// We don't support any manufacturer specific command -- ignore it.  
			result = -1;
		}
	}else{
		result = -1;
	}

	return result;
}

/*
int report_basic_status(struct zclincomingmsg *zclin)
{
	struct zcl_basic_status_cmd cmd;
	cmd.cmdid = ZCLBASICSTATUS;
	cmd.req.status= zclin->data[2];
	struct device * d = gateway_getdevice_shortaddr(zclin->message->SrcAddr);
	if(d){
		cmd.req.ieeeaddr = d->ieeeaddr;
	}

	//write(g_znpwfd, &cmd, sizeof(struct zcl_basic_status_cmd));

	return 0;
}
*/

int handle_basic_status(struct zclincomingmsg *zclin)
{
	struct zcl_basic_status_cmd cmd;
	cmd.cmdid = ZCLBASICSTATUS;
	//cmd.req.status= zclin->data[2];
	cmd.req.status= 1;
	struct device * d = gateway_getdevice_shortaddr(zclin->message->SrcAddr);
	if(d){
		cmd.req.ieeeaddr = d->ieeeaddr;
	}
	d->timestamp = time(NULL);
	if(d->status & DEVICE_APP_DEL) {
		d->status &= ~DEVICE_APP_DEL;
		sqlitedb_update_device_status(d);
	}

	//write(g_znpwfd, &cmd, sizeof(struct zcl_basic_status_cmd));

	return 0;
}

int handle_onoff_state(struct zclincomingmsg *zclin)
{
	switch(zclin->zclframehdr.commandid) {
	case ZCL_CMD_READ_RSP:
		zcl_pross_read_onoff_rsp(zclin);
			break;
	case ZCL_CMD_DEFAULT_RSP:
		zcl_pross_onoff_response(zclin);
		break;
	return 0;
	}
	
	return -1;
}

int handle_levelctr_rsp(struct zclincomingmsg *zclin)
{
	struct zcl_general_default_response_cmd cmd;
	cmd.cmdid = ZCLGENLEVELCTLRSP;
	cmd.req.cmd_ind = zclin->data[0];
	cmd.req.status= zclin->data[1];
	struct device * d = gateway_getdevice_shortaddr(zclin->message->SrcAddr);
	if(d){
		cmd.req.ieeeaddr = d->ieeeaddr;
		cmd.req.clusterid = zclin->message->ClusterId;
		cmd.req.endpoint = zclin->message->SrcEndpoint;
	}
	cmd.req.serialnum = zclin->zclframehdr.transseqnum;

	write(g_znpwfd, &cmd, sizeof(struct zcl_general_default_response_cmd));

	return 0;
}

int handle_warning_rsp(struct zclincomingmsg *zclin)
{
	printf("handle_warning_rsp\n");
	struct zcl_general_default_response_cmd cmd;
	cmd.cmdid = ZCLWARNINGRSP;
	cmd.req.cmd_ind = zclin->data[0];
	cmd.req.status= zclin->data[1];
	struct device * d = gateway_getdevice_shortaddr(zclin->message->SrcAddr);
	if(d){
		cmd.req.ieeeaddr = d->ieeeaddr;
		cmd.req.clusterid = zclin->message->ClusterId;
		cmd.req.endpoint = zclin->message->SrcEndpoint;
	}
	cmd.req.serialnum = zclin->zclframehdr.transseqnum;

	write(g_znpwfd, &cmd, sizeof(struct zcl_general_default_response_cmd));

	return 0;
}

int zclss_send_ias_wd_start_warning_cmd(unsigned char srcep, unsigned char dstep, unsigned short dstaddr, zclWDStartWarning_t *pWarning, unsigned char disabledefaultrsp, unsigned char seqnum){ 
	unsigned char buf[5];

	buf[0] = pWarning->warningmessage.warningbyte;
	buf[1] = LO_UINT16( pWarning->warningDuration );
	buf[2] = HI_UINT16( pWarning->warningDuration );
	buf[3] = pWarning->strobeDutyCycle;
	buf[4] = pWarning->strobeLevel;

	return zcl_sendcommand(srcep, dstep, dstaddr, ZCL_CLUSTER_ID_SS_IAS_WD, 
			COMMAND_SS_IAS_WD_START_WARNING,TRUE,
			ZCL_FRAME_CLIENT_SERVER_DIR,disabledefaultrsp, 0,
			seqnum, 5, buf);
}
