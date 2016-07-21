#include <stdio.h>
#include "zcl_datatype.h"

int main(){
	union zclzonechangenotification c;
	fprintf(stdout, "size :%d\n", sizeof(union zclzonechangenotification));
	c.uszonestatus = 49;
	fprintf(stdout, "alarm1 %d\n", c.zonestatus.alarm1);
	fprintf(stdout, "alarm2 %d\n", c.zonestatus.alarm2);
	fprintf(stdout, "tmper %d\n", c.zonestatus.tamper);
	fprintf(stdout, "batt %d\n", c.zonestatus.batt);
	fprintf(stdout, "supervisionreports %d\n", c.zonestatus.supervisionreports);
	fprintf(stdout, "restorereports %d\n", c.zonestatus.restorereports);
	fprintf(stdout, "trouble %d\n", c.zonestatus.trouble);
	fprintf(stdout, "acmain %d\n", c.zonestatus.acmain);
}
