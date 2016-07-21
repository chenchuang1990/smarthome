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

#include "rpc.h"
#include "mtSys.h"
#include "mtZdo.h"
#include "mtAf.h"
#include "mtParser.h"
#include "mtSapi.h"
#include "rpcTransport.h"
#include "dbgPrint.h"

#define consolePrint printf
#define consoleClearLn(); printf("%c[2K", 27);
#define consoleFlush(); fflush(stdout);
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
static uint8_t mtZdoEndDeviceAnnceIndCb(EndDeviceAnnceIndFormat_t *msg)
{
	consolePrint("mtZdoEndDeviceAnnceIndCb\n");
	consolePrint("SrcAddr: 0x%04X\n", msg->SrcAddr);
	consolePrint("NwkAddr: 0x%04X\n", msg->NwkAddr);
	consolePrint("IEEEAddr: 0x%016llX\n",
	        (long long unsigned int) msg->IEEEAddr);
	consolePrint("Capabilities: 0x%02X\n", msg->Capabilities);
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
	consolePrint("GroupId: 0x%04X\n", msg->GroupId);
	consolePrint("ClusterId: 0x%04X\n", msg->ClusterId);
	consolePrint("SrcAddr: 0x%04X\n", msg->SrcAddr);
	consolePrint("SrcEndpoint: 0x%02X\n", msg->SrcEndpoint);
	consolePrint("DstEndpoint: 0x%02X\n", msg->DstEndpoint);
	consolePrint("WasVroadcast: 0x%02X\n", msg->WasVroadcast);
	consolePrint("LinkQuality: 0x%02X\n", msg->LinkQuality);
	consolePrint("SecurityUse: 0x%02X\n", msg->SecurityUse);
	consolePrint("TimeStamp: 0x%08X\n", msg->TimeStamp);
	consolePrint("TransSeqNum: 0x%02X\n", msg->TransSeqNum);
	consolePrint("Len: 0x%02X\n", msg->Len);
	uint32_t i;
	for (i = 0; i < msg->Len; i++)
	{
		consolePrint("Data[%d]: 0x%02X\n", i, msg->Data[i]);
	}

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
	for (i = 0; i < msg->Len; i++)
	{
		consolePrint("Data[%d]: 0x%02X\n", i, msg->Data[i]);
	}

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

static int32_t startNetwork(void)
{
	char cDevType;
	uint8_t devType;
	int32_t status;
	uint8_t newNwk = 0;
	char sCh[128];

	do
	{
		status = setNVStartup(
		ZCD_STARTOPT_CLEAR_STATE | ZCD_STARTOPT_CLEAR_CONFIG);
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
		status = setNVPanID(0xFFFF);
		if (status != MT_RPC_SUCCESS)
		{
			dbg_print(PRINT_LEVEL_WARNING, "setNVPanID failed\n");
			return -1;
		}
		status = setNVChanList(0x07FFF800);
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
		status = MT_RPC_SUCCESS;
	}
	else if (status == RESTORED_NETWORK)
	{
		dbg_print(PRINT_LEVEL_INFO, "zdoInit RESTORED_NETWORK\n");
		status = MT_RPC_SUCCESS;
	}
	else
	{
		dbg_print(PRINT_LEVEL_INFO, "zdoInit failed\n");
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
	int32_t status = 0;
	RegisterFormat_t reg;

	reg.EndPoint = 1;
	reg.AppProfId = 0x0104;
	reg.AppDeviceId = 0x0100;
	reg.AppDevVer = 1;
	reg.LatencyReq = 0;
	reg.AppNumInClusters = 1;
	reg.AppInClusterList[0] = 0x0006;
	reg.AppNumOutClusters = 0;

	status = afRegister(&reg);
	return status;
}

/*********************************************************************
 * INTERFACE FUNCTIONS
 */
uint32_t appInit(void)
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

void appProcess(void *argument)
{
	int32_t status;

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
	if (status != -1)
	{
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

	while (1)
	{
		status = clGetCmd();
		while (status != -1)
		{
			status = rpcWaitMqClientMsg(1000);
			consolePrint("\n");
		}
	}
}
