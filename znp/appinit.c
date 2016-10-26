/**************************************************************************************************
 * Filename:       cmdLine.c
 * Description:    This file contains cmdLine application.
 *
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 *
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/*********************************************************************
 * INCLUDES
 */
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>


#include "rpc.h"
#include "mtSys.h"
#include "mtZdo.h"
#include "mtAf.h"
#include "mtParser.h"
#include "mtSapi.h"
#include "rpcTransport.h"
#include "dbgPrint.h"
#include "commands.h"
#include "sqlitedb.h"
#include "list.h"

#include "zcl.h"
#include "gateway.h"
#include "zcl_down_cmd.h"
#include "zcl_register_cluster.h"
#include "protocol_cmdtype.h"
#include "zcl_datatype.h"
#include "zcl_general.h"
#include "zcl_ha.h"

#include "key.h"


#define consolePrint printf
#define consoleClearLn(); printf("%c[2K", 27);
#define consoleFlush(); fflush(stdout);


extern int g_znpwfd;

extern pthread_mutex_t state_lock;
extern pthread_cond_t state_wait; 
extern pthread_mutex_t big_mutex;


/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * TYPES
 */

/*********************************************************************
 * LOCAL VARIABLE
 */

//init ZDO device state
devStates_t devState = DEV_HOLD;
uint8_t gSrcEndPoint = 1;
uint8_t gDstEndPoint = 1;

/***********************************************************************/

/*********************************************************************
 * LOCAL FUNCTIONS
 */
//ZDO Callbacks
static uint8_t mtZdoStateChangeIndCb(uint8_t newDevState);
static uint8_t mtZdoGetLinkKeyCb(GetLinkKeySrspFormat_t *msg);
static uint8_t mtZdoNwkAddrRspCb(NwkAddrRspFormat_t *msg);
static uint8_t mtZdoIeeeAddrRspCb(IeeeAddrRspFormat_t *msg);
static uint8_t mtZdoNodeDescRspCb(NodeDescRspFormat_t *msg);
static uint8_t mtZdoPowerDescRspCb(PowerDescRspFormat_t *msg);
static uint8_t mtZdoSimpleDescRspCb(SimpleDescRspFormat_t *msg);
static uint8_t mtZdoActiveEpRspCb(ActiveEpRspFormat_t *msg);
static uint8_t mtZdoMatchDescRspCb(MatchDescRspFormat_t *msg);
static uint8_t mtZdoComplexDescRspCb(ComplexDescRspFormat_t *msg);
static uint8_t mtZdoUserDescRspCb(UserDescRspFormat_t *msg);
static uint8_t mtZdoUserDescConfCb(UserDescConfFormat_t *msg);
static uint8_t mtZdoServerDiscRspCb(ServerDiscRspFormat_t *msg);
static uint8_t mtZdoEndDeviceBindRspCb(EndDeviceBindRspFormat_t *msg);
static uint8_t mtZdoBindRspCb(BindRspFormat_t *msg);
static uint8_t mtZdoUnbindRspCb(UnbindRspFormat_t *msg);
static uint8_t mtZdoMgmtNwkDiscRspCb(MgmtNwkDiscRspFormat_t *msg);
static uint8_t mtZdoMgmtLqiRspCb(MgmtLqiRspFormat_t *msg);
static uint8_t mtZdoMgmtRtgRspCb(MgmtRtgRspFormat_t *msg);
static uint8_t mtZdoMgmtBindRspCb(MgmtBindRspFormat_t *msg);
static uint8_t mtZdoMgmtLeaveRspCb(MgmtLeaveRspFormat_t *msg);
static uint8_t mtZdoMgmtDirectJoinRspCb(MgmtDirectJoinRspFormat_t *msg);
static uint8_t mtZdoMgmtPermitJoinRspCb(MgmtPermitJoinRspFormat_t *msg);
static uint8_t mtZdoEndDeviceAnnceIndCb(EndDeviceAnnceIndFormat_t *msg);
static uint8_t mtZdoMatchDescRspSentCb(MatchDescRspSentFormat_t *msg);
static uint8_t mtZdoStatusErrorRspCb(StatusErrorRspFormat_t *msg);
static uint8_t mtZdoSrcRtgIndCb(SrcRtgIndFormat_t *msg);
static uint8_t mtZdoBeaconNotifyIndCb(BeaconNotifyIndFormat_t *msg);
static uint8_t mtZdoJoinCnfCb(JoinCnfFormat_t *msg);
static uint8_t mtZdoNwkDiscoveryCnfCb(NwkDiscoveryCnfFormat_t *msg);
static uint8_t mtZdoLeaveIndCb(LeaveIndFormat_t *msg);
static uint8_t mtZdoMsgCbIncomingCb(MsgCbIncomingFormat_t *msg);

//SYS Callbacks
//static uint8_t mtSysResetInd(uint8_t resetReason, uint8_t version[5]);
static uint8_t mtSysPingSrspCb(PingSrspFormat_t *msg);
static uint8_t mtSysGetExtAddrSrspCb(GetExtAddrSrspFormat_t *msg);
static uint8_t mtSysRamReadSrspCb(RamReadSrspFormat_t *msg);
static uint8_t mtSysResetIndCb(ResetIndFormat_t *msg);
static uint8_t mtSysVersionSrspCb(VersionSrspFormat_t *msg);
static uint8_t mtSysOsalNvReadSrspCb(OsalNvReadSrspFormat_t *msg);
static uint8_t mtSysOsalNvLengthSrspCb(OsalNvLengthSrspFormat_t *msg);
static uint8_t mtSysOsalTimerExpiredCb(OsalTimerExpiredFormat_t *msg);
static uint8_t mtSysStackTuneSrspCb(StackTuneSrspFormat_t *msg);
static uint8_t mtSysAdcReadSrspCb(AdcReadSrspFormat_t *msg);
static uint8_t mtSysGpioSrspCb(GpioSrspFormat_t *msg);
static uint8_t mtSysRandomSrspCb(RandomSrspFormat_t *msg);
static uint8_t mtSysGetTimeSrspCb(GetTimeSrspFormat_t *msg);
static uint8_t mtSysSetTxPowerSrspCb(SetTxPowerSrspFormat_t *msg);

//AF callbacks
static uint8_t mtAfDataConfirmCb(DataConfirmFormat_t *msg);
static uint8_t mtAfIncomingMsgCb(IncomingMsgFormat_t *msg);
static uint8_t mtAfIncomingMsgExt(IncomingMsgExtFormat_t *msg);
static uint8_t mtAfDataRetrieveSrspCb(DataRetrieveSrspFormat_t *msg);
static uint8_t mtAfReflectErrorCb(ReflectErrorFormat_t *msg);

//SAPI Callbacks
static uint8_t mtSapiReadConfigurationSrspCb(ReadConfigurationSrspFormat_t *msg);
static uint8_t mtSapiGetDeviceInfoSrspCb(GetDeviceInfoSrspFormat_t *msg);
static uint8_t mtSapiFindDeviceCnfCb(FindDeviceCnfFormat_t *msg);
static uint8_t mtSapiSendDataCnfCb(SendDataCnfFormat_t *msg);
static uint8_t mtSapiReceiveDataIndCb(ReceiveDataIndFormat_t *msg);
static uint8_t mtSapiAllowBindCnfCb(AllowBindCnfFormat_t *msg);
static uint8_t mtSapiBindCnfCb(BindCnfFormat_t *msg);
static uint8_t mtSapiStartCnfCb(StartCnfFormat_t *msg);

//helper functions
static uint8_t setNVStartup(uint8_t startupOption);
static uint8_t setNVChanList(uint32_t chanList);
static uint8_t setNVPanID(uint32_t panId);
static uint8_t setNVDevType(uint8_t devType);
static int32_t startNetwork(void);
static int32_t registerAf(void);



/*********************************************************************
 * CALLBACK FUNCTIONS
 */

// SYS callbacks
static mtSysCb_t mtSysCb =
{ mtSysPingSrspCb, mtSysGetExtAddrSrspCb, mtSysRamReadSrspCb,
	mtSysResetIndCb, mtSysVersionSrspCb, mtSysOsalNvReadSrspCb,
	mtSysOsalNvLengthSrspCb, mtSysOsalTimerExpiredCb,
	mtSysStackTuneSrspCb, mtSysAdcReadSrspCb, mtSysGpioSrspCb,
	mtSysRandomSrspCb, mtSysGetTimeSrspCb, mtSysSetTxPowerSrspCb };

static mtZdoCb_t mtZdoCb =
{ mtZdoNwkAddrRspCb,       // MT_ZDO_NWK_ADDR_RSP
	mtZdoIeeeAddrRspCb,      // MT_ZDO_IEEE_ADDR_RSP
	mtZdoNodeDescRspCb,      // MT_ZDO_NODE_DESC_RSP
	mtZdoPowerDescRspCb,     // MT_ZDO_POWER_DESC_RSP
	mtZdoSimpleDescRspCb,    // MT_ZDO_SIMPLE_DESC_RSP
	mtZdoActiveEpRspCb,      // MT_ZDO_ACTIVE_EP_RSP
	mtZdoMatchDescRspCb,     // MT_ZDO_MATCH_DESC_RSP
	mtZdoComplexDescRspCb,   // MT_ZDO_COMPLEX_DESC_RSP
	mtZdoUserDescRspCb,      // MT_ZDO_USER_DESC_RSP
	mtZdoUserDescConfCb,     // MT_ZDO_USER_DESC_CONF
	mtZdoServerDiscRspCb,    // MT_ZDO_SERVER_DISC_RSP
	mtZdoEndDeviceBindRspCb, // MT_ZDO_END_DEVICE_BIND_RSP
	mtZdoBindRspCb,          // MT_ZDO_BIND_RSP
	mtZdoUnbindRspCb,        // MT_ZDO_UNBIND_RSP
	mtZdoMgmtNwkDiscRspCb,   // MT_ZDO_MGMT_NWK_DISC_RSP
	mtZdoMgmtLqiRspCb,       // MT_ZDO_MGMT_LQI_RSP
	mtZdoMgmtRtgRspCb,       // MT_ZDO_MGMT_RTG_RSP
	mtZdoMgmtBindRspCb,      // MT_ZDO_MGMT_BIND_RSP
	mtZdoMgmtLeaveRspCb,     // MT_ZDO_MGMT_LEAVE_RSP
	mtZdoMgmtDirectJoinRspCb,     // MT_ZDO_MGMT_DIRECT_JOIN_RSP
	mtZdoMgmtPermitJoinRspCb,     // MT_ZDO_MGMT_PERMIT_JOIN_RSP
	mtZdoStateChangeIndCb,   // MT_ZDO_STATE_CHANGE_IND
	mtZdoEndDeviceAnnceIndCb,   // MT_ZDO_END_DEVICE_ANNCE_IND
	mtZdoSrcRtgIndCb,        // MT_ZDO_SRC_RTG_IND
	mtZdoBeaconNotifyIndCb,	 //MT_ZDO_BEACON_NOTIFY_IND
	mtZdoJoinCnfCb,			 //MT_ZDO_JOIN_CNF
	mtZdoNwkDiscoveryCnfCb,	 //MT_ZDO_NWK_DISCOVERY_CNF
	NULL,                    // MT_ZDO_CONCENTRATOR_IND_CB
	mtZdoLeaveIndCb,         // MT_ZDO_LEAVE_IND
	mtZdoStatusErrorRspCb,   //MT_ZDO_STATUS_ERROR_RSP
	mtZdoMatchDescRspSentCb,  //MT_ZDO_MATCH_DESC_RSP_SENT
	mtZdoMsgCbIncomingCb, mtZdoGetLinkKeyCb };

