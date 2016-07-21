#include<stdio.h>

#include "bytebuffer.h"

int main(){ 
	unsigned char test[10] = {0x08, 0x01, 0x02,0x03, 0x04, 0x05, 0x06,0x07,0x08,0x09};
	char * testa = (char *)test;
	unsigned char a = 0;
	unsigned int aa = 1;
	bytebuffer_readbyte((const unsigned char **)&testa, &a);
	unsigned short value = 0;
	bytebuffer_readword((const unsigned char **)&testa, &value);

//	unsigned char buffer[25] = {0};
//	unsigned char e = 8;
//	unsigned short b = 258;
//	unsigned int c = 50595078;
//	unsigned long long d = 578437695752307201;
//
//	unsigned char buf[5] = {0x01,0x02,0x03,0x04,0x05};
//
//	unsigned char * tb = buffer;
//	bytebuffer_writebyte(&tb, e);
//	bytebuffer_writeword(&tb, b);
//	bytebuffer_writedword(&tb, c);
//	bytebuffer_writequadword(&tb, d);
//	bytebuffer_writebytes(&tb, buf, 5);

	system("pause");

	return 0;
}
