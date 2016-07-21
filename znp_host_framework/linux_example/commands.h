#ifndef __GCOMMAND_H_H__
#define __GCOMMAND_H_H__

#define SYS_PING                  0 
#define SYS_SET_EXTADDR           1 
#define SYS_GET_EXTADDR           2 
#define SYS_RAM_READ              3 
#define SYS_RAM_WRITE             4 
#define SYS_RESET_REQ             5 
#define SYS_VERSION               6 
#define SYS_OSAL_NV_READ          7 
#define SYS_OSAL_NV_WRITE         8 
#define SYS_OSAL_NV_ITEM_INIT     9 
#define SYS_OSAL_NV_DELETE        10
#define SYS_OSAL_NV_LENGTH        11
#define SYS_OSAL_START_TIMER      12
#define SYS_OSAL_STOP_TIMER       13
#define SYS_STACK_TUNE            14
#define SYS_ADC_READ              15
#define SYS_GPIO                  16
#define SYS_RANDOM                17
#define SYS_SET_TIME              18
#define SYS_GET_TIME              19
#define SYS_SET_TX_POWER          20
#define AF_REGISTER               21
#define AF_DATA_REQUEST           22
#define AF_DATA_REQUEST_EXT       23
#define AF_DATA_REQUEST_SRC_RTG   24
#define AF_INTER_PAN_CTL          25
#define AF_DATA_STORE             26
#define AF_DATA_RETRIEVE          27
#define AF_APSF_CONFIG_SET        28
#define ZDO_NWK_ADDR_REQ          29
#define ZDO_IEEE_ADDR_REQ         30
#define ZDO_NODE_DESC_REQ         31
#define ZDO_POWER_DESC_REQ        32
#define ZDO_SIMPLE_DESC_REQ       33
#define ZDO_ACTIVE_EP_REQ         34
#define ZDO_MATCH_DESC_REQ        35
#define ZDO_COMPLEX_DESC_REQ      36
#define ZDO_USER_DESC_REQ         37
#define ZDO_DEVICE_ANNCE          38
#define ZDO_USER_DESC_SET         39
#define ZDO_SERVER_DISC_REQ       40
#define ZDO_END_DEVICE_BIND_REQ   41
#define ZDO_BIND_REQ              42
#define ZDO_UNBIND_REQ            43
#define ZDO_MGMT_NWK_DISC_REQ     44
#define ZDO_MGMT_LQI_REQ          45
#define ZDO_MGMT_RTG_REQ          46
#define ZDO_MGMT_BIND_REQ         47
#define ZDO_MGMT_LEAVE_REQ        48
#define ZDO_MGMT_DIRECT_JOIN_REQ  49
#define ZDO_MGMT_PERMIT_JOIN_REQ  50
#define ZDO_MGMT_NWK_UPDATE_REQ   51
#define ZDO_STARTUP_FROM_APP      52
#define ZDO_AUTO_FIND_DESTINATION 53
#define ZDO_SET_LINK_KEY          54
#define ZDO_REMOVE_LINK_KEY       55
#define ZDO_GET_LINK_KEY          56
#define ZDO_NWK_DISCOVERY_REQ     57
#define ZDO_JOIN_REQ              58
#define ZDO_MSG_CB_REGISTER       59
#define ZDO_MSG_CB_REMOVE         60
#define ZB_SYSTEM_RESET           61
#define ZB_APP_REGISTER_REQ       62
#define ZB_START_REQ              63
#define ZB_PERMIT_JOINING_REQ     64
#define ZB_BIND_DEVICE            65
#define ZB_ALLOW_BIND             66
#define ZB_SEND_DATA_REQ          67
#define ZB_FIND_DEVICE_REQ        68
#define ZB_WRITE_CONFIGURATION    69
#define ZB_GET_DEVICE_INFO        70
#define ZB_READ_CONFIGURATION     71

#define COMMANDS_SIZE  72

#ifdef __cplusplus
extern "C"
{
#endif
static void sendcmd(unsigned char * req, unsigned char cmdtype);

#ifdef __cplusplus
}
#endif
#endif
