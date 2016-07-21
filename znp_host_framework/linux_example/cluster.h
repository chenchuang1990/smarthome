#ifndef __CLUSTER_H_H_
#define __CLUSTER_H_H_

#define BASIC                                     0x0000    // General Basic 0x0000                                      
#define POWER_CONFIGURATION                       0x0001    // General Power Configuration 0x0001
#define DEVICE_TEMPERATURE_CONFIGURATION          0x0002    // General Device Temperature Configuration 0x0002
#define IDENTIFY                                  0x0003    // General Identify 0x0003
#define GROUPS                                    0x0004    // General Groups 0x0004
#define SCENES                                    0x0005    // General Scenes 0x0005
#define ONOFF                                     0x0006    // General On/Off 0x0006
#define ONOFF_SWITCH_CONFIGURATION                0x0007    // General On/Off Switch Configuration 0x0007
#define LEVEL_CONTROL                             0x0008    // General Level Control 0x0008
#define ALARMS                                    0x0009    // General Alarms 0x0009
#define TIME                                      0x000a    // General Time 0x000A
#define BINARY_INPUT_BASIC                        0x000f    // General Binary Input (Basic) 0x000F
#define PARTITION                                 0x0016    // General Partition 0x0016
#define POWER_PROFILE                             0x001a    // General Power Profile 0x001a
#define EN50523APPLIANCE_CONTROL                  0x001b    // General EN50523Appliance Control 0x001b
#define POLL_CONTROL                              0x0020    // General Poll Control 0x0020
#define SHADE_CONFIGURATION                       0x0100    // Closures Shade Configuration 0x0100
#define DOOR_LOCK                                 0x0101    // Closures Door Lock 0x0101
#define WINDOW_COVERING                           0x0102    // Closures Window Covering 0x0102
#define PUMP_CONFIGURATION_AND_CONTROL            0x0200    // HVAC Pump Configuration and Control 0x0200
#define THERMOSTAT                                0x0201    // HVAC Thermostat 0x0201
#define FAN_CONTROL                               0x0202    // HVAC Fan Control 0x0202
#define THERMOSTAT_USER_INTERFACE_CONFIGURATION   0x0204    // HVAC Thermostat User Interface Configuration 0x0204
#define COLOR_CONTROL                             0x0300    // Lighting Color Control 0x0300
#define ILLUMINANCE_MEASUREMENT                   0x0400    // Measurement & Sensing Illuminance Measurement 0x0400
#define ILLUMINANCE_LEVEL_SENSING                 0x0401    // Measurement & Sensing Illuminance Level Sensing 0x0401
#define TEMPERATURE_MEASUREMENT                   0x0402    // Measurement & Sensing Temperature Measurement 0x0402
#define PRESSURE_MEASUREMENT                      0x0403    // Measurement & Sensing Pressure Measurement 0x0403
#define FLOW_MEASUREMENT                          0x0404    // Measurement & Sensing Flow Measurement 0x0404
#define RELATIVE_HUMIDITY_MEASUREMENT             0x0405    // Measurement & Sensing Relative Humidity Measurement 0x0405
#define OCCUPANCY_SENSING                         0x0406    // Measurement & Sensing Occupancy Sensing 0x0406
#define IAS_ZONE                                  0x0500    // Security and Safety IAS Zone 0x0500
#define IAS_ACE                                   0x0501    // Security and Safety IAS ACE 0x0501
#define IAS_WD                                    0x0502    // Security and Safety IAS WD 0x0502
#define METERING                                  0x0702    // Smart Energy Metering 0x0702
#define EN50523_APPLIANCE_IDENTIFICATION          0x0b00    // Home Automation EN50523 Appliance Identification 0x0b00
#define METER_IDENTIFICATION                      0x0b01    // Home Automation Meter Identification 0x0b01
#define EN50523_APPLIANCE_EVENTS_AND_ALERT        0x0b02    // Home Automation EN50523 Appliance events and Alert 0x0b02
#define APPLIANCE_STATISTICS                      0x0b03    // Home Automation Appliance statistics 0x0b03
#define ELECTRICITY_MEASUREMENT                   0x0b04    // Home Automation Electricity Measurement 0x0b04
#define DIAGNOSTICS                               0x0b05    // Home Automation Diagnostics 0x0b05

#define CLUSTERCOUNT 41
static unsigned short g_clusters[CLUSTERCOUNT] = {
	BASIC,
	POWER_CONFIGURATION,
	DEVICE_TEMPERATURE_CONFIGURATION,
	IDENTIFY,
	GROUPS,
	SCENES,
	ONOFF,
	ONOFF_SWITCH_CONFIGURATION,
	LEVEL_CONTROL,
	ALARMS,
	TIME,
	BINARY_INPUT_BASIC,
	PARTITION,
	POWER_PROFILE,
	EN50523APPLIANCE_CONTROL,
	POLL_CONTROL,
	SHADE_CONFIGURATION,
	DOOR_LOCK,
	WINDOW_COVERING,
	PUMP_CONFIGURATION_AND_CONTROL,
	THERMOSTAT,
	FAN_CONTROL,
	THERMOSTAT_USER_INTERFACE_CONFIGURATION,
	COLOR_CONTROL,
	ILLUMINANCE_MEASUREMENT,
	ILLUMINANCE_LEVEL_SENSING,
	TEMPERATURE_MEASUREMENT,
	PRESSURE_MEASUREMENT,
	FLOW_MEASUREMENT,
	RELATIVE_HUMIDITY_MEASUREMENT,
	OCCUPANCY_SENSING,
	IAS_ZONE,
	IAS_ACE,
	IAS_WD,
	METERING,
	EN50523_APPLIANCE_IDENTIFICATION,
	METER_IDENTIFICATION,
	EN50523_APPLIANCE_EVENTS_AND_ALERT,
	APPLIANCE_STATISTICS,
	ELECTRICITY_MEASUREMENT,
	DIAGNOSTICS,
};

#endif