static mtAfCb_t mtAfCb =
{ mtAfDataConfirmCb,				//MT_AF_DATA_CONFIRM
	mtAfIncomingMsgCb,				//MT_AF_INCOMING_MSG
	mtAfIncomingMsgExt,				//MT_AF_INCOMING_MSG_EXT
	mtAfDataRetrieveSrspCb,			//MT_AF_DATA_RETRIEVE
	mtAfReflectErrorCb,			    //MT_AF_REFLECT_ERROR
};

// SAPI callbacks
static mtSapiCb_t mtSapiCb =
{ mtSapiReadConfigurationSrspCb,				//MT_SAPI_READ_CONFIGURATION
	mtSapiGetDeviceInfoSrspCb,				//MT_SAPI_GET_DEVICE_INFO
	mtSapiFindDeviceCnfCb,				//MT_SAPI_FIND_DEVICE_CNF
	mtSapiSendDataCnfCb,				//MT_SAPI_SEND_DATA_CNF
	mtSapiReceiveDataIndCb,				//MT_SAPI_RECEIVE_DATA_IND
	mtSapiAllowBindCnfCb,				//MT_SAPI_ALLOW_BIND_CNF
	mtSapiBindCnfCb,				//MT_SAPI_BIND_CNF
	mtSapiStartCnfCb,				//MT_SAPI_START_CNF
};

/********************************************************************
 * START OF SYS CALL BACK FUNCTIONS
 */

static uint8_t mtSysPingSrspCb(PingSrspFormat_t *msg)
{
	consolePrint("mtSysPingSrspCb\n");
	consolePrint("Capabilities: 0x%04X\n", msg->Capabilities);
	return 0;
}
static uint8_t mtSysGetExtAddrSrspCb(GetExtAddrSrspFormat_t *msg)
{
	consolePrint("mtSysGetExtAddrSrspCb\n");
	consolePrint("ExtAddr: 0x%016llX\n", (long long unsigned int) msg->ExtAddr);
	return 0;
}
static uint8_t mtSysRamReadSrspCb(RamReadSrspFormat_t *msg)
{
	consolePrint("mtSysRamReadSrspCb\n");
	if (msg->Status == MT_RPC_SUCCESS)
	{
		consolePrint("Status: 0x%02X\n", msg->Status);
		consolePrint("Len: 0x%02X\n", msg->Len);
		uint32_t i;
		for (i = 0; i < msg->Len; i++)
		{
			consolePrint("Value[%d]: 0x%02X\n", i, msg->Value[i]);
		}
	}
	else
	{
		consolePrint("RamReadSrsp Status: FAIL 0x%02X\n", msg->Status);
	}
	return msg->Status;
}
static uint8_t mtSysResetIndCb(ResetIndFormat_t *msg)
{
	consolePrint("ZNP Version: %d.%d.%d\n", msg->MajorRel, msg->MinorRel,
			msg->HwRev);
	return 0;
}
static uint8_t mtSysVersionSrspCb(VersionSrspFormat_t *msg)
{
	consolePrint("mtSysVersionSrspCb\n");
	consolePrint("TransportRev: 0x%02X\n", msg->TransportRev);
	consolePrint("Product: 0x%02X\n", msg->Product);
	consolePrint("MajorRel: 0x%02X\n", msg->MajorRel);
	consolePrint("MinorRel: 0x%02X\n", msg->MinorRel);
	consolePrint("MaintRel: 0x%02X\n", msg->MaintRel);
	return 0;
}
static uint8_t mtSysOsalNvReadSrspCb(OsalNvReadSrspFormat_t *msg)
{
	consolePrint("mtSysOsalNvReadSrspCb\n");
	if (msg->Status == MT_RPC_SUCCESS)
	{
		consolePrint("Status: 0x%02X\n", msg->Status);
		consolePrint("Len: 0x%02X\n", msg->Len);
		uint32_t i;
		for (i = 0; i < msg->Len; i++)
		{
			consolePrint("Value[%d]: 0x%02X\n", i, msg->Value[i]);
		}
	}
	else
	{
		consolePrint("OsalNvReadSrsp Status: FAIL 0x%02X\n", msg->Status);
	}
	return msg->Status;
}
static uint8_t mtSysOsalNvLengthSrspCb(OsalNvLengthSrspFormat_t *msg)
{
	consolePrint("mtSysOsalNvLengthSrspCb\n");
	consolePrint("ItemLen: 0x%04X\n", msg->ItemLen);
	return 0;
}
static uint8_t mtSysOsalTimerExpiredCb(OsalTimerExpiredFormat_t *msg)
{
	consolePrint("mtSysOsalTimerExpiredCb\n");
	consolePrint("Id: 0x%02X\n", msg->Id);
	return 0;
}
static uint8_t mtSysStackTuneSrspCb(StackTuneSrspFormat_t *msg)
{
	consolePrint("mtSysStackTuneSrspCb\n");
	consolePrint("Value: 0x%02X\n", msg->Value);
	return 0;
}
static uint8_t mtSysAdcReadSrspCb(AdcReadSrspFormat_t *msg)
{
	consolePrint("mtSysAdcReadSrspCb\n");
	consolePrint("Value: 0x%04X\n", msg->Value);
	return 0;
}
static uint8_t mtSysGpioSrspCb(GpioSrspFormat_t *msg)
{
	consolePrint("mtSysGpioSrspCb\n");
	consolePrint("Value: 0x%02X\n", msg->Value);
	return 0;
}
static uint8_t mtSysRandomSrspCb(RandomSrspFormat_t *msg)
{
	consolePrint("mtSysRandomSrspCb\n");
	consolePrint("Value: 0x%04X\n", msg->Value);
	return 0;
}
static uint8_t mtSysGetTimeSrspCb(GetTimeSrspFormat_t *msg)
{
	consolePrint("mtSysGetTimeSrspCb\n");
	consolePrint("UTCTime: 0x%08X\n", msg->UTCTime);
	consolePrint("Hour: 0x%02X\n", msg->Hour);
	consolePrint("Minute: 0x%02X\n", msg->Minute);
	consolePrint("Second: 0x%02X\n", msg->Second);
	consolePrint("Month: 0x%02X\n", msg->Month);
	consolePrint("Day: 0x%02X\n", msg->Day);
	consolePrint("Year: 0x%04X\n", msg->Year);
	return 0;
}
static uint8_t mtSysSetTxPowerSrspCb(SetTxPowerSrspFormat_t *msg)
{
	consolePrint("mtSysSetTxPowerSrspCb\n");
	consolePrint("TxPower: 0x%02X\n", msg->TxPower);
	return 0;
}
/********************************************************************
 * END OF SYS CALL BACK FUNCTIONS
 */

/********************************************************************
 * START OF ZDO CALL BACK FUNCTIONS
 */

/********************************************************************
 * @fn     Callback function for ZDO State Change Indication

 * @brief  receives the AREQ status and specifies the change ZDO state
 *
 * @param  uint8 zdoState
 *
 * @return SUCCESS or FAILURE
 */
static uint8_t mtZdoStateChangeIndCb(uint8_t newDevState)
{
	switch ((devStates_t) newDevState)
	{
		case DEV_HOLD:
			dbg_print(PRINT_LEVEL_INFO,
					"mtZdoStateChangeIndCb: Initialized - not started automatically\n");
			break;
		case DEV_INIT:
			dbg_print(PRINT_LEVEL_INFO,
					"mtZdoStateChangeIndCb: Initialized - not connected to anything\n");
			break;
		case DEV_NWK_DISC:
			dbg_print(PRINT_LEVEL_INFO,
					"mtZdoStateChangeIndCb: Discovering PAN's to join\n");
			consolePrint("Network Discovering\n");
			break;
		case DEV_NWK_JOINING:
			dbg_print(PRINT_LEVEL_INFO, "mtZdoStateChangeIndCb: Joining a PAN\n");
			consolePrint("Network Joining\n");
			break;
		case DEV_NWK_REJOIN:
			dbg_print(PRINT_LEVEL_INFO,
					"mtZdoStateChangeIndCb: ReJoining a PAN, only for end devices\n");
			consolePrint("Network Rejoining\n");
			break;
		case DEV_END_DEVICE_UNAUTH:
			consolePrint("Network Authenticating\n");
			dbg_print(PRINT_LEVEL_INFO,
					"mtZdoStateChangeIndCb: Joined but not yet authenticated by trust center\n");
			break;
		case DEV_END_DEVICE:
			consolePrint("Network Joined\n");
			dbg_print(PRINT_LEVEL_INFO,
					"mtZdoStateChangeIndCb: Started as device after authentication\n");
			break;
		case DEV_ROUTER:
			consolePrint("Network Joined\n");
			dbg_print(PRINT_LEVEL_INFO,
					"mtZdoStateChangeIndCb: Device joined, authenticated and is a router\n");
			break;
		case DEV_COORD_STARTING:
			consolePrint("Network Starting\n");
			dbg_print(PRINT_LEVEL_INFO,
					"mtZdoStateChangeIndCb: Started as Zigbee Coordinator\n");
			break;
		case DEV_ZB_COORD:
			consolePrint("Network Started\n");
			dbg_print(PRINT_LEVEL_INFO,
					"mtZdoStateChangeIndCb: Started as Zigbee Coordinator\n");
			set_led_onoff(LED_Z, LED_ON);
			//sleep(5);
			#if 0
			printf("newDevState is %d\n", newDevState);
			pthread_mutex_lock(&state_lock);
			printf("pthread_cond_signal...\n");
			pthread_cond_signal(&state_wait);
			printf("pthread_cond_signal end\n");
			pthread_mutex_unlock(&state_lock);
			#endif
			break;
		case DEV_NWK_ORPHAN:
			consolePrint("Network Orphaned\n");
			dbg_print(PRINT_LEVEL_INFO,
					"mtZdoStateChangeIndCb: Device has lost information about its parent\n");
			break;
		default:
			dbg_print(PRINT_LEVEL_INFO, "mtZdoStateChangeIndCb: unknown state");
			break;
	}

	devState = (devStates_t) newDevState;

	return SUCCESS;
}

