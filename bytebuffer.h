#include <stdlib.h>
#include <string.h>
#include "endianness.h"

static inline void bytebuffer_skipbyte(const unsigned char **s)
{
	(*s)++;
}

static inline void bytebuffer_skipbytes(const unsigned char **s, unsigned char count){
	(*s)+=count;
}

static inline void bytebuffer_readbyte(const unsigned char** s, unsigned char* d){
	*d = (unsigned char)**s; (*s)++;
}

static inline void bytebuffer_readword(const unsigned char** s, unsigned short* d){
	*d = 0;
	((unsigned char *)d)[0] = ((unsigned char *)*s)[0];
	((unsigned char *)d)[1] = ((unsigned char *)*s)[1];
	if(!ISBIGENDIAN){
		*d = swap16(*d);
	}
	*s += 2;
}

static inline void bytebuffer_readwordl(const unsigned char** s, unsigned short* d){
	*d = 0;
	((unsigned char *)d)[0] = ((unsigned char *)*s)[0];
	((unsigned char *)d)[1] = ((unsigned char *)*s)[1];
	if(ISBIGENDIAN){
		*d = swap16(*d);
	}
	*s += 2;
}


static inline void bytebuffer_readdword(const unsigned char** s, unsigned int* d){
	*d = 0;
	((unsigned char *)d)[0] = ((unsigned char *)*s)[0];
	((unsigned char *)d)[1] = ((unsigned char *)*s)[1];
	((unsigned char *)d)[2] = ((unsigned char *)*s)[2];
	((unsigned char *)d)[3] = ((unsigned char *)*s)[3];
	if (!ISBIGENDIAN){
		*d = swap32(*d);
	}

	*s += 4;
}

static inline void bytebuffer_readdwordl(const unsigned char** s, unsigned int* d){
	*d = 0;
	((unsigned char *)d)[0] = ((unsigned char *)*s)[0];
	((unsigned char *)d)[1] = ((unsigned char *)*s)[1];
	((unsigned char *)d)[2] = ((unsigned char *)*s)[2];
	((unsigned char *)d)[3] = ((unsigned char *)*s)[3];
	if (ISBIGENDIAN){
		*d = swap32(*d);
	}

	*s += 4;
}


static inline void bytebuffer_readmac(const unsigned char ** s, unsigned long long *d){
	*d = 0;
	((unsigned char *)d)[0] = 0;
	((unsigned char *)d)[1] = 0;
	((unsigned char *)d)[2] = ((unsigned char *)*s)[0];
	((unsigned char *)d)[3] = ((unsigned char *)*s)[1];
	((unsigned char *)d)[4] = ((unsigned char *)*s)[2];
	((unsigned char *)d)[5] = ((unsigned char *)*s)[3];
	((unsigned char *)d)[6] = ((unsigned char *)*s)[4];
	((unsigned char *)d)[7] = ((unsigned char *)*s)[5];
	if(!ISBIGENDIAN){
		*d = swap64(*d);
	}

	*s+=6;
}

static inline void bytebuffer_readquadword(const unsigned char ** s, unsigned long long *d){
	*d = 0;
	((unsigned char *)d)[0] = ((unsigned char *)*s)[0];
	((unsigned char *)d)[1] = ((unsigned char *)*s)[1];
	((unsigned char *)d)[2] = ((unsigned char *)*s)[2];
	((unsigned char *)d)[3] = ((unsigned char *)*s)[3];
	((unsigned char *)d)[4] = ((unsigned char *)*s)[4];
	((unsigned char *)d)[5] = ((unsigned char *)*s)[5];
	((unsigned char *)d)[6] = ((unsigned char *)*s)[6];
	((unsigned char *)d)[7] = ((unsigned char *)*s)[7];
	if(!ISBIGENDIAN){
		*d = swap64(*d);
	}

	*s+=8;
}

static inline void bytebuffer_readquadwordl(const unsigned char ** s, unsigned long long *d){
	*d = 0;
	((unsigned char *)d)[0] = ((unsigned char *)*s)[0];
	((unsigned char *)d)[1] = ((unsigned char *)*s)[1];
	((unsigned char *)d)[2] = ((unsigned char *)*s)[2];
	((unsigned char *)d)[3] = ((unsigned char *)*s)[3];
	((unsigned char *)d)[4] = ((unsigned char *)*s)[4];
	((unsigned char *)d)[5] = ((unsigned char *)*s)[5];
	((unsigned char *)d)[6] = ((unsigned char *)*s)[6];
	((unsigned char *)d)[7] = ((unsigned char *)*s)[7];
	if(ISBIGENDIAN){
		*d = swap64(*d);
	}

	*s+=8;
}

