#ifndef _ZCL_REGISTER_CLUSTER_H_H
#define _ZCL_REGISTER_CLUSTER_H_H

#define MAX_CLUSTER_COUNT 16
#define APP_DEVICETYPEID_SS          1
//#define APP_DEVICETYPEID_SS          0x0401

#define APP_DEVICETYPEID_SS_ENDPOINT 1

#define APP_DEVICETYPEID_CLOSURES 2
#define APP_DEVICETYPEID_CLOSURES_ENDPOINT 2

int zcl_register_cluster_ss();
int zcl_register_cluster_closures();

#endif