static uint8_t mtZdoGetLinkKeyCb(GetLinkKeySrspFormat_t *msg)
{

	consolePrint("mtZdoGetLinkKeyCb\n");
	if (msg->Status == MT_RPC_SUCCESS)
	{
		consolePrint("Status: 0x%02X\n", msg->Status);
		consolePrint("IEEEAddr: 0x%016llX\n",
				(long long unsigned int) msg->IEEEAddr);
	}
	else
	{
		consolePrint("GetLinkKey Status: FAIL 0x%02X\n", msg->Status);
	}

	return msg->Status;
}
static uint8_t mtZdoNwkAddrRspCb(NwkAddrRspFormat_t *msg)
{
	consolePrint("mtZdoNwkAddrRspCb\n");
	if (msg->Status == MT_RPC_SUCCESS)
	{
		consolePrint("Status: 0x%02X\n", msg->Status);
		consolePrint("IEEEAddr: 0x%016llX\n",
				(long long unsigned int) msg->IEEEAddr);
		consolePrint("NwkAddr: 0x%04X\n", msg->NwkAddr);
		consolePrint("StartIndex: 0x%02X\n", msg->StartIndex);
		consolePrint("NumAssocDev: 0x%02X\n", msg->NumAssocDev);
		uint32_t i;
		for (i = 0; i < msg->NumAssocDev; i++)
		{
			consolePrint("AssocDevList[%d]: 0x%04X\n", i, msg->AssocDevList[i]);
		}
	}
	else
	{
		consolePrint("NwkAddrRsp Status: FAIL 0x%02X\n", msg->Status);
	}

	return msg->Status;
}
static uint8_t mtZdoIeeeAddrRspCb(IeeeAddrRspFormat_t *msg)
{
	consolePrint("mtZdoIeeeAddrRspCb\n");
	if (msg->Status == MT_RPC_SUCCESS)
	{
		consolePrint("Status: 0x%02X\n", msg->Status);
		consolePrint("IEEEAddr: 0x%016llX\n",
				(long long unsigned int) msg->IEEEAddr);
		//consolePrint("%08X\n", msg -> IEEEAddr);
		consolePrint("NwkAddr: 0x%04X\n", msg->NwkAddr);
		consolePrint("StartIndex: 0x%02X\n", msg->StartIndex);
		consolePrint("NumAssocDev: 0x%02X\n", msg->NumAssocDev);
		uint32_t i;
		for (i = 0; i < msg->NumAssocDev; i++)
		{
			consolePrint("AssocDevList[%d]: 0x%04X\n", i, msg->AssocDevList[i]);
		}
	}
	else
	{
		consolePrint("IeeeAddrRsp Status: FAIL 0x%02X\n", msg->Status);
	}

	return msg->Status;
}
static uint8_t mtZdoNodeDescRspCb(NodeDescRspFormat_t *msg)
{
	consolePrint("mtZdoNodeDescRspCb\n");
	if (msg->Status == MT_RPC_SUCCESS)
	{
		consolePrint("SrcAddr: 0x%04X\n", msg->SrcAddr);
		consolePrint("Status: 0x%02X\n", msg->Status);
		consolePrint("NwkAddr: 0x%04X\n", msg->NwkAddr);
		consolePrint("LoTy_ComDescAv_UsrDesAv: 0x%02X\n",
				msg->LoTy_ComDescAv_UsrDesAv);
		consolePrint("APSFlg_FrqBnd: 0x%02X\n", msg->APSFlg_FrqBnd);
		consolePrint("MACCapFlg: 0x%02X\n", msg->MACCapFlg);
		consolePrint("ManufacturerCode: 0x%04X\n", msg->ManufacturerCode);
		consolePrint("MaxBufferSize: 0x%02X\n", msg->MaxBufferSize);
		consolePrint("MaxTransferSize: 0x%04X\n", msg->MaxTransferSize);
		consolePrint("ServerMask: 0x%04X\n", msg->ServerMask);
		consolePrint("MaxOutTransferSize: 0x%04X\n", msg->MaxOutTransferSize);
		consolePrint("DescriptorCapabilities: 0x%02X\n",
				msg->DescriptorCapabilities);
	}
	else
	{
		consolePrint("NodeDescRsp Status: FAIL 0x%02X\n", msg->Status);
	}

	return msg->Status;
}
static uint8_t mtZdoPowerDescRspCb(PowerDescRspFormat_t *msg)
{
	consolePrint("mtZdoPowerDescRspCb\n");
	if (msg->Status == MT_RPC_SUCCESS)
	{
		consolePrint("SrcAddr: 0x%04X\n", msg->SrcAddr);
		consolePrint("Status: 0x%02X\n", msg->Status);
		consolePrint("NwkAddr: 0x%04X\n", msg->NwkAddr);
		consolePrint("CurrntPwrMode_AvalPwrSrcs: 0x%02X\n",
				msg->CurrntPwrMode_AvalPwrSrcs);
		consolePrint("CurrntPwrSrc_CurrntPwrSrcLvl: 0x%02X\n",
				msg->CurrntPwrSrc_CurrntPwrSrcLvl);
	}
	else
	{
		consolePrint("PowerDescRsp Status: FAIL 0x%02X\n", msg->Status);
	}

	return msg->Status;
}

void report_new_device(struct device * d)
{
	struct zcl_zone_enroll_req_cmd enroll_cmd;
	memset(&enroll_cmd, 0, sizeof(struct zcl_zone_enroll_req_cmd));
	enroll_cmd.cmdid = ZCLZONEENROLLREQ;
	enroll_cmd.req.ieeeaddr = d->ieeeaddr;
	int n = write(g_znpwfd, &enroll_cmd, sizeof(struct zcl_zone_enroll_req_cmd));	
	fprintf(stdout, "********send add new device %llX %d %d\n", enroll_cmd.req.ieeeaddr, n, sizeof(struct zcl_zone_enroll_req_cmd));
}

static void send_read_attr_cmd(struct device *d)
{
	zclReadCmd_t readcmd; 
	readcmd.numAttr = 8;
	readcmd.attrID[0] = ATTRID_BASIC_ZCL_VERSION;
	readcmd.attrID[1] = ATTRID_BASIC_APPL_VERSION;
	readcmd.attrID[2] = ATTRID_BASIC_STACK_VERSION;
	readcmd.attrID[3] = ATTRID_BASIC_HW_VERSION;
	readcmd.attrID[4] = ATTRID_BASIC_MANUFACTURER_NAME;
	readcmd.attrID[5] = ATTRID_BASIC_MODEL_ID;
	readcmd.attrID[6] = ATTRID_BASIC_DATE_CODE;         
	readcmd.attrID[7] = ATTRID_BASIC_POWER_SOURCE;

	struct endpoint *ep = list_entry(d->eplisthead.next, struct endpoint, list);
	if(ep) {
		printf("send_read_attr_cmd:send read attribute cmd\n");
		zcl_SendRead(1, ep->simpledesc.simpledesc.Endpoint, d->shortaddr, ZCL_CLUSTER_ID_GEN_BASIC, &readcmd, ZCL_CLUSTER_ID_GEN_BASIC, 0, 0);
	}
}

static uint8_t mtZdoSimpleDescRspCb(SimpleDescRspFormat_t *msg)
{
	consolePrint("mtZdoSimpleDescRspCb\n");
	if (msg->Status == MT_RPC_SUCCESS)
	{
		consolePrint("SrcAddr: 0x%04X\n", msg->SrcAddr);
		consolePrint("Status: 0x%02X\n", msg->Status);
		consolePrint("NwkAddr: 0x%04X\n", msg->NwkAddr);
		consolePrint("Len: 0x%02X\n", msg->Len);
		consolePrint("Endpoint: 0x%02X\n", msg->Endpoint);
		consolePrint("ProfileID: 0x%04X\n", msg->ProfileID);
		consolePrint("DeviceID: 0x%04X\n", msg->DeviceID);
		consolePrint("DeviceVersion: 0x%02X\n", msg->DeviceVersion);
		consolePrint("NumInClusters: 0x%02X\n", msg->NumInClusters);
		uint32_t i;

		pthread_mutex_lock(&big_mutex);
		printf("[mtZdoSimpleDescRspCb] lock\n");
		struct device * d = gateway_getdevice_shortaddr(msg->SrcAddr);
		for (i = 0; i < msg->NumInClusters; i++)
		{
			consolePrint("InClusterList[%d]: 0x%04X\n", i,
					msg->InClusterList[i]);
		}
		consolePrint("NumOutClusters: 0x%02X\n", msg->NumOutClusters);
		for (i = 0; i < msg->NumOutClusters; i++)
		{
			consolePrint("OutClusterList[%d]: 0x%04X\n", i,
					msg->OutClusterList[i]);
		}
		
		if(d && !device_has_enpoint(d, msg->Endpoint)){ 
			device_increase(d);
		}

		if(d && (d->epcursor < d->activeep.ActiveEPCount)){
			struct simpledesc sc;
			memset(&sc, 0, sizeof(struct simpledesc));
			memcpy(&sc.simpledesc, msg, sizeof(SimpleDescRspFormat_t));

			struct endpoint * ep = endpoint_create(&sc);
			device_addendpoint(d, ep);

			SimpleDescReqFormat_t req;
			req.DstAddr = msg->SrcAddr;
			req.NwkAddrOfInterest = msg->NwkAddr;
			req.Endpoint = d->activeep.ActiveEPList[d->epcursor];
			sendcmd((unsigned char *)&req,ZDO_SIMPLE_DESC_REQ);

		}else if(d && (d->epcursor == d->activeep.ActiveEPCount)){
			struct simpledesc sc;
			memset(&sc, 0, sizeof(struct simpledesc));
			memcpy(&sc.simpledesc, msg, sizeof(SimpleDescRspFormat_t));

			struct endpoint * ep = endpoint_create(&sc);
			device_addendpoint(d, ep);

			sqlitedb_update_device_endpoint(d);
			device_set_status(d, DEVICE_GET_SIMPLEDESC);
			report_new_device(d);
			device_set_status(d, DEVICE_ACTIVE);
			send_read_attr_cmd(d);
		}
		printf("[mtZdoSimpleDescRspCb] unlock\n");
		pthread_mutex_unlock(&big_mutex);
	}
	else
	{
		consolePrint("SimpleDescRsp Status: FAIL 0x%02X\n", msg->Status);
	}

	return msg->Status;
}

