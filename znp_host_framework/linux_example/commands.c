
#include "rpc.h"
#include "mtSys.h"
#include "mtZdo.h"
#include "mtAf.h"
#include "mtParser.h"
#include "mtSapi.h"
#include "rpcTransport.h"
#include "dbgPrint.h"

void sendcmd(unsigned char * req, unsigned char cmdtype){
	switch (cmdtype)
	{
	case 0:
		sysPing();
		break;
	case 1:
		sysSetExtAddr((SetExtAddrFormat_t*) req);
		break;
	case 2:
		sysGetExtAddr();
		break;
	case 3:
		sysRamRead((RamReadFormat_t*) req);
		break;
	case 4:
		sysRamWrite((RamWriteFormat_t*) req);
		break;
	case 5:
		sysResetReq((ResetReqFormat_t*) req);
		break;
	case 6:
		sysVersion();
		break;
	case 7:
		sysOsalNvRead((OsalNvReadFormat_t*) req);
		break;
	case 8:
		sysOsalNvWrite((OsalNvWriteFormat_t*) req);
		break;
	case 9:
		sysOsalNvItemInit((OsalNvItemInitFormat_t*) req);
		break;
	case 10:
		sysOsalNvDelete((OsalNvDeleteFormat_t*) req);
		break;
	case 11:
		sysOsalNvLength((OsalNvLengthFormat_t*) req);
		break;
	case 12:
		sysOsalStartTimer((OsalStartTimerFormat_t*) req);
		break;
	case 13:
		sysOsalStopTimer((OsalStopTimerFormat_t*) req);
		break;
	case 14:
		sysStackTune((StackTuneFormat_t*) req);
		break;
	case 15:
		sysAdcRead((AdcReadFormat_t*) req);
		break;
	case 16:
		sysGpio((GpioFormat_t*) req);
		break;
	case 17:
		sysRandom();
		break;
	case 18:
		sysSetTime((SetTimeFormat_t*) req);
		break;
	case 19:
		sysGetTime();
		break;
	case 20:
		sysSetTxPower((SetTxPowerFormat_t*) req);
		break;
	case 21:
		afRegister((RegisterFormat_t*) req);
		break;
	case 22:
		afDataRequest((DataRequestFormat_t*) req);
		break;
	case 23:
		afDataRequestExt((DataRequestExtFormat_t*) req);
		break;
	case 24:
		afDataRequestSrcRtg((DataRequestSrcRtgFormat_t*) req);
		break;
	case 25:
		afInterPanCtl((InterPanCtlFormat_t*) req);
		break;
	case 26:
		afDataStore((DataStoreFormat_t*) req);
		break;
	case 27:
		afDataRetrieve((DataRetrieveFormat_t*) req);
		break;
	case 28:
		afApsfConfigSet((ApsfConfigSetFormat_t*) req);
		break;
	case 29:
		zdoNwkAddrReq((NwkAddrReqFormat_t*) req);
		break;
	case 30:
		zdoIeeeAddrReq((IeeeAddrReqFormat_t*) req);
		break;
	case 31:
		zdoNodeDescReq((NodeDescReqFormat_t*) req);
		break;
	case 32:
		zdoPowerDescReq((PowerDescReqFormat_t*) req);
		break;
	case 33:
		zdoSimpleDescReq((SimpleDescReqFormat_t*) req);
		break;
	case 34:
		zdoActiveEpReq((ActiveEpReqFormat_t*) req);
		break;
	case 35:
		zdoMatchDescReq((MatchDescReqFormat_t*) req);
		break;
	case 36:
		zdoComplexDescReq((ComplexDescReqFormat_t*) req);
		break;
	case 37:
		zdoUserDescReq((UserDescReqFormat_t*) req);
		break;
	case 38:
		zdoDeviceAnnce((DeviceAnnceFormat_t*) req);
		break;
	case 39:
		zdoUserDescSet((UserDescSetFormat_t*) req);
		break;
	case 40:
		zdoServerDiscReq((ServerDiscReqFormat_t*) req);
		break;
	case 41:
		zdoEndDeviceBindReq((EndDeviceBindReqFormat_t*) req);
		break;
	case 42:
		zdoBindReq((BindReqFormat_t*) req);
		break;
	case 43:
		zdoUnbindReq((UnbindReqFormat_t*) req);
		break;
	case 44:
		zdoMgmtNwkDiscReq((MgmtNwkDiscReqFormat_t*) req);
		break;
	case 45:
		zdoMgmtLqiReq((MgmtLqiReqFormat_t*) req);
		break;
	case 46:
		zdoMgmtRtgReq((MgmtRtgReqFormat_t*) req);
		break;
	case 47:
		zdoMgmtBindReq((MgmtBindReqFormat_t*) req);
		break;
	case 48:
		zdoMgmtLeaveReq((MgmtLeaveReqFormat_t*) req);
		break;
	case 49:
		zdoMgmtDirectJoinReq((MgmtDirectJoinReqFormat_t*) req);
		break;
	case 50:
		zdoMgmtPermitJoinReq((MgmtPermitJoinReqFormat_t*) req);
		break;
	case 51:
		zdoMgmtNwkUpdateReq((MgmtNwkUpdateReqFormat_t*) req);
		break;
	case 52:
		zdoStartupFromApp((StartupFromAppFormat_t*) req);
		break;
	case 53:
		zdoAutoFindDestination((AutoFindDestinationFormat_t*) req);
		break;
	case 54:
		zdoSetLinkKey((SetLinkKeyFormat_t*) req);
		break;
	case 55:
		zdoRemoveLinkKey((RemoveLinkKeyFormat_t*) req);
		break;
	case 56:
		zdoGetLinkKey((GetLinkKeyFormat_t*) req);
		break;
	case 57:
		zdoNwkDiscoveryReq((NwkDiscoveryReqFormat_t*) req);
		break;
	case 58:
		zdoJoinReq((JoinReqFormat_t*) req);
		break;
	case 59:
		zdoMsgCbRegister((MsgCbRegisterFormat_t*) req);
		break;
	case 60:
		zdoMsgCbRemove((MsgCbRemoveFormat_t*) req);
		break;
	case 61:
		zbSystemReset();
		break;
	case 62:
		zbAppRegisterReq((AppRegisterReqFormat_t*) req);
		break;
	case 63:
		zbStartReq();
		break;
	case 64:
		zbPermitJoiningReq((PermitJoiningReqFormat_t*) req);
		break;
	case 65:
		zbBindDevice((BindDeviceFormat_t*) req);
		break;
	case 66:
		zbAllowBind((AllowBindFormat_t*) req);
		break;
	case 67:
		zbSendDataReq((SendDataReqFormat_t*) req);
		break;
	case 68:
		zbFindDeviceReq((FindDeviceReqFormat_t*) req);
		break;
	case 69:
		zbWriteConfiguration((WriteConfigurationFormat_t*) req);
		break;
	case 70:
		zbGetDeviceInfo((GetDeviceInfoFormat_t*) req);
		break;
	case 71:
		zbReadConfiguration((ReadConfigurationFormat_t*) req);
		break;

	}

}
