#ifndef __ZCL_SS_H_H_
#define __ZCL_SS_H_H_

#include "zcl.h"
/************************************/
/***  IAS Zone Cluster Attributes ***/
/************************************/
// Zone Information attributes set
#define ATTRID_SS_IAS_ZONE_STATE                                         0x0000 // M, R, ENUM8
#define ATTRID_SS_IAS_ZONE_TYPE                                          0x0001 // M, R, ENUM16
#define ATTRID_SS_IAS_ZONE_STATUS                                        0x0002 // M, R, BITMAP16

/*** Zone State Attribute values ***/
#define SS_IAS_ZONE_STATE_NOT_ENROLLED                                   0x00
#define SS_IAS_ZONE_STATE_ENROLLED                                       0x01

/*** Zone Type Attribute values ***/
// NOTE: if more Zone Type Attribute values are added,
//       zclSS_ZoneTypeSupported() macro SHALL be updated.
#define SS_IAS_ZONE_TYPE_STANDARD_CIE                                    0x0000
#define SS_IAS_ZONE_TYPE_MOTION_SENSOR                                   0x000D
#define SS_IAS_ZONE_TYPE_CONTACT_SWITCH                                  0x0015
#define SS_IAS_ZONE_TYPE_FIRE_SENSOR                                     0x0028
#define SS_IAS_ZONE_TYPE_WATER_SENSOR                                    0x002A
#define SS_IAS_ZONE_TYPE_GAS_SENSOR                                      0x002B
#define SS_IAS_ZONE_TYPE_PERSONAL_EMERGENCY_DEVICE                       0x002C
#define SS_IAS_ZONE_TYPE_VIBRATION_MOVEMENT_SENSOR                       0x002D
#define SS_IAS_ZONE_TYPE_REMOTE_CONTROL                                  0x010F
#define SS_IAS_ZONE_TYPE_KEY_FOB                                         0x0115
#define SS_IAS_ZONE_TYPE_KEYPAD                                          0x021D
#define SS_IAS_ZONE_TYPE_STANDARD_WARNING_DEVICE                         0x0225
#define SS_IAS_ZONE_TYPE_GLASS_BREAK_SENSOR                              0x0226
#define SS_IAS_ZONE_TYPE_SECURITY_REPEATER                               0x0229
#define SS_IAS_ZONE_TYPE_INVALID_ZONE_TYPE                               0xFFFF

/*** Zone Status Attribute values ***/
#define SS_IAS_ZONE_STATUS_ALARM1_ALARMED                                0x0001
#define SS_IAS_ZONE_STATUS_ALARM2_ALARMED                                0x0002
#define SS_IAS_ZONE_STATUS_TAMPERED_YES                                  0x0004
#define SS_IAS_ZONE_STATUS_BATTERY_LOW                                   0x0008
#define SS_IAS_ZONE_STATUS_SUPERVISION_REPORTS_YES                       0x0010
#define SS_IAS_ZONE_STATUS_RESTORE_REPORTS_YES                           0x0020
#define SS_IAS_ZONE_STATUS_TROUBLE_YES                                   0x0040
#define SS_IAS_ZONE_STATUS_AC_MAINS_FAULT                                0x0080

// Zone Settings attributes set
#define ATTRID_SS_IAS_CIE_ADDRESS                                        0x0010 // M, R/W, IEEE ADDRESS
#define ATTRID_SS_ZONE_ID                                                0x0011 // M, R, UINT8
#define ATTRID_SS_NUM_ZONE_SENSITIVITY_LEVELS_SUPPORTED                  0x0012 // O, R, UINT8
#define ATTRID_SS_CURRENT_ZONE_SENSITIVITY_LEVEL                         0x0013 // O, R, UINT8

// Server commands generated (Server-to-Client in ZCL Header)
#define COMMAND_SS_IAS_ZONE_STATUS_CHANGE_NOTIFICATION                   0x00
#define COMMAND_SS_IAS_ZONE_STATUS_ENROLL_REQUEST                        0x01