static uint8_t mtZdoActiveEpRspCb(ActiveEpRspFormat_t *msg)
{
	consolePrint("mtZdoActiveEpRspCb\n");
	if (msg->Status == MT_RPC_SUCCESS)
	{
		consolePrint("SrcAddr: 0x%04X\n", msg->SrcAddr);
		consolePrint("Status: 0x%02X\n", msg->Status);
		consolePrint("NwkAddr: 0x%04X\n", msg->NwkAddr);
		consolePrint("ActiveEPCount: 0x%02X\n", msg->ActiveEPCount);

		uint32_t i;
		for (i = 0; i < msg->ActiveEPCount; i++)
		{
			consolePrint("ActiveEPList[%d]: 0x%02X\n", i, msg->ActiveEPList[i]); 
		}

		pthread_mutex_lock(&big_mutex);
		printf("[mtZdoActiveEpRspCb] lock\n");
		struct device * d = gateway_getdevice_shortaddr(msg->SrcAddr);

		if(d && !device_check_status(d, DEVICE_SEND_SIMPLEDESC)){

			device_set_status(d, DEVICE_SEND_SIMPLEDESC);
			device_set_status(d, DEVICE_GET_ACTIVEEP);
			device_setep(d, msg);

			SimpleDescReqFormat_t req;
			req.DstAddr = msg->SrcAddr;
			req.NwkAddrOfInterest = msg->NwkAddr;
			req.Endpoint = msg->ActiveEPList[0];
			sendcmd((unsigned char *)&req,ZDO_SIMPLE_DESC_REQ);

			//			device_increase(d);
		}
		printf("[mtZdoActiveEpRspCb] unlock\n");
		pthread_mutex_unlock(&big_mutex);

	}
	else
	{
		consolePrint("ActiveEpRsp Status: FAIL 0x%02X\n", msg->Status);
	}

	return msg->Status;
}
static uint8_t mtZdoMatchDescRspCb(MatchDescRspFormat_t *msg)
{
	consolePrint("mtZdoMatchDescRspCb\n");
	if (msg->Status == MT_RPC_SUCCESS)
	{
		consolePrint("SrcAddr: 0x%04X\n", msg->SrcAddr);
		consolePrint("Status: 0x%02X\n", msg->Status);
		consolePrint("NwkAddr: 0x%04X\n", msg->NwkAddr);
		consolePrint("MatchLength: 0x%02X\n", msg->MatchLength);
		uint32_t i;
		for (i = 0; i < msg->MatchLength; i++)
		{
			consolePrint("MatchList[%d]: 0x%02X\n", i, msg->MatchList[i]);
		}
	}
	else
	{
		consolePrint("MatchDescRsp Status: FAIL 0x%02X\n", msg->Status);
	}

	return msg->Status;
}
static uint8_t mtZdoComplexDescRspCb(ComplexDescRspFormat_t *msg)
{
	consolePrint("mtZdoComplexDescRspCb\n");

	consolePrint("SrcAddr: 0x%04X\n", msg->SrcAddr);
	consolePrint("Status: 0x%02X\n", msg->Status);
	consolePrint("NwkAddr: 0x%04X\n", msg->NwkAddr);
	consolePrint("ComplexLength: 0x%02X\n", msg->ComplexLength);
	uint32_t i;
	for (i = 0; i < msg->ComplexLength; i++)
	{
		consolePrint("ComplexList[%d]: 0x%02X\n", i, msg->ComplexList[i]);
	}

	return msg->Status;
}
static uint8_t mtZdoUserDescRspCb(UserDescRspFormat_t *msg)
{
	consolePrint("mtZdoUserDescRspCb\n");
	if (msg->Status == MT_RPC_SUCCESS)
	{
		consolePrint("SrcAddr: 0x%04X\n", msg->SrcAddr);
		consolePrint("Status: 0x%02X\n", msg->Status);
		consolePrint("NwkAddr: 0x%04X\n", msg->NwkAddr);
		consolePrint("Len: 0x%02X\n", msg->Len);
		uint32_t i;
		for (i = 0; i < msg->Len; i++)
		{
			consolePrint("CUserDescriptor[%d]: 0x%02X\n", i,
					msg->CUserDescriptor[i]);
		}
	}
	else
	{
		consolePrint("UserDescRsp Status: FAIL 0x%02X\n", msg->Status);
	}

	return msg->Status;
}
static uint8_t mtZdoUserDescConfCb(UserDescConfFormat_t *msg)
{
	consolePrint("mtZdoUserDescConfCb\n");
	if (msg->Status == MT_RPC_SUCCESS)
	{
		consolePrint("SrcAddr: 0x%04X\n", msg->SrcAddr);
		consolePrint("Status: 0x%02X\n", msg->Status);
		consolePrint("NwkAddr: 0x%04X\n", msg->NwkAddr);
	}
	else
	{
		consolePrint("UserDescConf Status: FAIL 0x%02X\n", msg->Status);
	}

	return msg->Status;
}
static uint8_t mtZdoServerDiscRspCb(ServerDiscRspFormat_t *msg)
{
	consolePrint("mtZdoServerDiscRspCb\n");
	if (msg->Status == MT_RPC_SUCCESS)
	{
		consolePrint("SrcAddr: 0x%04X\n", msg->SrcAddr);
		consolePrint("Status: 0x%02X\n", msg->Status);
		consolePrint("ServerMask: 0x%04X\n", msg->ServerMask);
	}
	else
	{
		consolePrint("ServerDiscRsp Status: FAIL 0x%02X\n", msg->Status);
	}

	return msg->Status;
}
static uint8_t mtZdoEndDeviceBindRspCb(EndDeviceBindRspFormat_t *msg)
{
	consolePrint("mtZdoEndDeviceBindRspCb\n");
	if (msg->Status == MT_RPC_SUCCESS)
	{
		consolePrint("SrcAddr: 0x%04X\n", msg->SrcAddr);
		consolePrint("Status: 0x%02X\n", msg->Status);
	}
	else
	{
		consolePrint("EndDeviceBindRsp Status: FAIL 0x%02X\n", msg->Status);
	}

	return msg->Status;
}
static uint8_t mtZdoBindRspCb(BindRspFormat_t *msg)
{
	consolePrint("mtZdoBindRspCb\n");
	if (msg->Status == MT_RPC_SUCCESS)
	{
		consolePrint("SrcAddr: 0x%04X\n", msg->SrcAddr);
		consolePrint("Status: 0x%02X\n", msg->Status);
	}
	else
	{
		consolePrint("BindRsp Status: FAIL 0x%02X\n", msg->Status);
	}

	return msg->Status;
}
static uint8_t mtZdoUnbindRspCb(UnbindRspFormat_t *msg)
{
	consolePrint("mtZdoUnbindRspCb\n");
	if (msg->Status == MT_RPC_SUCCESS)
	{
		consolePrint("SrcAddr: 0x%04X\n", msg->SrcAddr);
		consolePrint("Status: 0x%02X\n", msg->Status);
	}
	else
	{
		consolePrint("UnbindRsp Status: FAIL 0x%02X\n", msg->Status);
	}

	return msg->Status;
}
static uint8_t mtZdoMgmtNwkDiscRspCb(MgmtNwkDiscRspFormat_t *msg)
{
	consolePrint("mtZdoMgmtNwkDiscRspCb\n");
	if (msg->Status == MT_RPC_SUCCESS)
	{
		consolePrint("SrcAddr: 0x%04X\n", msg->SrcAddr);
		consolePrint("Status: 0x%02X\n", msg->Status);
		consolePrint("NetworkCount: 0x%02X\n", msg->NetworkCount);
		consolePrint("StartIndex: 0x%02X\n", msg->StartIndex);
		consolePrint("NetworkListCount: 0x%02X\n", msg->NetworkListCount);

		uint32_t i;
		for (i = 0; i < msg->NetworkListCount; i++)
		{
			consolePrint("mtZdoNetworkListItems[%d]:\n", i);
			consolePrint("\tPanID: 0x%016llX\n",
					(long long unsigned int) msg->NetworkList[i].PanID);
			consolePrint("\tLogicalChannel: 0x%02X\n",
					msg->NetworkList[i].LogicalChannel);
			consolePrint("\tStackProf_ZigVer: 0x%02X\n",
					msg->NetworkList[i].StackProf_ZigVer);
			consolePrint("\tBeacOrd_SupFramOrd: 0x%02X\n",
					msg->NetworkList[i].BeacOrd_SupFramOrd);
			consolePrint("\tPermitJoin: 0x%02X\n\n",
					msg->NetworkList[i].PermitJoin);
		}
	}
	else
	{
		consolePrint("MgmtNwkDiscRsp Status: FAIL 0x%02X\n", msg->Status);
	}

	return msg->Status;
}
static uint8_t mtZdoMgmtLqiRspCb(MgmtLqiRspFormat_t *msg)
{
	consolePrint("mtZdoMgmtLqiRspCb\n");
	if (msg->Status == MT_RPC_SUCCESS)
	{
		consolePrint("SrcAddr: 0x%04X\n", msg->SrcAddr);
		consolePrint("Status: 0x%02X\n", msg->Status);
		consolePrint("NeighborTableEntries: 0x%02X\n",
				msg->NeighborTableEntries);
		consolePrint("StartIndex: 0x%02X\n", msg->StartIndex);
		consolePrint("NeighborLqiListCount: 0x%02X\n",
				msg->NeighborLqiListCount);
		uint32_t i;
		for (i = 0; i < msg->NeighborLqiListCount; i++)
		{

			consolePrint("mtZdoNeighborLqiListItem[%d]:\n", i);

			consolePrint("\tExtendedPanID: 0x%016llX\n",
					(long long unsigned int) msg->NeighborLqiList[i].ExtendedPanID);
			consolePrint("\tExtendedAddress: 0x%016llX\n",
					(long long unsigned int) msg->NeighborLqiList[i].ExtendedAddress);
			consolePrint("\tNetworkAddress: 0x%04X\n",
					msg->NeighborLqiList[i].NetworkAddress);
			consolePrint("\tDevTyp_RxOnWhenIdle_Relat: 0x%02X\n",
					msg->NeighborLqiList[i].DevTyp_RxOnWhenIdle_Relat);
			consolePrint("\tPermitJoining: 0x%02X\n",
					msg->NeighborLqiList[i].PermitJoining);
			consolePrint("\tDepth: 0x%02X\n", msg->NeighborLqiList[i].Depth);
			consolePrint("\tLQI: 0x%02X\n", msg->NeighborLqiList[i].LQI);
		}
	}
	else
	{
		consolePrint("MgmtLqiRsp Status: FAIL 0x%02X\n", msg->Status);
	}

	return msg->Status;
}
static uint8_t mtZdoMgmtRtgRspCb(MgmtRtgRspFormat_t *msg)
{
	consolePrint("mtZdoMgmtRtgRspCb\n");
	if (msg->Status == MT_RPC_SUCCESS)
	{
		consolePrint("SrcAddr: 0x%04X\n", msg->SrcAddr);
		consolePrint("Status: 0x%02X\n", msg->Status);
		consolePrint("RoutingTableEntries: 0x%02X\n", msg->RoutingTableEntries);
		consolePrint("StartIndex: 0x%02X\n", msg->StartIndex);
		consolePrint("RoutingTableListCount: 0x%02X\n",
				msg->RoutingTableListCount);
		uint32_t i;
		for (i = 0; i < msg->RoutingTableListCount; i++)
		{
			consolePrint("RoutingTableListItem[%d]:\n", i);
			consolePrint("\tDstAddr: 0x%04X\n",
					msg->RoutingTableList[i].DstAddr);
			consolePrint("\tStatus: 0x%02X\n", msg->RoutingTableList[i].Status);
			consolePrint("\tNextHop: 0x%04X\n",
					msg->RoutingTableList[i].NextHop);
		}
	}
	else
	{
		consolePrint("MgmtRtgRsp Status: FAIL 0x%02X\n", msg->Status);
	}

	return msg->Status;
}
static uint8_t mtZdoMgmtBindRspCb(MgmtBindRspFormat_t *msg)
{
	consolePrint("mtZdoMgmtBindRspCb\n");
	if (msg->Status == MT_RPC_SUCCESS)
	{
		consolePrint("SrcAddr: 0x%04X\n", msg->SrcAddr);
		consolePrint("Status: 0x%02X\n", msg->Status);
		consolePrint("BindingTableEntries: 0x%02X\n", msg->BindingTableEntries);
		consolePrint("StartIndex: 0x%02X\n", msg->StartIndex);
		consolePrint("BindingTableListCount: 0x%02X\n",
				msg->BindingTableListCount);
		uint32_t i;
		for (i = 0; i < msg->BindingTableListCount; i++)
		{
			consolePrint("BindingTableList[%d]:\n", i);
			consolePrint("SrcIEEEAddr: 0x%016llX\n",
					(long long unsigned int) msg->BindingTableList[i].SrcIEEEAddr);
			consolePrint("\tSrcEndpoint: 0x%02X\n",
					msg->BindingTableList[i].SrcEndpoint);
			consolePrint("\tClusterID: 0x%02X\n",
					msg->BindingTableList[i].ClusterID);
			consolePrint("\tDstAddrMode: 0x%02X\n",
					msg->BindingTableList[i].DstAddrMode);
			consolePrint("DstIEEEAddr: 0x%016llX\n",
					(long long unsigned int) msg->BindingTableList[i].DstIEEEAddr);
			consolePrint("\tDstEndpoint: 0x%02X\n",
					msg->BindingTableList[i].DstEndpoint);
		}
	}
	else
	{
		consolePrint("MgmtBindRsp Status: FAIL 0x%02X\n", msg->Status);
	}

	return msg->Status;
}
static uint8_t mtZdoMgmtLeaveRspCb(MgmtLeaveRspFormat_t *msg)
{
	consolePrint("mtZdoMgmtLeaveRspCb\n");
	if (msg->Status == MT_RPC_SUCCESS)
	{
		consolePrint("SrcAddr: 0x%04X\n", msg->SrcAddr);
		consolePrint("Status: 0x%02X\n", msg->Status);
	}
	else
	{
		consolePrint("MgmtLeaveRsp Status: FAIL 0x%02X\n", msg->Status);
	}

	return msg->Status;
}
static uint8_t mtZdoMgmtDirectJoinRspCb(MgmtDirectJoinRspFormat_t *msg)
{
	consolePrint("mtZdoMgmtDirectJoinRspCb\n");
	if (msg->Status == MT_RPC_SUCCESS)
	{
		consolePrint("SrcAddr: 0x%04X\n", msg->SrcAddr);
		consolePrint("Status: 0x%02X\n", msg->Status);
	}
	else
	{
		consolePrint("MgmtDirectJoinRsp Status: FAIL 0x%02X\n", msg->Status);
	}

	return msg->Status;
}
static uint8_t mtZdoMgmtPermitJoinRspCb(MgmtPermitJoinRspFormat_t *msg)
{
	consolePrint("mtZdoMgmtPermitJoinRspCb\n");
	if (msg->Status == MT_RPC_SUCCESS)
	{
		consolePrint("SrcAddr: 0x%04X\n", msg->SrcAddr);
		consolePrint("Status: 0x%02X\n", msg->Status);
	}
	else
	{
		consolePrint("MgmtPermitJoinRsp Status: FAIL 0x%02X\n", msg->Status);
	}

	return msg->Status;
}

