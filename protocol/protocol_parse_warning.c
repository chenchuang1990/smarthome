#include <stdio.h>
#include "bytebuffer.h"
#include "protocol_cmdtype.h"
//设备报警(仅针对DeviceID=0x0403的设备
//-------
//标识位 1 byte "0xCE"
//消息长度 2 bytes 标识位(含)字节长度长度
//消息ID 2 bytes 0x80 0x0D
//
//SerialID 4 bytes 序列号
//DeviceID 8 bytes 设备ID(IEEE)
//EndPoint 1 byte
//WarningDuration 2 bytes 报警时间
//WarningMode 1 byte(value 0~6 解释在下面)
//Strobe 1 byte(value 0~1解释见下面)
//SirenLevel 1 byte(value 0~3 explain below)
//StrobeLevel 1 byte(value 0~3 explain below)
//StrobeDutyCycle 1 byte(unknow tobe test)
//校验码 (从开头到校验位前一位的^)
//标识位 1 byte
//-------
//解释
///***  warningMode field values ***/ 
//SS_IAS_START_WARNING_WARNING_MODE_STOP                           0
//SS_IAS_START_WARNING_WARNING_MODE_BURGLAR                        1
//SS_IAS_START_WARNING_WARNING_MODE_FIRE                           2
//SS_IAS_START_WARNING_WARNING_MODE_EMERGENCY                      3
//SS_IAS_START_WARNING_WARNING_MODE_POLICE_PANIC                   4
//SS_IAS_START_WARNING_WARNING_MODE_FIRE_PANIC                     5
//SS_IAS_START_WARNING_WARNING_MODE_EMERGENCY_PANIC                6
//
///*** start warning: strobe field values ***/ 
//SS_IAS_START_WARNING_STROBE_NO_STROBE_WARNING                    0
//SS_IAS_START_WARNING_STROBE_USE_STPOBE_IN_PARALLEL_TO_WARNING    1
//
///*** siren level field values ***/                                        
//SS_IAS_SIREN_LEVEL_LOW_LEVEL_SOUND                               0
//SS_IAS_SIREN_LEVEL_MEDIUM_LEVEL_SOUND                            1
//SS_IAS_SIREN_LEVEL_HIGH_LEVEL_SOUND                              2
//SS_IAS_SIREN_LEVEL_VERY_HIGH_LEVEL_SOUND                         3
//
///*** strobe level field values ***/                                       
//SS_IAS_STROBE_LEVEL_LOW_LEVEL_STROBE                             0
//SS_IAS_STROBE_LEVEL_MEDIUM_LEVEL_STROBE                          1
//SS_IAS_STROBE_LEVEL_HIGH_LEVEL_STROBE                            2
//SS_IAS_STROBE_LEVEL_VERY_HIGH_LEVEL_STROBE                       3
//-------

unsigned long long protocol_parse_warning(unsigned char * buf, unsigned short len, struct protocol_cmdtype_warning * warning){ 
	const unsigned char * p = buf;
	bytebuffer_skipbytes(&p, 5);
	bytebuffer_readdword(&p, &warning->serialnum);
	unsigned long long ieee;
	bytebuffer_readquadword(&p, &ieee);
	unsigned char endpoint;
	bytebuffer_readbyte(&p, &endpoint);
	unsigned short warningduration;
	bytebuffer_readword(&p, &warningduration);
	fprintf(stdout, "***** %d\n", warningduration);
	unsigned char warnmode;
	bytebuffer_readbyte(&p, &warnmode);
	unsigned char strobe;
	bytebuffer_readbyte(&p, &strobe);
	unsigned char sirenlevel;
	bytebuffer_readbyte(&p, &sirenlevel);
	unsigned char strobelevel;
	bytebuffer_readbyte(&p, &strobelevel);
	unsigned char strobedutycycle;
	bytebuffer_readbyte(&p, &strobedutycycle);
	warning->endpoint = endpoint;
	warning->start_warning.warningDuration = warningduration;
	warning->start_warning.warningmessage.warningbits.warnMode = warnmode;
	warning->start_warning.warningmessage.warningbits.warnStrobe = strobe;
	warning->start_warning.warningmessage.warningbits.warnSirenLevel = sirenlevel;
	warning->start_warning.strobeLevel = strobelevel;
	warning->start_warning.strobeDutyCycle = strobedutycycle;

	return ieee;
}