// Server commands received (Client-to-Server in ZCL Header)
#define COMMAND_SS_IAS_ZONE_STATUS_ENROLL_RESPONSE                       0x00
#define COMMAND_SS_IAS_ZONE_STATUS_INIT_NORMAL_OP_MODE                   0x01
#define COMMAND_SS_IAS_ZONE_STATUS_INIT_TEST_MODE                        0x02

// permitted values for Enroll Response Code field
#define SS_IAS_ZONE_STATUS_ENROLL_RESPONSE_CODE_SUCCESS                  0x00
#define SS_IAS_ZONE_STATUS_ENROLL_RESPONSE_CODE_NOT_SUPPORTED            0x01
#define SS_IAS_ZONE_STATUS_ENROLL_RESPONSE_CODE_NO_ENROLL_PERMIT         0x02
#define SS_IAS_ZONE_STATUS_ENROLL_RESPONSE_CODE_TOO_MANY_ZONES           0x03

// Payload Lengths
#define PAYLOAD_LEN_ZONE_STATUS_CHANGE_NOTIFICATION   6
#define PAYLOAD_LEN_ZONE_ENROLL_REQUEST               4
#define PAYLOAD_LEN_ZONE_STATUS_ENROLL_RSP            2
#define PAYLOAD_LEN_ZONE_STATUS_INIT_TEST_MODE        2

/************************************/
/***  IAS ACE Cluster Attributes  ***/
/************************************/
// cluster has no attributes

// Server commands received (Client-to-Server in ZCL Header)
#define COMMAND_SS_IAS_ACE_ARM                                           0x00
#define COMMAND_SS_IAS_ACE_BYPASS                                        0x01
#define COMMAND_SS_IAS_ACE_EMERGENCY                                     0x02
#define COMMAND_SS_IAS_ACE_FIRE                                          0x03
#define COMMAND_SS_IAS_ACE_PANIC                                         0x04
#define COMMAND_SS_IAS_ACE_GET_ZONE_ID_MAP                               0x05
#define COMMAND_SS_IAS_ACE_GET_ZONE_INFORMATION                          0x06
#define COMMAND_SS_IAS_ACE_GET_PANEL_STATUS                              0x07
#define COMMAND_SS_IAS_ACE_GET_BYPASSED_ZONE_LIST                        0x08
#define COMMAND_SS_IAS_ACE_GET_ZONE_STATUS                               0x09

// Server commands generated (Server-to-Client in ZCL Header)
#define COMMAND_SS_IAS_ACE_ARM_RESPONSE                                  0x00
#define COMMAND_SS_IAS_ACE_GET_ZONE_ID_MAP_RESPONSE                      0x01
#define COMMAND_SS_IAS_ACE_GET_ZONE_INFORMATION_RESPONSE                 0x02
#define COMMAND_SS_IAS_ACE_ZONE_STATUS_CHANGED                           0x03
#define COMMAND_SS_IAS_ACE_PANEL_STATUS_CHANGED                          0x04
#define COMMAND_SS_IAS_ACE_GET_PANEL_STATUS_RESPONSE                     0x05
#define COMMAND_SS_IAS_ACE_SET_BYPASSED_ZONE_LIST                        0x06
#define COMMAND_SS_IAS_ACE_BYPASS_RESPONSE                               0x07
#define COMMAND_SS_IAS_ACE_GET_ZONE_STATUS_RESPONSE                      0x08

/*** Arm Mode field permitted values ***/
#define SS_IAS_ACE_ARM_DISARM                                            0x00
#define SS_IAS_ACE_ARM_DAY_HOME_ZONES_ONLY                               0x01
#define SS_IAS_ACE_ARM_NIGHT_SLEEP_ZONES_ONLY                            0x02
#define SS_IAS_ACE_ARM_ALL_ZONES                                         0x03