static void clear_device_state(struct device *d)
{
	struct list_head *pos, *n;
	struct endpoint *ep;
	
	list_for_each_safe(pos,n,&d->eplisthead) {
		ep = list_entry(pos, struct endpoint, list);
		if(ep && (ZCL_HA_DEVICEID_MAINS_POWER_OUTLET == ep->simpledesc.simpledesc.DeviceID || 
			ZCL_HA_DEVICEID_ON_OFF_OUTPUT== ep->simpledesc.simpledesc.DeviceID)) {
			//ep->simpledesc.device_state = 0;
			sqlitedb_update_device_state(d->ieeeaddr, ep->simpledesc.simpledesc.Endpoint, 0);
		}
	}
}

static int ias_zone_device(struct device *d)
{
	struct list_head *pos, *n;
	struct endpoint *ep;

	list_for_each_safe(pos, n, &d->eplisthead) {
		ep = list_entry(pos, struct endpoint, list);
		if(ZCL_HA_DEVICEID_IAS_ZONE == ep->simpledesc.simpledesc.DeviceID)
			return 1;
	}
	return 0;
}

static uint8_t mtZdoEndDeviceAnnceIndCb(EndDeviceAnnceIndFormat_t *msg)
{
	consolePrint("mtZdoEndDeviceAnnceIndCb\n");
	consolePrint("SrcAddr: 0x%04X\n", msg->SrcAddr);
	consolePrint("NwkAddr: 0x%04X\n", msg->NwkAddr);
	consolePrint("IEEEAddr: 0x%016llX\n",
			(long long unsigned int) msg->IEEEAddr);
	//consolePrint("Capabilities: 0x%02X\n", msg->Capabilities);

	//znp_map_insert(msg->NwkAddr, msg->IEEEAddr);
	pthread_mutex_lock(&big_mutex);
	printf("[mtZdoEndDeviceAnnceIndCb] lock\n");
	struct device * d = gateway_getdevice(getgateway(), msg->IEEEAddr);
	if(!d){
		d = device_create(msg->IEEEAddr, msg->NwkAddr);
		gateway_adddevice(getgateway(), d);
		sqlitedb_insert_device_ieee(msg->IEEEAddr, msg->NwkAddr);
	}
	
	//d->status &= ~DEVICE_APP_DEL;
	d->status &= ~DEVICE_LEAVE_NET;
	sqlitedb_update_device_status(d);
	//d->noneedcheck = 0;
	d->accesscnt = 0;

	d->timestamp = time(NULL);
	
	if(d->shortaddr != msg->NwkAddr) { 
		sqlitedb_update_device_shortaddr(msg->IEEEAddr, msg->NwkAddr);
		d->shortaddr = msg->NwkAddr;
	}
	/*
	struct zcl_zone_enroll_req_cmd enroll_cmd;
	memset(&enroll_cmd, 0, sizeof(struct zcl_zone_enroll_req_cmd));
	enroll_cmd.cmdid = ZCLZONEENROLLREQ;
	enroll_cmd.req.ieeeaddr = d->ieeeaddr;
	int n = write(g_znpwfd, &enroll_cmd, sizeof(struct zcl_zone_enroll_req_cmd));
	fprintf(stdout, "********send add new device %llX %d %d\n", enroll_cmd.req.ieeeaddr, n, sizeof(struct zcl_zone_enroll_req_cmd));
	*/

	/*added by cc on 0707*/
	if(0 == strncasecmp(d->manufacturername, "feibit", 6)) {
		struct list_head *ep_pos, *ep_n; 
		struct endpoint * ep;
		list_for_each_safe(ep_pos, ep_n, &d->eplisthead){
			ep = list_entry(ep_pos, struct endpoint, list);
			if(ep)
				ep->simpledesc.zcl_transnum = 0;
		} 
	}
		
#if 0	
	struct zcl_basic_status_cmd cmd;
	cmd.cmdid = ZCLBASICSTATUS;
	cmd.req.status= 1;	
	cmd.req.ieeeaddr = d->ieeeaddr;
	//d->timestamp = time(NULL);

	write(g_znpwfd, &cmd, sizeof(struct zcl_basic_status_cmd));
#endif	
	//d->status |= DEVICE_ACTIVE;
#if 0
	if(d && !device_check_status(d, DEVICE_SEND_ATTR)){ 
		zclReadCmd_t readcmd; 
		readcmd.numAttr = 8;
		readcmd.attrID[0] = ATTRID_BASIC_ZCL_VERSION;
		readcmd.attrID[1] = ATTRID_BASIC_APPL_VERSION;
		readcmd.attrID[2] = ATTRID_BASIC_STACK_VERSION;
		readcmd.attrID[3] = ATTRID_BASIC_HW_VERSION;
		readcmd.attrID[4] = ATTRID_BASIC_MANUFACTURER_NAME;
		readcmd.attrID[5] = ATTRID_BASIC_MODEL_ID;
		readcmd.attrID[6] = ATTRID_BASIC_DATE_CODE;         
		readcmd.attrID[7] = ATTRID_BASIC_POWER_SOURCE;

		//zcl_SendRead(message->DstEndpoint, message->SrcEndpoint, message->SrcAddr, ZCL_CLUSTER_ID_GEN_BASIC, &readcmd, ZCL_CLUSTER_ID_GEN_BASIC,0,0);
		printf("mtZdoEndDeviceAnnceIndCb:send read attribute cmd\n");
		zcl_SendRead(1, 1, msg->SrcAddr, ZCL_CLUSTER_ID_GEN_BASIC, &readcmd, ZCL_CLUSTER_ID_GEN_BASIC, 0, get_sequence());
		device_set_status(d, DEVICE_SEND_ATTR);
	}
#endif
	/*added on 0629*/
	#if 0
	if(d && !device_check_status(d, DEVICE_ACTIVE)) {
		report_new_device(d);
		device_set_status(d, DEVICE_ACTIVE);
	}
	#endif
	/*printf("check DEVICE_ACTIVE\n");
	if(d && device_check_status(d, DEVICE_ACTIVE)) {
		printf("report new device\n");
		report_new_device(d);
		//device_set_status(d, DEVICE_ACTIVE);
	}*/
	
	if(d && (((d->status & DEVICE_SEND_ACTIVEEP) == 0) || list_empty(&d->eplisthead))) {
	//if(d && ((d->status & DEVICE_SEND_ACTIVEEP) == 0)) {
		consolePrint("mtZdoEndDeviceAnnceIndCb: request active_ep_req\n");		
		//device_set_status(d, DEVICE_SEND_ACTIVEEP);
		d->status |= DEVICE_SEND_ACTIVEEP;
		d->status &= ~DEVICE_SEND_SIMPLEDESC;
		sqlitedb_update_device_status(d);
		
		ActiveEpReqFormat_t queryep;
		memset(&queryep, 0, sizeof(ActiveEpReqFormat_t));
		queryep.NwkAddrOfInterest = msg->NwkAddr;
		queryep.DstAddr = msg->SrcAddr;
		sendcmd((unsigned char *)&queryep, ZDO_ACTIVE_EP_REQ);
	}
	else if(d && device_check_status(d, DEVICE_ACTIVE)) {
		if(!ias_zone_device(d)) {
			printf("report new device\n");
			report_new_device(d);
		}
	}

	if(d && !list_empty(&d->eplisthead)) {
		clear_device_state(d);
	}
	printf("[mtZdoEndDeviceAnnceIndCb] unlock\n");
	pthread_mutex_unlock(&big_mutex);

	return 0;
}

static uint8_t mtZdoMatchDescRspSentCb(MatchDescRspSentFormat_t *msg)
{
	consolePrint("mtZdoMatchDescRspSentCb\n");
	consolePrint("NwkAddr: 0x%04X\n", msg->NwkAddr);
	consolePrint("NumInClusters: 0x%02X\n", msg->NumInClusters);
	uint32_t i;
	for (i = 0; i < msg->NumInClusters; i++)
	{
		consolePrint("InClusterList[%d]: 0x%04X\n", i, msg->InClusterList[i]);
	}
	consolePrint("NumOutClusters: 0x%02X\n", msg->NumOutClusters);
	for (i = 0; i < msg->NumOutClusters; i++)
	{
		consolePrint("OutClusterList[%d]: 0x%04X\n", i, msg->OutClusterList[i]);
	}

	return 0;
}
static uint8_t mtZdoStatusErrorRspCb(StatusErrorRspFormat_t *msg)
{
	consolePrint("mtZdoStatusErrorRspCb\n");
	if (msg->Status == MT_RPC_SUCCESS)
	{
		consolePrint("SrcAddr: 0x%04X\n", msg->SrcAddr);
		consolePrint("Status: 0x%02X\n", msg->Status);
	}
	else
	{
		consolePrint("StatusErrorRsp Status: FAIL 0x%02X\n", msg->Status);
	}

	return msg->Status;
}
static uint8_t mtZdoSrcRtgIndCb(SrcRtgIndFormat_t *msg)
{
	consolePrint("mtZdoSrcRtgIndCb\n");
	consolePrint("DstAddr: 0x%04X\n", msg->DstAddr);
	consolePrint("RelayCount: 0x%02X\n", msg->RelayCount);
	uint32_t i;
	for (i = 0; i < msg->RelayCount; i++)
	{
		consolePrint("RelayList[%d]: 0x%04X\n", i, msg->RelayList[i]);
	}

	return 0;
}
static uint8_t mtZdoBeaconNotifyIndCb(BeaconNotifyIndFormat_t *msg)
{
	consolePrint("mtZdoBeaconNotifyIndCb\n");
	consolePrint("BeaconCount: 0x%02X\n", msg->BeaconCount);
	uint32_t i;
	for (i = 0; i < msg->BeaconCount; i++)
	{
		consolePrint("BeaconListItem[%d]:\n", i);

		consolePrint("\tSrcAddr: 0x%04X\n", msg->BeaconList[i].SrcAddr);
		consolePrint("\tPanId: 0x%04X\n", msg->BeaconList[i].PanId);
		consolePrint("\tLogicalChannel: 0x%02X\n",
				msg->BeaconList[i].LogicalChannel);
		consolePrint("\tPermitJoining: 0x%02X\n",
				msg->BeaconList[i].PermitJoining);
		consolePrint("\tRouterCap: 0x%02X\n", msg->BeaconList[i].RouterCap);
		consolePrint("\tPDevCap: 0x%02X\n", msg->BeaconList[i].DevCap);
		consolePrint("\tProtocolVer: 0x%02X\n", msg->BeaconList[i].ProtocolVer);
		consolePrint("\tStackProf: 0x%02X\n", msg->BeaconList[i].StackProf);
		consolePrint("\tLQI: 0x%02X\n", msg->BeaconList[i].Lqi);
		consolePrint("\tDepth: 0x%02X\n", msg->BeaconList[i].Depth);
		consolePrint("\tUpdateId: 0x%02X\n", msg->BeaconList[i].UpdateId);
		consolePrint("ExtendedPanID: 0x%016llX\n",
				(long long unsigned int) msg->BeaconList[i].ExtendedPanId);
	}

	return 0;
}
static uint8_t mtZdoJoinCnfCb(JoinCnfFormat_t *msg)
{
	consolePrint("mtZdoJoinCnfCb\n");
	if (msg->Status == MT_RPC_SUCCESS)
	{
		consolePrint("Status: 0x%02X\n", msg->Status);
		consolePrint("DevAddr: 0x%04X\n", msg->DevAddr);
		consolePrint("ParentAddr: 0x%04X\n", msg->ParentAddr);
	}
	else
	{
		consolePrint("JoinCnf Status: FAIL 0x%02X\n", msg->Status);
	}

	return msg->Status;
}
static uint8_t mtZdoNwkDiscoveryCnfCb(NwkDiscoveryCnfFormat_t *msg)
{
	consolePrint("mtZdoNwkDiscoveryCnfCb\n");
	if (msg->Status == MT_RPC_SUCCESS)
	{
		consolePrint("Status: 0x%02X\n", msg->Status);
	}
	else
	{
		consolePrint("NwkDiscoveryCnf Status: FAIL 0x%02X\n", msg->Status);
	}

	return msg->Status;
}
static uint8_t mtZdoLeaveIndCb(LeaveIndFormat_t *msg)
{
	consolePrint("mtZdoLeaveIndCb\n");
	consolePrint("SrcAddr: 0x%04X\n", msg->SrcAddr);
	consolePrint("ExtAddr: 0x%016llX\n", (long long unsigned int) msg->ExtAddr);
	consolePrint("Request: 0x%02X\n", msg->Request);
	consolePrint("Remove: 0x%02X\n", msg->Remove);
	consolePrint("Rejoin: 0x%02X\n", msg->Rejoin);

	struct list_head *pos, *n; 
	struct endpoint * ep;
	
	pthread_mutex_lock(&big_mutex);
	printf("[mtZdoLeaveIndCb] lock\n");
	struct device * d = gateway_getdevice(getgateway(), msg->ExtAddr);
	if(d){
		//d->status &= ~DEVICE_ACTIVE;
		d->status &= ~DEVICE_SEND_ATTR;
		d->status |= DEVICE_LEAVE_NET;
		sqlitedb_update_device_status(d);
		
		list_for_each_safe(pos, n, &d->eplisthead){
			ep = list_entry(pos, struct endpoint, list);
			if(ep) {
				ep->simpledesc.zcl_transnum = 0;
				ep->simpledesc.arm.armmodel = 0;
				sqlitedb_update_device_seq(msg->ExtAddr, ep->simpledesc.simpledesc.Endpoint, 0);
			}
			else
				consolePrint("no fucking device\n");
			
		}
	}
	printf("[mtZdoLeaveIndCb] unlock\n");
	pthread_mutex_unlock(&big_mutex);

	return 0;
}
static uint8_t mtZdoMsgCbIncomingCb(MsgCbIncomingFormat_t *msg)
{

	consolePrint("mtZdoMsgCbIncomingCb\n");
	consolePrint("SrcAddr: 0x%04X\n", msg->SrcAddr);
	consolePrint("WasBroadcast: 0x%02X\n", msg->WasBroadcast);
	consolePrint("ClusterID: 0x%04X\n", msg->ClusterID);
	consolePrint("SecurityUse: 0x%02X\n", msg->SecurityUse);
	consolePrint("SeqNum: 0x%02X\n", msg->SeqNum);
	consolePrint("MacDstAddr: 0x%04X\n", msg->MacDstAddr);
	consolePrint("Status: 0x%02X\n", msg->Status);
	consolePrint("ExtAddr: 0x%016llX\n", (long long unsigned int) msg->ExtAddr);
	consolePrint("NwkAddr: 0x%04X\n", msg->NwkAddr);

	return 0;
}

