#ifndef __TOOLKIT_H_H_
#define __TOOLKIT_H_H_

void toolkit_printbytes(unsigned char* buf, unsigned int len);
void toolkit_backtrace(void);
unsigned long long toolkit_getmac();
unsigned char toolkit_in_period(unsigned char starthour, unsigned char startminute, unsigned char endhour, unsigned char endminute, unsigned char targethour, unsigned char targetminute);

#endif