/*** Arm Notification field permitted values ***/
#define SS_IAS_ACE_ARM_NOTIFICATION_ALL_ZONES_DISARMED                   0x00
#define SS_IAS_ACE_ARM_NOTIFICATION_DAY_HOME_ZONES_ONLY                  0x01
#define SS_IAS_ACE_ARM_NOTIFICATION_NIGHT_SLEEP_ZONES_ONLY               0x02
#define SS_IAS_ACE_ARM_NOTIFICATION_ALL_ZONES_ARMED                      0x03
#define SS_IAS_ACE_ARM_NOTIFICATION_INVALID_ARM_DISARM_CODE              0x04
#define SS_IAS_ACE_ARM_NOTIFICATION_NOT_READY_TO_ARM                     0x05
#define SS_IAS_ACE_ARM_NOTIFICATION_ALREADY_DISARMED                     0x06

/*** Panel Status field permitted values ***/
#define SS_IAS_ACE_PANEL_STATUS_ALL_ZONES_DISARMED                       0x00
#define SS_IAS_ACE_PANEL_STATUS_ARMED_STAY                               0x01
#define SS_IAS_ACE_PANEL_STATUS_ARMED_NIGHT                              0x02
#define SS_IAS_ACE_PANEL_STATUS_ARMED_AWAY                               0x03
#define SS_IAS_ACE_PANEL_STATUS_EXIT_DELAY                               0x04
#define SS_IAS_ACE_PANEL_STATUS_ENTRY_DELAY                              0x05
#define SS_IAS_ACE_PANEL_STATUS_NOT_READY_TO_ARM                         0x06
#define SS_IAS_ACE_PANEL_STATUS_IN_ALARM                                 0x07
#define SS_IAS_ACE_PANEL_STATUS_ARMING_STAY                              0x08
#define SS_IAS_ACE_PANEL_STATUS_ARMING_NIGHT                             0x09
#define SS_IAS_ACE_PANEL_STATUS_ARMING_AWAY                              0x0A

/*** Audible Notification field permitted values ***/
#define SS_IAS_ACE_AUDIBLE_NOTIFICATION_MUTE                             0x00
#define SS_IAS_ACE_AUDIBLE_NOTIFICATION_DEFAULT_SOUND                    0x01

/*** Alarm Status field permitted values ***/
#define SS_IAS_ACE_ALARM_STATUS_NO_ALARM                                 0x00
#define SS_IAS_ACE_ALARM_STATUS_BURGLAR                                  0x01
#define SS_IAS_ACE_ALARM_STATUS_FIRE                                     0x02
#define SS_IAS_ACE_ALARM_STATUS_EMERGENCY                                0x03
#define SS_IAS_ACE_ALARM_STATUS_POLICE_PANIC                             0x04
#define SS_IAS_ACE_ALARM_STATUS_FIRE_PANIC                               0x05
#define SS_IAS_ACE_ALARM_STATUS_EMERGENCY_PANIC                          0x06

/*** Bypass Result field permitted values ***/
#define SS_IAS_ACE_BYPASS_RESULT_ZONE_BYPASSED                           0x00
#define SS_IAS_ACE_BYPASS_RESULT_ZONE_NOT_BYPASSED                       0x01
#define SS_IAS_ACE_BYPASS_RESULT_NOT_ALLOWED                             0x02
#define SS_IAS_ACE_BYPASS_RESULT_INVALID_ZONE_ID                         0x03
#define SS_IAS_ACE_BYPASS_RESULT_UNKNOWN_ZONE_ID                         0x04
#define SS_IAS_ACE_BYPASS_RESULT_INVALID_ARM_DISARM_CODE                 0x05

// Field Lengths
#define ZONE_ID_MAP_ARRAY_SIZE  16
#define ARM_DISARM_CODE_LEN     8
#define ZONE_LABEL_LEN          24

// Payload Lengths
#define PAYLOAD_LEN_GET_ZONE_STATUS                 5
#define PAYLOAD_LEN_PANEL_STATUS_CHANGED            4
#define PAYLOAD_LEN_GET_PANEL_STATUS_RESPONSE       4

/************************************/
/***  IAS WD Cluster Attributes   ***/
/************************************/
// Maximum Duration attribute
#define ATTRID_SS_IAS_WD_MAXIMUM_DURATION                                0x0000

