#ifndef __ZCL_DATATYPE_H_H_
#define __ZCL_DATATYPE_H_H_

#define ZCLZONEENROLLREQ    0x00000001
#define ZCLZONECHANGENOTIFICATION 0x00000002
#define ZCLBASICSTATUS 0x00000003
#define ZCLONOFFREPORT 0x00000004


#define ZCLGENERALDEFAULTRSP	0x0000000b

#define ZCLGENONOFFRSP				0x00000010
#define ZCLGENLEVELCTLRSP			0x00000011
#define ZCLREADONOFFRSP				0x00000016
#define ZCLWARNINGRSP				0x00000502

struct __attribute__((packed))zclzoneenrollreq{
	unsigned long long ieeeaddr;
	unsigned short groupid;
	unsigned short clusterid;
	unsigned short zonetype;
	unsigned char endpoint;
	unsigned char zoneid;
};

struct __attribute__((packed))zcl_zone_enroll_req_cmd{
	unsigned int cmdid;
	struct zclzoneenrollreq req;
};

//ZoneStatusAttribute
//The ZoneStatusattribute is a bit map. The meaning of each bit is summarized in
//Table 8.6.
/*------------------------------------------------------------------------------------
 * |Attribute Bit Number | Meaning                      | Values                       
 * |0                    | Alarm1                       | 1 – opened or alarmed
 * |                     |                              | 0 – closed or not alarmed
 * |1                    | Alarm2                       | 1 – opened or alarmed
 * |                     |                              | 0 – closed or not alarmed
 * |2                    | Tamper                       | 1 – Tampered 
 * |                     |                              | 0 – Not tampered
 * |3                    | Battery                      | 1 – Low battery
 * |                     |                              | 0 – Battery OK
 * |4                    | Supervision reports (Note 1) | 1 – Reports
 * |                     |                              | 0 – Does not report
 * |5                    | Restore reports (Note 2)     | 1 – Reports restore
 * |                     |                              | 0 – Does not report restore
 * |6                    | Trouble                      | 1 – Trouble/Failure
 * |                     |                              | 0 – OK
 * |7                    | AC (mains)                   | 1 – AC/Mains fault
 * |                     |                              | 0 – AC/Mains OK
 * |8-15                 | Reserved                     | -
 * ------------------------------------------------------------------------------------
 *
 * Note 1:This bit indicates whether the Zoneissues periodic Zone Status Change
 * Notification commands. The CIE device may use these periodic reports as an
 * indication that a zone is operational. Zones that do not implement the periodic
 * reporting are required to set this bit to zero (the CIE will know not to interpret the
 * lack of reports as a problem).
 * Note 2:This bit indicates whether or not a Zone Status Change Notification
 * command will be sent to indicate that an alarm is no longer present. Some Zones
 * do not have the ability to detect that alarm condition is no longer present, they
 * only can tell that an alarm has occurred.These Zones must set the “Restore” bit to
 * zero, indicating to the CIE not to look for alarm-restore notifications.
 */
struct __attribute__((packed))zclzonechangenotification{
	unsigned long long ieeeaddr;
	unsigned short clusterid;
	unsigned short zonetype;
	unsigned char endpoint;
	union _zonechangenotification{
		unsigned short uszonestatus;
		struct{ 
			unsigned char alarm1:1;
			unsigned char alarm2:1;
			unsigned char tamper:1;
			unsigned char batt:1;
			unsigned char supervisionreports:1;
			unsigned char restorereports:1;
			unsigned char trouble:1;
			unsigned char acmain:1;
			unsigned char reserved;
		}zonestatus;
	}zonechangenotification;
	unsigned char ext_status;
	unsigned char trans_num;
};

struct __attribute__((packed))zcl_zone_change_notification_cmd {
	unsigned int cmdid;
	struct zclzonechangenotification req;
};


struct __attribute__((packed))zclgeneraldefaultresponse {
	unsigned long long ieeeaddr;
	unsigned int serialnum;
	unsigned short clusterid;
	unsigned char endpoint;
	unsigned char cmd_ind;
	unsigned char status;
};

struct __attribute__((packed))zcl_general_default_response_cmd {
	unsigned int cmdid;
	struct zclgeneraldefaultresponse req;
};

struct __attribute__((packed))zclbasicstatus {
	unsigned long long ieeeaddr;
	unsigned char status;
};

struct __attribute__((packed))zcl_basic_status_cmd {
	unsigned int cmdid;
	struct zclbasicstatus req;
};

struct __attribute__((packed))zclonoffreport {
	unsigned long long ieeeaddr;
	unsigned char endpoint;
	unsigned char state;
};

struct __attribute__((packed))zcl_onoff_report_cmd {
	unsigned int cmdid;
	struct zclonoffreport req;
};

struct __attribute__((packed))zclreadonoffrsp {
	unsigned long long ieeeaddr;
	unsigned int serialnum;
	unsigned char endpoint;
	unsigned char state;
};

struct __attribute__((packed))zcl_read_onoff_rsp_cmd {
	unsigned int cmdid;
	struct zclreadonoffrsp req;
};


#endif