/********************************************************************
 * END OF ZDO CALL BACK FUNCTIONS
 */

/********************************************************************
 * START OF AF CALL BACK FUNCTIONS
 */

static uint8_t mtAfDataConfirmCb(DataConfirmFormat_t *msg)
{
	consolePrint("mtAfDataConfirmCb\n");
	if (msg->Status == MT_RPC_SUCCESS)
	{
		consolePrint("Status: 0x%02X\n", msg->Status);
		consolePrint("Endpoint: 0x%02X\n", msg->Endpoint);
		consolePrint("TransId: 0x%02X\n", msg->TransId);
	}
	else
	{
		consolePrint("DataConfirm Status: FAIL 0x%02X\n", msg->Status);
	}

	return msg->Status;
}
static uint8_t mtAfIncomingMsgCb(IncomingMsgFormat_t *msg)
{
	consolePrint("mtAfIncomingMsgCb\n");
	//consolePrint("GroupId: 0x%04X\n", msg->GroupId);
	consolePrint("ClusterId: 0x%04X\n", msg->ClusterId);
	consolePrint("SrcAddr: 0x%04X\n", msg->SrcAddr);
	consolePrint("SrcEndpoint: 0x%02X\n", msg->SrcEndpoint);
//	consolePrint("DstEndpoint: 0x%02X\n", msg->DstEndpoint);
//	consolePrint("WasVroadcast: 0x%02X\n", msg->WasVroadcast);
//	consolePrint("LinkQuality: 0x%02X\n", msg->LinkQuality);
//	consolePrint("SecurityUse: 0x%02X\n", msg->SecurityUse);
//	consolePrint("TimeStamp: 0x%08X\n", msg->TimeStamp);
	
//	consolePrint("TransSeqNum: 0x%02X\n", msg->TransSeqNum);
//	consolePrint("Len: 0x%02X\n", msg->Len);
//#if 0
	uint32_t i;
	printf("Data: ");
	for (i = 0; i < msg->Len; i++)
	{
		printf("0x%02X ", msg->Data[i]);
	}
	consolePrint("\n");
//#endif	
	//struct device * d = gateway_getdevice_shortaddr(msg->SrcAddr);
	//if(d){ 
		//d->status |= DEVICE_ACTIVE;
	pthread_mutex_lock(&big_mutex);
	printf("zcl_proccessincomingmessage lock\n");
	zcl_proccessincomingmessage(msg);
	printf("zcl_proccessincomingmessage unlock\n");
	pthread_mutex_unlock(&big_mutex);
		//d->timestamp = time(NULL);
	//}

	return 0;
}
static uint8_t mtAfIncomingMsgExt(IncomingMsgExtFormat_t *msg)
{
	consolePrint("mtAfIncomingMsgExt\n");
	consolePrint("GroupId: 0x%04X\n", msg->GroupId);
	consolePrint("ClusterId: 0x%04X\n", msg->ClusterId);
	consolePrint("SrcAddrMode: 0x%02X\n", msg->SrcAddrMode);
	consolePrint("SrcAddr: 0x%016llX\n", (long long unsigned int) msg->SrcAddr);
	consolePrint("SrcEndpoint: 0x%02X\n", msg->SrcEndpoint);
	consolePrint("SrcPanId: 0x%04X\n", msg->SrcPanId);
	consolePrint("DstEndpoint: 0x%02X\n", msg->DstEndpoint);
	consolePrint("WasVroadcast: 0x%02X\n", msg->WasVroadcast);
	consolePrint("LinkQuality: 0x%02X\n", msg->LinkQuality);
	consolePrint("SecurityUse: 0x%02X\n", msg->SecurityUse);
	consolePrint("TimeStamp: 0x%08X\n", msg->TimeStamp);
	consolePrint("TransSeqNum: 0x%02X\n", msg->TransSeqNum);
	consolePrint("Len: 0x%02X\n", msg->Len);
	uint32_t i;
	printf("Data: ");
	for (i = 0; i < msg->Len; i++)
	{
		printf("0x%02X ",  msg->Data[i]);
	}
	consolePrint("\n");

	return 0;
}
static uint8_t mtAfDataRetrieveSrspCb(DataRetrieveSrspFormat_t *msg)
{
	consolePrint("mtAfDataRetrieveSrspCb\n");
	if (msg->Status == MT_RPC_SUCCESS)
	{
		consolePrint("Status: 0x%02X\n", msg->Status);
		consolePrint("Length: 0x%02X\n", msg->Length);
		uint32_t i;
		for (i = 0; i < msg->Length; i++)
		{
			consolePrint("Data[%d]: 0x%02X\n", i, msg->Data[i]);
		}
	}
	else
	{
		consolePrint("DataRetrieveSrsp Status: FAIL 0x%02X\n", msg->Status);
	}

	return msg->Status;
}
static uint8_t mtAfReflectErrorCb(ReflectErrorFormat_t *msg)
{
	consolePrint("mtAfReflectErrorCb\n");
	if (msg->Status == MT_RPC_SUCCESS)
	{
		consolePrint("Status: 0x%02X\n", msg->Status);
		consolePrint("Endpoint: 0x%02X\n", msg->Endpoint);
		consolePrint("TransId: 0x%02X\n", msg->TransId);
		consolePrint("DstAddrMode: 0x%02X\n", msg->DstAddrMode);
		consolePrint("DstAddr: 0x%04X\n", msg->DstAddr);
	}
	else
	{
		consolePrint("ReflectError Status: FAIL 0x%02X\n", msg->Status);
	}

	return msg->Status;
}

/********************************************************************
 * END OF AF CALL BACK FUNCTIONS
 */

/********************************************************************
 * START OF SAPI CALL BACK FUNCTIONS
 */

static uint8_t mtSapiReadConfigurationSrspCb(ReadConfigurationSrspFormat_t *msg)
{
	consolePrint("mtSapiReadConfigurationSrspCb\n");
	if (msg->Status == MT_RPC_SUCCESS)
	{
		consolePrint("Status: 0x%02X\n", msg->Status);
		consolePrint("ConfigId: 0x%02X\n", msg->ConfigId);
		consolePrint("Len: 0x%02X\n", msg->Len);
		uint32_t i;
		for (i = 0; i < msg->Len; i++)
		{
			consolePrint("Value[%d]: 0x%02X\n", i, msg->Value[i]);
		}
	}
	else
	{
		consolePrint("ReadConfigurationSrsp Status: FAIL 0x%02X\n",
				msg->Status);
	}
	return msg->Status;
}
static uint8_t mtSapiGetDeviceInfoSrspCb(GetDeviceInfoSrspFormat_t *msg)
{
	consolePrint("mtSapiGetDeviceInfoSrspCb\n");

	switch (msg->Param)
	{
		case 0:
			consolePrint("Param: (0x%02X) State\n", msg->Param);
			consolePrint("Value: 0x%01X\n", msg->Value[0]);
			break;
		case 1:
			consolePrint("Param: (0x%02X) IEEE Address\n", msg->Param);
			consolePrint(
					"Value: 0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X\n",
					(unsigned char) msg->Value[0], (unsigned char) msg->Value[1],
					(unsigned char) msg->Value[2], (unsigned char) msg->Value[3],
					(unsigned char) msg->Value[4], (unsigned char) msg->Value[5],
					(unsigned char) msg->Value[6], (unsigned char) msg->Value[7]);
			break;
		case 2:
			consolePrint("Param: (0x%02X) Short Address\n", msg->Param);
			consolePrint("Value: 0x%04X\n",
					BUILD_UINT16(msg->Value[0], msg->Value[1]));
			break;
		case 3:
			consolePrint("Param: (0x%02X) Parent Short Address\n", msg->Param);
			consolePrint("Value: 0x%04X\n",
					BUILD_UINT16(msg->Value[0], msg->Value[1]));
			break;
		case 4:
			consolePrint("Param: (0x%02X) Parent IEEE Address\n", msg->Param);
			consolePrint(
					"Value: 0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X\n",
					(unsigned char) msg->Value[0], (unsigned char) msg->Value[1],
					(unsigned char) msg->Value[2], (unsigned char) msg->Value[3],
					(unsigned char) msg->Value[4], (unsigned char) msg->Value[5],
					(unsigned char) msg->Value[6], (unsigned char) msg->Value[7]);
			break;
		case 5:
			consolePrint("Param: (0x%02X) Channel\n", msg->Param);
			consolePrint("Value: 0x%01X\n", msg->Value[0]);
			break;
		case 6:
			consolePrint("Param: (0x%02X) PAN ID\n", msg->Param);
			consolePrint("Value: 0x%04X\n",
					BUILD_UINT16(msg->Value[0], msg->Value[1]));
			break;
		case 7:
			consolePrint("Param: (0x%02X) Extended PAN ID\n", msg->Param);
			consolePrint(
					"Value: 0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X\n",
					(unsigned char) msg->Value[0], (unsigned char) msg->Value[1],
					(unsigned char) msg->Value[2], (unsigned char) msg->Value[3],
					(unsigned char) msg->Value[4], (unsigned char) msg->Value[5],
					(unsigned char) msg->Value[6], (unsigned char) msg->Value[7]);
			break;

	}


	return 0;
}
static uint8_t mtSapiFindDeviceCnfCb(FindDeviceCnfFormat_t *msg)
{
	consolePrint("mtSapiFindDeviceCnfCb\n");
	consolePrint("SearchKey: 0x%04X\n", msg->SearchKey);
	consolePrint("Result: 0x%016llX\n", (long long unsigned int) msg->Result);
	return 0;
}
static uint8_t mtSapiSendDataCnfCb(SendDataCnfFormat_t *msg)
{
	consolePrint("mtSapiSendDataCnfCb\n");
	if (msg->Status == MT_RPC_SUCCESS)
	{
		consolePrint("Handle: 0x%02X\n", msg->Handle);
		consolePrint("Status: 0x%02X\n", msg->Status);
	}
	else
	{
		consolePrint("SendDataCnf Status: FAIL 0x%02X\n", msg->Status);
	}
	return msg->Status;
}
static uint8_t mtSapiReceiveDataIndCb(ReceiveDataIndFormat_t *msg)
{
	consolePrint("mtSapiReceiveDataIndCb\n");
	consolePrint("Source: 0x%04X\n", msg->Source);
	consolePrint("Command: 0x%04X\n", msg->Command);
	consolePrint("Len: 0x%04X\n", msg->Len);
	uint32_t i;
	for (i = 0; i < msg->Len; i++)
	{
		consolePrint("Data[%d]: 0x%02X\n", i, msg->Data[i]);
	}
	return 0;
}
static uint8_t mtSapiAllowBindCnfCb(AllowBindCnfFormat_t *msg)
{
	consolePrint("mtSapiAllowBindCnfCb\n");
	consolePrint("Source: 0x%04X\n", msg->Source);
	return 0;
}
static uint8_t mtSapiBindCnfCb(BindCnfFormat_t *msg)
{
	consolePrint("mtSapiBindCnfCb\n");
	if (msg->Status == MT_RPC_SUCCESS)
	{
		consolePrint("CommandId: 0x%04X\n", msg->CommandId);
		consolePrint("Status: 0x%02X\n", msg->Status);
	}
	else
	{
		consolePrint("BindCnf Status: FAIL 0x%02X\n", msg->Status);
	}
	return msg->Status;
}
static uint8_t mtSapiStartCnfCb(StartCnfFormat_t *msg)
{
	consolePrint("mtSapiStartCnfCb\n");
	if (msg->Status == MT_RPC_SUCCESS)
	{
		consolePrint("Status: 0x%02X\n", msg->Status);
	}
	else
	{
		consolePrint("StartCnf Status: FAIL 0x%02X\n", msg->Status);
	}
	return msg->Status;
}

