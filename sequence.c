#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define PAGESIZE 512

//#define DSTPATH "/media/mmcblk0p1/srcseq.c"

static unsigned long long xlate_seqstr2seqnum(char *str)
{
	int i;
	char *out_ptr = NULL;
	char *subp[3];
	unsigned long long seqnum_h, seqnum_m, seqnum_l;
	unsigned long long seqnum;
	
	for(i = 0; i < 3; str = NULL, i++) {
		subp[i] = strtok_r(str, ".", &out_ptr);
		if(NULL == subp[i])
			break;
		printf("--> %s\n", subp[i]);
	}
	if(0 == i) {
		printf("no match\n");
		return 0;
	}
	
	seqnum_h = strtoul(subp[0] + 4, NULL, 10);
	seqnum_m = strtoul(subp[1], NULL, 10);
	seqnum_l = strtoul(subp[2], NULL, 10);
	
	seqnum = (seqnum_h << (5 * 8)) | (seqnum_m << (4 * 8)) | seqnum_l;
	printf("seqnum:%llx\n", seqnum);
	
	return seqnum;
}

unsigned long long load_sequence(char *path)
{
	int fd, size;
	unsigned long long ret;
	char buf[PAGESIZE] = {0};
	
	fd = open(path, O_RDONLY);
	if(-1 == fd) {
		perror("load_sequence::open");
		return 0;
	}
	
	size = read(fd, buf, PAGESIZE);
	if(-1 == size) {
		perror("read");
		return 0;
	}
	else if(size == PAGESIZE) {
		buf[PAGESIZE - 1] = 0;
		printf("warning:read size maybe overflow\n");
	}
	
	ret = xlate_seqstr2seqnum(buf);
	
	return ret;
}
