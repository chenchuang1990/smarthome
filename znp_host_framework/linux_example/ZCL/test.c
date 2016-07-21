#include <stdio.h>
#include "zcl.h"

int main(){
	struct zclframecontrol c;
	unsigned char a = 21;
	memcpy(&c, &a, 1);
	fprintf(stdout, "%d\n", c.type);
	fprintf(stdout, "%d\n", c.manuSpecific);
	fprintf(stdout, "%d\n", c.direction);
	fprintf(stdout, "%d\n", c.disableDefaultRsp);
	fprintf(stdout, "%d\n", sizeof(c));
}