/********************************************************************
 * END OF SAPI CALL BACK FUNCTIONS
 */

// helper functions for building and sending the NV messages
static uint8_t setNVStartup(uint8_t startupOption)
{
	uint8_t status;
	OsalNvWriteFormat_t nvWrite;

	// sending startup option
	nvWrite.Id = ZCD_NV_STARTUP_OPTION;
	nvWrite.Offset = 0;
	nvWrite.Len = 1;
	nvWrite.Value[0] = startupOption;
	status = sysOsalNvWrite(&nvWrite);

	dbg_print(PRINT_LEVEL_INFO, "\n");

	dbg_print(PRINT_LEVEL_INFO, "NV Write Startup Option cmd sent[%d]...\n",
			status);

	return status;
}

static uint8_t setNVDevType(uint8_t devType)
{
	uint8_t status;
	OsalNvWriteFormat_t nvWrite;

	nvWrite.Id = ZCD_NV_LOGICAL_TYPE;
	nvWrite.Offset = 0;
	nvWrite.Len = 1;
	nvWrite.Value[0] = devType;
	status = sysOsalNvWrite(&nvWrite);

	dbg_print(PRINT_LEVEL_INFO, "\n");
	dbg_print(PRINT_LEVEL_INFO, "NV Write Device Type cmd sent... [%d]\n",
			status);

	return status;
}

static uint8_t setNVPanID(uint32_t panId)
{
	uint8_t status;
	OsalNvWriteFormat_t nvWrite;

	dbg_print(PRINT_LEVEL_INFO, "\n");
	dbg_print(PRINT_LEVEL_INFO, "NV Write PAN ID cmd sending...\n");

	nvWrite.Id = ZCD_NV_PANID;
	nvWrite.Offset = 0;
	nvWrite.Len = 2;
	nvWrite.Value[0] = LO_UINT16(panId);
	nvWrite.Value[1] = HI_UINT16(panId);
	status = sysOsalNvWrite(&nvWrite);

	dbg_print(PRINT_LEVEL_INFO, "\n");
	dbg_print(PRINT_LEVEL_INFO, "NV Write PAN ID cmd sent...[%d]\n", status);

	return status;
}

static uint8_t setNVChanList(uint32_t chanList)
{
	OsalNvWriteFormat_t nvWrite;
	uint8_t status;

	// setting chanList
	nvWrite.Id = ZCD_NV_CHANLIST;
	nvWrite.Offset = 0;
	nvWrite.Len = 4;
	nvWrite.Value[0] = BREAK_UINT32(chanList, 0);
	nvWrite.Value[1] = BREAK_UINT32(chanList, 1);
	nvWrite.Value[2] = BREAK_UINT32(chanList, 2);
	nvWrite.Value[3] = BREAK_UINT32(chanList, 3);
	status = sysOsalNvWrite(&nvWrite);

	dbg_print(PRINT_LEVEL_INFO, "\n");
	dbg_print(PRINT_LEVEL_INFO, "NV Write Channel List cmd sent...[%d]\n",
			status);

	return status;
}

static int noneed_nv_start(void)
{
	int ret = access("/home/root/neednv", F_OK);
	printf("ret = %d\n", ret);
	return (0 == ret) ? 1 : 0;
}

static int32_t startNetwork(void)
{
	uint8_t devType;
	int32_t status;
	uint8_t newNwk = 0;
	
	printf("noneed_nv_start\n");
	do
	{
		if(noneed_nv_start()) {
			printf("setNVStartup\n");
			status = setNVStartup(ZCD_STARTOPT_CLEAR_STATE | ZCD_STARTOPT_CLEAR_CONFIG);
			system("rm /home/root/neednv");
		}
		else
			status = setNVStartup(0);
		newNwk = 1;

	} while (0);

	if (status != MT_RPC_SUCCESS)
	{
		dbg_print(PRINT_LEVEL_WARNING, "network start failed\n");
		return -1;
	}
	consolePrint("Resetting ZNP\n");
	ResetReqFormat_t resReq;
	resReq.Type = 1;
	sysResetReq(&resReq);
	//flush the rsp
	rpcWaitMqClientMsg(5000);

	if (newNwk)
	{
#ifndef CC26xx
		devType = DEVICETYPE_COORDINATOR;
		status = setNVDevType(devType);

		if (status != MT_RPC_SUCCESS)
		{
			dbg_print(PRINT_LEVEL_WARNING, "setNVDevType failed\n");
			return 0;
		}
#endif // CC26xx


		//Select random PAN ID for Coord and join any PAN for RTR/ED
		//status = setNVPanID(0x0202);
		status = setNVPanID(0xFFFF);
		if (status != MT_RPC_SUCCESS)
		{
			dbg_print(PRINT_LEVEL_WARNING, "setNVPanID failed\n");
			return -1;
		}
		status = setNVChanList(0x07FFF800);
		//status = setNVChanList(0x00000013);
		if (status != MT_RPC_SUCCESS)
		{
			dbg_print(PRINT_LEVEL_INFO, "setNVPanID failed\n");
			return -1;
		}

	}

	registerAf();
	consolePrint("EndPoint: 1\n");

	status = zdoInit();
	if (status == NEW_NETWORK)
	{
		dbg_print(PRINT_LEVEL_INFO, "zdoInit NEW_NETWORK\n");
		consolePrint("new network\n");
		status = MT_RPC_SUCCESS;
	}
	else if (status == RESTORED_NETWORK)
	{
		dbg_print(PRINT_LEVEL_INFO, "zdoInit RESTORED_NETWORK\n");
		consolePrint("restored network\n");
		status = MT_RPC_SUCCESS;
	}
	else
	{
		dbg_print(PRINT_LEVEL_INFO, "zdoInit failed\n");
		consolePrint("zdoinit fail\n");
		status = -1;
	}

	dbg_print(PRINT_LEVEL_INFO, "process zdoStatechange callbacks\n");

	//flush AREQ ZDO State Change messages
	while (status != -1)
	{
		status = rpcWaitMqClientMsg(5000);

		if (((devType == DEVICETYPE_COORDINATOR) && (devState == DEV_ZB_COORD))
				|| ((devType == DEVICETYPE_ROUTER) && (devState == DEV_ROUTER))
				|| ((devType == DEVICETYPE_ENDDEVICE)
					&& (devState == DEV_END_DEVICE)))
		{
			break;
		}
	}
	//set startup option back to keep configuration in case of reset
	status = setNVStartup(0);
	if (devState < DEV_END_DEVICE)
	{
		//start network failed
		return -1;
	}

	return 0;
}


static int32_t registerAf(void)
{
	// register the in and out clusterid. step one
	//	StartupFromAppFormat_t startupfromapp;
	//	startupfromapp.StartDelay = 0;
	//	sendcmd((unsigned char *)&startupfromapp, ZDO_STARTUP_FROM_APP); 
	zcl_register_cluster_ss();
	zcl_register_cluster_closures();

	return 1;
}


