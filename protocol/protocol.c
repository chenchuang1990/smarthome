#include <stdio.h>
#include "connection.h"
#include "bytebuffer.h"
#include "protocol.h"

void printhex(unsigned char *addr, int len)
{
	int i;
	if(len < 0)
		return;
	for(i = 0; i < len; i++) 
		printf("%02x ", addr[i]);
	printf("\n");
}

unsigned char protocol_checksum(unsigned char * buf, unsigned int buflen){
	unsigned char temp = buf[0];
	int i = 1;
	for(; i < buflen; i++){
		temp ^= buf[i];
	}

	return temp;
}

#define BUFLEN 1024
/*
int protocol_check(struct connection * c, unsigned short * messageid){ 
	unsigned char buf[BUFLEN] = {0};
	unsigned int len = connection_readbuf_getahead(c, buf, BUFLEN);
	if(len == 0){
		messageid = ILLEGAL;

		return 0;
	}
	int i;
	printf("protocol_check:");
	for(i = 0; i < len; i++) {
		printf("%02x ", buf[i]);
	}
	printf("\n");
	unsigned short tmp;
	if(buf[0] != 0xCE){ 
		printf("no head!\n");
		connection_readbuf_pop(c);
		protocol_check(c, &tmp);
	}else if(len > 2){ 
		unsigned short messagelen=bytebuffer_getword(&buf[1]);
		if(messagelen < 8 || messagelen > 2048){
			printf("messagelen is wrong!\n");
			connection_readbuf_pop(c);
			protocol_check(c,&tmp);
		}
		else if(messagelen > len){
			printf("messagelen > len\n");
			*messageid= HALFPACK;
			return 0;
		}else{
			unsigned char checksum = protocol_checksum(buf, messagelen-2);
			printf("checksum is %02x, end is %02x, messagelen is %02x", checksum, buf[messagelen - 1], messagelen);
			if(checksum == buf[messagelen - 2] && buf[messagelen - 1] == 0xCE){ 
				printf("correct message\n");
				unsigned short cmdid = bytebuffer_getword(&buf[3]);
				*messageid = cmdid;
				return messagelen;
			}else{
				printf("checksum is wrong\n");
				connection_readbuf_pop(c);
				protocol_check(c, &tmp); 
			}
		}
	}else{
		printf("halfpack\n");
		*messageid = HALFPACK;
		return 0;
	}

	return 0;
}
*/
int protocol_check(struct connection * c, unsigned short * messageid) { 	
	unsigned char buf[BUFLEN] = {0};
	unsigned char *pbuf = buf;
	unsigned int len = connection_readbuf_getahead(c, buf, BUFLEN);
	if(len == 0){
		messageid = ILLEGAL;
		pbuf = NULL;
		return 0;
	}
	while(len > 0) {
		if(pbuf[0] != 0xCE){ 
			printf("no head!\n");
			connection_readbuf_pop(c);
			pbuf++;
			len--;
			continue;
			//protocol_check(c, &tmp);
		}else if(len > 2){ 
			unsigned short messagelen=bytebuffer_getword(&pbuf[1]);
			if(messagelen < 8 || messagelen > 2048){
				printf("messagelen is wrong!\n");
				connection_readbuf_pop(c);
				//protocol_check(c,&tmp);
				pbuf++;
				len--;
				continue;
			}
			else if(messagelen > len){
				printf("messagelen > len\n");
				*messageid= HALFPACK;
				pbuf = NULL;
				return 0;
			}else{
				unsigned char checksum = protocol_checksum(pbuf, messagelen-2);
				if(checksum == pbuf[messagelen - 2] && pbuf[messagelen - 1] == 0xCE){ 
					printf("correct message\n");
					unsigned short cmdid = bytebuffer_getword(&pbuf[3]);
					*messageid = cmdid;
					pbuf = NULL;
					return messagelen;
				}else{
					printf("checksum is wrong\n");
					connection_readbuf_pop(c);
					pbuf++;
					len--;
					continue;
					//protocol_check(c, &tmp); 
				}
			}
		}
		else {
			printf("halfpack\n");
			*messageid = HALFPACK;
			return 0;
		}
		
	}
	pbuf = NULL;
	return 0;	
} 

