#ifndef __ZNP_H_H_H_
#define __ZNP_H_H_H_

int znp_start(int wfd, int znprfd, char * serialport);

struct zcl_command{
	unsigned long long ieee;
};

#endif