static inline void bytebuffer_readbytes( const unsigned char ** s, char * str, int len){
	int i;
	for(i = 0; i < len; i++) {
		str[i] = ((char *)*s)[i];
	}

	*s += len;
}

static inline void bytebuffer_writebyte( unsigned char **s, unsigned char d){
	(*((unsigned char *)*s)) = d;
	(*s)++;
}

static inline void bytebuffer_writeword( unsigned char **s, unsigned short d){
	*((unsigned short *)(*s)) = ISBIGENDIAN?d:swap16(d);
	*s += 2;
}

static inline void bytebuffer_writewordl( unsigned char **s, unsigned short d){
	*((unsigned short *)(*s)) = !ISBIGENDIAN?d:swap16(d);
	*s += 2;
}

static inline void bytebuffer_writedword( unsigned char **s, unsigned int d){
	*((unsigned int *)(*s)) = ISBIGENDIAN?d:swap32(d);
	*s += 4;
}

static inline void bytebuffer_writedwordl( unsigned char **s, unsigned int d){
	*((unsigned int *)(*s)) = !ISBIGENDIAN?d:swap32(d);
	*s += 4;
}

static inline void bytebuffer_writemac( unsigned char **s, unsigned long long d){
	if(ISBIGENDIAN){
		*((unsigned char*)(*s)) = ((unsigned char*)&d)[0];
		(*s)++;
		*((unsigned char*)(*s)) = ((unsigned char*)&d)[1];
		(*s)++;
		*((unsigned char*)(*s)) = ((unsigned char*)&d)[2];
		(*s)++;
		*((unsigned char*)(*s)) = ((unsigned char*)&d)[3];
		(*s)++;
		*((unsigned char*)(*s)) = ((unsigned char*)&d)[4];
		(*s)++;
		*((unsigned char*)(*s)) = ((unsigned char*)&d)[5];
		(*s)++;
	}else{
		*((unsigned char*)(*s)) = ((unsigned char*)&d)[5];
		(*s)++;
		*((unsigned char*)(*s)) = ((unsigned char*)&d)[4];
		(*s)++;
		*((unsigned char*)(*s)) = ((unsigned char*)&d)[3];
		(*s)++;
		*((unsigned char*)(*s)) = ((unsigned char*)&d)[2];
		(*s)++;
		*((unsigned char*)(*s)) = ((unsigned char*)&d)[1];
		(*s)++;
		*((unsigned char*)(*s)) = ((unsigned char*)&d)[0];
		(*s)++;
	}
}

static inline void bytebuffer_writequadword( unsigned char **s, unsigned long long d){
	*((unsigned long long *)(*s)) = ISBIGENDIAN?d:swap64(d);
	*s += 8;
}

static inline void bytebuffer_writequadwordl( unsigned char **s, unsigned long long d){
	*((unsigned long long *)(*s)) = !ISBIGENDIAN?d:swap64(d);
	*s += 8;
}

static inline void bytebuffer_writebytes( unsigned char **s, unsigned char * d, unsigned int len){
	memcpy(*s, d, len);
	*s += len;
}

static inline unsigned short bytebuffer_getword(unsigned char * value){
	unsigned short result = 0;
	((unsigned char *)&result)[0] = value[0];
	((unsigned char *)&result)[1] = value[1];
	if (!ISBIGENDIAN){
		return swap16(result);
	}

	return result;
}

static inline unsigned int bytebuffer_getdword(unsigned char *value){
	unsigned int result = 0;
	((unsigned char *)&result)[0] = value[0];
	((unsigned char *)&result)[1] = value[1];
	((unsigned char *)&result)[2] = value[2];
	((unsigned char *)&result)[3] = value[3];
	if (!ISBIGENDIAN){
		return swap32(result);
	}
	return result;
}

static inline unsigned long long bytebuffer_getquadword(unsigned char * value){
	unsigned long long result = 0;

	((unsigned char *)&result)[0] = value[0];
	((unsigned char *)&result)[1] = value[1];
	((unsigned char *)&result)[2] = value[2];
	((unsigned char *)&result)[3] = value[3];
	((unsigned char *)&result)[4] = value[4];
	((unsigned char *)&result)[5] = value[5];
	((unsigned char *)&result)[6] = value[6];
	((unsigned char *)&result)[7] = value[7];
	if (!ISBIGENDIAN){
		result = swap64(result);
	}

	return result;
}
