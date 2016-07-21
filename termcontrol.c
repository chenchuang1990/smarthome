#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include "termcontrol.h"
#include "connection.h"
#include "ceconf.h"

struct termconf{
	char * portname; 
	int speed;
	int parity;
};


void
set_blocking (int fd, int should_block)
{
	struct termios tty;
	memset (&tty, 0, sizeof(struct termios) );
	if (tcgetattr (fd, &tty) != 0)
	{
		fprintf (stdout, "error from tggetattr %s \n", strerror(errno));
		return;
	}

	tty.c_cc[VMIN]  = should_block ? 1 : 0;
	tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

	if (tcsetattr (fd, TCSANOW, &tty) != 0){
		fprintf (stdout, "error setting term attributes %s\n", strerror(errno));
	}

}

int set_term_attribs(int fd, struct termconf * conf){
	struct termios tty;
	memset (&tty, 0, sizeof(struct termios) );
	if (tcgetattr (fd, &tty) != 0)
	{
		fprintf(stdout, "error from tcgetattr %s \n", strerror(errno));
		return -1;
	}

//	cfsetospeed (&tty, conf->speed);
//	cfsetispeed (&tty, conf->speed);
	cfsetospeed (&tty, B115200);
	cfsetispeed (&tty, B115200);

	tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
	// disable IGNBRK for mismatched speed tests; otherwise receive break
	// as \000 chars
	tty.c_iflag &= ~IGNBRK;         // disable break processing
	tty.c_lflag = 0;                // no signaling chars, no echo,
	// no canonical processing
	tty.c_oflag = 0;                // no remapping, no delays
	tty.c_cc[VMIN]  = 0;            // read doesn't block
	tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

	tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

	tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
	// enable reading
	tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
	tty.c_cflag |= conf->parity;
	tty.c_cflag &= ~CSTOPB;
	tty.c_cflag &= ~CRTSCTS;

	if (tcsetattr (fd, TCSANOW, &tty) != 0) {
		fprintf(stdout, "error from tcsetattr %s \n", strerror(errno));
		return -1;
	}

	return 0;
}

struct connection * openterm(struct termconf * conf){
	int fd = open(conf->portname, O_RDWR | O_NOCTTY | O_SYNC);
	if (fd < 0){
		fprintf(stdout, "error %d opending %s: %s\n", errno, conf->portname, strerror(errno));
		return NULL;
	}
	set_term_attribs(fd, conf);
	set_blocking(fd, 0);
	make_socket_non_blocking(fd);

	struct connection * conn = freeconnlist_getconn();
	connection_init(conn, fd, CONNSERIALPORT);
	connrbtree_insert(conn);

	return conn;
}


struct connection * connserialport(){
	struct termconf conf;
	conf.portname = ceconf_getserialport();
	conf.speed = ceconf_getspeed();
	conf.parity = ceconf_getparity();

	return openterm(&conf);
}