/**************** test by cc start***************
static unsigned short g_clusters[] = {
	ZCL_CLUSTER_ID_GEN_BASIC,                          
	ZCL_CLUSTER_ID_GEN_POWER_CFG,                      
	ZCL_CLUSTER_ID_GEN_DEVICE_TEMP_CONFIG,             
	ZCL_CLUSTER_ID_GEN_IDENTIFY, 
	ZCL_CLUSTER_ID_GEN_COMMISSIONING,
	ZCL_CLUSTER_ID_GEN_GROUPS,                         
	ZCL_CLUSTER_ID_GEN_SCENES,                         
	ZCL_CLUSTER_ID_GEN_ON_OFF,                         
	ZCL_CLUSTER_ID_GEN_ON_OFF_SWITCH_CONFIG,           
	ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL,                  
	ZCL_CLUSTER_ID_GEN_ALARMS,                         
	ZCL_CLUSTER_ID_GEN_TIME,
	
	ZCL_CLUSTER_ID_SS_IAS_ZONE,                        
	ZCL_CLUSTER_ID_SS_IAS_ACE,                         
	ZCL_CLUSTER_ID_SS_IAS_WD, 
	
	ZCL_CLUSTER_ID_GEN_LOCATION,                       
	ZCL_CLUSTER_ID_GEN_ANALOG_INPUT_BASIC,             
	ZCL_CLUSTER_ID_GEN_ANALOG_OUTPUT_BASIC,            
	ZCL_CLUSTER_ID_GEN_ANALOG_VALUE_BASIC,             
	ZCL_CLUSTER_ID_GEN_BINARY_INPUT_BASIC,             
	ZCL_CLUSTER_ID_GEN_BINARY_OUTPUT_BASIC,            
	ZCL_CLUSTER_ID_GEN_BINARY_VALUE_BASIC,             
	ZCL_CLUSTER_ID_GEN_MULTISTATE_INPUT_BASIC,         
	ZCL_CLUSTER_ID_GEN_MULTISTATE_OUTPUT_BASIC,        
	ZCL_CLUSTER_ID_GEN_MULTISTATE_VALUE_BASIC,         
	                  
	ZCL_CLUSTER_ID_GEN_PARTITION,                      

	ZCL_CLUSTER_ID_OTA,                                

	ZCL_CLUSTER_ID_GEN_POWER_PROFILE,                  
	ZCL_CLUSTER_ID_GEN_APPLIANCE_CONTROL,              

	ZCL_CLUSTER_ID_GEN_POLL_CONTROL,                   

	ZCL_CLUSTER_ID_GREEN_POWER_PROXY,                  

	ZCL_CLUSTER_ID_CLOSURES_SHADE_CONFIG,              
	ZCL_CLUSTER_ID_CLOSURES_DOOR_LOCK,                 
	ZCL_CLUSTER_ID_CLOSURES_WINDOW_COVERING,           

	ZCL_CLUSTER_ID_HVAC_PUMP_CONFIG_CONTROL,           
	ZCL_CLUSTER_ID_HVAC_THERMOSTAT,                    
	ZCL_CLUSTER_ID_HVAC_FAN_CONTROL,                   
	ZCL_CLUSTER_ID_HVAC_DIHUMIDIFICATION_CONTROL,      
	ZCL_CLUSTER_ID_HVAC_USER_INTERFACE_CONFIG,         

	ZCL_CLUSTER_ID_LIGHTING_COLOR_CONTROL,             
	ZCL_CLUSTER_ID_LIGHTING_BALLAST_CONFIG,            

	ZCL_CLUSTER_ID_MS_ILLUMINANCE_MEASUREMENT,         
	ZCL_CLUSTER_ID_MS_ILLUMINANCE_LEVEL_SENSING_CONFIG,
	ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT,         
	ZCL_CLUSTER_ID_MS_PRESSURE_MEASUREMENT,        
	ZCL_CLUSTER_ID_MS_FLOW_MEASUREMENT,                
	ZCL_CLUSTER_ID_MS_RELATIVE_HUMIDITY,               
	ZCL_CLUSTER_ID_MS_OCCUPANCY_SENSING,               
	

	ZCL_CLUSTER_ID_PI_BINARY_VALUE_BACNET_EXT,         
	ZCL_CLUSTER_ID_PI_MULTISTATE_INPUT_BACNET_REG,     
	ZCL_CLUSTER_ID_PI_MULTISTATE_INPUT_BACNET_EXT,     
	ZCL_CLUSTER_ID_PI_MULTISTATE_OUTPUT_BACNET_REG,    
	ZCL_CLUSTER_ID_PI_MULTISTATE_OUTPUT_BACNET_EXT,    
	ZCL_CLUSTER_ID_PI_MULTISTATE_VALUE_BACNET_REG,     
	ZCL_CLUSTER_ID_PI_MULTISTATE_VALUE_BACNET_EXT,     
	ZCL_CLUSTER_ID_PI_11073_PROTOCOL_TUNNEL,
	
	ZCL_CLUSTER_ID_SE_PRICE,                           
	ZCL_CLUSTER_ID_SE_DRLC,                            
	ZCL_CLUSTER_ID_SE_METERING,                        
	ZCL_CLUSTER_ID_SE_MESSAGING,                       
	ZCL_CLUSTER_ID_SE_TUNNELING,                       
	ZCL_CLUSTER_ID_SE_PREPAYMENT,                      
	ZCL_CLUSTER_ID_SE_ENERGY_MGMT,                     
	ZCL_CLUSTER_ID_SE_CALENDAR,                        
	ZCL_CLUSTER_ID_SE_DEVICE_MGMT,                     
	ZCL_CLUSTER_ID_SE_EVENTS,                          
	ZCL_CLUSTER_ID_SE_MDU_PAIRING,                     
	ZCL_CLUSTER_ID_SE_KEY_ESTABLISHMENT,               
	
	ZCL_CLUSTER_ID_HA_APPLIANCE_IDENTIFICATION,        
	ZCL_CLUSTER_ID_HA_METER_IDENTIFICATION,            
	ZCL_CLUSTER_ID_HA_APPLIANCE_EVENTS_ALERTS,         
	ZCL_CLUSTER_ID_HA_APPLIANCE_STATISTICS,            
	ZCL_CLUSTER_ID_HA_ELECTRICAL_MEASUREMENT,          
	ZCL_CLUSTER_ID_HA_DIAGNOSTIC,                      
	ZCL_CLUSTER_ID_LIGHT_LINK,                         
};

#define min(a,b) a>b?b:a

static int32_t registerAf(void)
{
	// register the in and out clusterid. step one
	int size = sizeof(g_clusters);
	RegisterFormat_t req;
	int count, idx, i, diff;
	printf("arry size is %d\n", size);
	for(count = 0, idx = 1; (count < size) && (idx < 253); count+=16, idx++) {
		memset(&req, 0, sizeof(RegisterFormat_t));
		req.EndPoint = idx;
		req.AppProfId = 0x0104;
		req.AppDeviceId = 1;
		diff = min(16, size - count);
		req.AppNumInClusters = diff;

		for (i = 0; i < diff; i++){
			req.AppInClusterList[i] = g_clusters[i];
		}
		req.AppNumOutClusters = 16;
		for (i = 0; i < diff; i++){
			req.AppOutClusterList[i] = g_clusters[i];
		}
		sendcmd((unsigned char *)&req, AF_REGISTER);
	}

	return 1;
}
**************** test by cc end ***************/


/*********************************************************************
 * INTERFACE FUNCTIONS
 */

uint8_t initDone = 0;
void* appMsgProcess(void *argument)
{

	if (initDone)
	{
	//	rpcWaitMqClientMsg(10000);
		rpcWaitMqClientMsg(50);
	}

	return 0;
}


void *keypresstask(void *argument)
{
	key_event_process(NULL);

	return NULL;
}


void appProcess(void * args)
{
	int znprfd = *((int *)args);
	pthread_t keythread;
	free(args);
	int32_t status;

	//fprintf(stdout, "znp znp read %d\n", znprfd);
	//Flush all messages from the que
	do
	{
		status = rpcWaitMqClientMsg(50);
	} while (status != -1);

	//init variable
	devState = DEV_HOLD;
	gSrcEndPoint = 1;
	gDstEndPoint = 1;

	status = startNetwork();
	
	initDone = 1;
	//set_led_onoff(LED_Z, LED_ON);
	if (status != -1)
	{
		//set_led_onoff(LED_Z, LED_ON);
		consolePrint("Network up\n\n");
	}
	else
	{
		consolePrint("Network Error\n\n");
	}

	sysGetExtAddr();

	OsalNvWriteFormat_t nvWrite;
	nvWrite.Id = ZCD_NV_ZDO_DIRECT_CB;
	nvWrite.Offset = 0;
	nvWrite.Len = 1;
	nvWrite.Value[0] = 1;
	status = sysOsalNvWrite(&nvWrite);

	//initDone = 1;
	//while(devState != DEV_ZB_COORD);
	pthread_create(&keythread, NULL, keypresstask, NULL);

	int commandtype = 0;
	for(;;){ 
		read(znprfd, &commandtype, sizeof(int));
		switch(commandtype){
			case PROTOCOL_IDENTIFY:
				{
					struct protocol_cmdtype_identify_ieee identify_ieee;
					int n = read(znprfd, &identify_ieee, sizeof(struct protocol_cmdtype_identify_ieee));
					fprintf(stdout, "identify znp recv %d %d -------\n", n, sizeof(struct protocol_cmdtype_identify_ieee));
					pthread_mutex_lock(&big_mutex);
					printf("[zcl_down_cmd_identify] lock\n");
					zcl_down_cmd_identify(identify_ieee.ieee,&identify_ieee.identify);
					printf("[zcl_down_cmd_identify] unlock\n");
					pthread_mutex_unlock(&big_mutex);
				}
				break;
			case PROTOCOL_WARNING:
				{
					struct protocol_cmdtype_warning_ieee warning_ieee;
					int n = read(znprfd, &warning_ieee, sizeof(struct protocol_cmdtype_warning_ieee));
					fprintf(stdout, "warning znp recv %d %d -------\n", n, sizeof(struct protocol_cmdtype_warning_ieee));
					pthread_mutex_lock(&big_mutex);
					printf("[zcl_down_cmd_warning] lock\n");
					zcl_down_cmd_warning(warning_ieee.ieee, &warning_ieee.warning);
					printf("[zcl_down_cmd_warning] unlock\n");
					pthread_mutex_unlock(&big_mutex);
				}
				break;
			case PROTOCOL_ONOFF:
				{
					struct protocol_cmdtype_onoff_ieee onoff_ieee;
					read(znprfd, &onoff_ieee, sizeof(struct protocol_cmdtype_onoff_ieee));
					pthread_mutex_lock(&big_mutex);
					printf("[zcl_down_cmd_onoff] lock\n");
					zcl_down_cmd_onoff(&onoff_ieee);
					printf("[zcl_down_cmd_onoff] unlock\n");
					pthread_mutex_unlock(&big_mutex);
				}
				break;
			case PROTOCOL_LEVEL_CTRL:
				{
					struct protocol_cmdtype_level_ctrl_ieee level_ctrl_ieee;
					read(znprfd, &level_ctrl_ieee, sizeof(struct protocol_cmdtype_level_ctrl_ieee));
					
					pthread_mutex_lock(&big_mutex);
					printf("[zcl_down_cmd_level_ctrl] lock\n");
					zcl_down_cmd_level_ctrl(&level_ctrl_ieee);
					printf("[zcl_down_cmd_level_ctrl] unlock\n");
					pthread_mutex_unlock(&big_mutex);
				}
				break;
			case PROTOCOL_PERMIT_JOING:
				{
					struct protocol_cmdtype_permit_joining permit_joining;
					read(znprfd, &permit_joining, sizeof(struct protocol_cmdtype_permit_joining));
					
					pthread_mutex_lock(&big_mutex);
					printf("[zcl_down_cmd_permit_joining] lock\n");
					zcl_down_cmd_permit_joining(&permit_joining);
					printf("[zcl_down_cmd_permit_joining] unlock\n");
					pthread_mutex_unlock(&big_mutex);
				}
				break;
			case PROTOCOL_CONFIG_REPORT:
				{
					struct protocol_cmdtype_config_reporting cfg_reporting;
					read(znprfd, &cfg_reporting, sizeof(struct protocol_cmdtype_config_reporting));
					
					pthread_mutex_lock(&big_mutex);
					printf("[zcl_down_cmd_config_reporting] lock\n");
					zcl_down_cmd_config_reporting(&cfg_reporting);
					printf("[zcl_down_cmd_config_reporting] unlock\n");
					pthread_mutex_unlock(&big_mutex);
				}
				break;
			#if 0
			case PROTOCOL_GET_STATUS:
				{
					struct protocol_cmdtype_get_device_status get_dstatus;
					read(znprfd, &get_dstatus, sizeof(struct protocol_cmdtype_get_device_status));
					zcl_down_cmd_get_dstatus(&get_dstatus);
				}
			
			case PROTOCOL_READ_ONOFF:
				{
					struct protocol_cmdtype_get_onoff_state onoff_state;
					read(znprfd, &onoff_state, sizeof(struct protocol_cmdtype_get_onoff_state));
					zcl_down_cmd_get_onoff(&onoff_state);
				}
			#endif
		}

	}
}

int appInit(void)
{
	int32_t status = 0;
	uint32_t msgCnt = 0;

	//Flush all messages from the que
	while (status != -1)
	{
		status = rpcWaitMqClientMsg(10);
		if (status != -1)
		{
			msgCnt++;
		}
	}

	dbg_print(PRINT_LEVEL_INFO, "flushed %d message from msg queue\n", msgCnt);

	//Register Callbacks MT system callbacks
	sysRegisterCallbacks(mtSysCb);
	zdoRegisterCallbacks(mtZdoCb);
	afRegisterCallbacks(mtAfCb);
	sapiRegisterCallbacks(mtSapiCb);


	return 0;
}