// Server commands received (Client-to-Server in ZCL Header)
#define COMMAND_SS_IAS_WD_START_WARNING                                  0x00
#define COMMAND_SS_IAS_WD_SQUAWK                                         0x01

/***  warningMode field values ***/
#define SS_IAS_START_WARNING_WARNING_MODE_STOP                           0
#define SS_IAS_START_WARNING_WARNING_MODE_BURGLAR                        1
#define SS_IAS_START_WARNING_WARNING_MODE_FIRE                           2
#define SS_IAS_START_WARNING_WARNING_MODE_EMERGENCY                      3
#define SS_IAS_START_WARNING_WARNING_MODE_POLICE_PANIC                   4
#define SS_IAS_START_WARNING_WARNING_MODE_FIRE_PANIC                     5
#define SS_IAS_START_WARNING_WARNING_MODE_EMERGENCY_PANIC                6

/*** start warning: strobe field values ***/
#define SS_IAS_START_WARNING_STROBE_NO_STROBE_WARNING                    0
#define SS_IAS_START_WARNING_STROBE_USE_STPOBE_IN_PARALLEL_TO_WARNING    1

/*** siren level field values ***/
#define SS_IAS_SIREN_LEVEL_LOW_LEVEL_SOUND                               0
#define SS_IAS_SIREN_LEVEL_MEDIUM_LEVEL_SOUND                            1
#define SS_IAS_SIREN_LEVEL_HIGH_LEVEL_SOUND                              2
#define SS_IAS_SIREN_LEVEL_VERY_HIGH_LEVEL_SOUND                         3

/*** strobe level field values ***/
#define SS_IAS_STROBE_LEVEL_LOW_LEVEL_STROBE                             0
#define SS_IAS_STROBE_LEVEL_MEDIUM_LEVEL_STROBE                          1
#define SS_IAS_STROBE_LEVEL_HIGH_LEVEL_STROBE                            2
#define SS_IAS_STROBE_LEVEL_VERY_HIGH_LEVEL_STROBE                       3

/*** squawkMode field values ***/
#define SS_IAS_SQUAWK_SQUAWK_MODE_SYSTEM_ALARMED_NOTIFICATION_SOUND      0
#define SS_IAS_SQUAWK_SQUAWK_MODE_SYSTEM_DISARMED_NOTIFICATION_SOUND     1

/*** squawk strobe field values ***/
#define SS_IAS_SQUAWK_STROBE_NO_STROBE_SQUAWK                            0
#define SS_IAS_SQUAWK_STROBE_USE_STROBE_BLINK_IN_PARALLEL_TO_SQUAWK      1

/*** squawk level field values ***/
#define SS_IAS_SQUAWK_SQUAWK_LEVEL_LOW_LEVEL_SOUND                       0
#define SS_IAS_SQUAWK_SQUAWK_LEVEL_MEDIUM_LEVEL_SOUND                    1
#define SS_IAS_SQUAWK_SQUAWK_LEVEL_HIGH_LEVEL_SOUND                      2
#define SS_IAS_SQUAWK_SQUAWK_LEVEL_VERY_HIGH_LEVEL_SOUND                 3

// The maximum number of entries in the Zone table
#define ZCL_SS_MAX_ZONES                                                 256
#define ZCL_SS_MAX_ZONE_ID                                               254

/***  typedef for IAS ACE Zone table ***/
typedef struct
{
  uint8   zoneID;
  uint16  zoneType;
  uint8   zoneAddress[Z_EXTADDR_LEN];
} IAS_ACE_ZoneTable_t;

typedef struct
{
  uint16 zoneStatus;     // current zone status - bit map
  uint8  extendedStatus; // bit map, currently set to All zeroes ( reserved )
  uint8  zoneID;         // allocated zone ID
  uint16 delay;          // delay from change in ZoneStatus attr to transmission of change notification cmd
} zclZoneChangeNotif_t;

struct zclincomingmsg;
int zclss_handleincoming( struct zclincomingmsg *);

#endif
