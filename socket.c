#define _GNU_SOURCE
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>
#include "connection.h"
#include "ceconf.h"
#include "toolkit.h"

#define _USE_DNS 

extern pthread_mutex_t conn_mutex;

int
make_socket_non_blocking (int sfd) {
	int flags, s;

	flags = fcntl (sfd, F_GETFL, 0);
	if (flags == -1) {
		perror ("fcntl");
		return -1;
	}

	flags |= O_NONBLOCK;
	s = fcntl (sfd, F_SETFL, flags);
	if (s == -1) {
		perror ("fcntl");
		return -1;
	}

	return 0;
}

int
create_and_bind (char *port) {
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	int s, sfd, opt = 1;

	memset (&hints, 0, sizeof (struct addrinfo));
	hints.ai_family = AF_UNSPEC;     /* Return IPv4 and IPv6 choices */
	hints.ai_socktype = SOCK_STREAM; /* We want a TCP socket */
	hints.ai_flags = AI_PASSIVE;     /* All interfaces */

	s = getaddrinfo (NULL, port, &hints, &result);
	if (s != 0) {
		fprintf (stderr, "getaddrinfo: %s\n", gai_strerror (s));
		return -1;
	}

	for (rp = result; rp != NULL; rp = rp->ai_next) {
		sfd = socket (rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (sfd == -1)
			continue;
		setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
		s = bind (sfd, rp->ai_addr, rp->ai_addrlen);
		if (s == 0) {
			/* We managed to bind successfully! */
			break;
		}

		close (sfd);
	}

	if (rp == NULL) {
		fprintf (stderr, "Could not bind\n");
		perror("[create_and_bind]");
		return -1;
	}

	freeaddrinfo (result);

	return sfd;
}

int openclient(char *addr, char *port) 
{
	typedef struct sockaddr SA;
	int sockfd;
	struct sockaddr_in servaddr;
printf("[openclient] socket...\n");
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf(stdout,"fail to connection to server\n");
		return -1;
	}
	
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;

#ifdef _USE_DNS 
	char hostname[64];
	struct hostent hostinfo, *phost;
	int ret, h_errno;
	char tempbuf[1024];
	
	sprintf(hostname, "%s", addr);
 printf("[openclient] gethostbyname_r...\n");
	ret = gethostbyname_r(hostname, &hostinfo, tempbuf, sizeof(tempbuf), &phost, &h_errno);
 	if((0 == ret) && phost)		
		bcopy((char*)phost->h_addr, (char*)&servaddr.sin_addr, phost->h_length);	
	else {		
		perror("[openclient]gethostbyname_r");		
		return -1;	
	}
#else
	servaddr.sin_addr.s_addr = inet_addr(addr);
#endif
	servaddr.sin_port = htons(atoi(port));
	printf("[openclient] connect...\n");

	if (connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) < 0) {
		fprintf(stdout, "fail to connect\n");
		close(sockfd);

		return -1;
	}
	printf("[openclient] make_socket_non_blocking...\n");

	make_socket_non_blocking(sockfd);

	return(sockfd);
}

struct connection * connectserver(){
	int fd = openclient(ceconf_getserveraddr(), ceconf_getserverport());
	struct connection * serverconn = NULL;
	if(fd != -1){
		pthread_mutex_lock(&conn_mutex);
		printf("[connectserver] lock\n");
		serverconn = freeconnlist_getconn();
		if(serverconn) {
			connection_init(serverconn, fd, CONNSOCKETSERVER);
			connrbtree_insert(serverconn);
		}
		else
			printf("connectserver::serverconn is NULL\n");
		
		printf("[connectserver] unlock\n");
		pthread_mutex_unlock(&conn_mutex);
	}

	return serverconn;
}

struct connection * createpipe(int * wfd) 
{
	int fdsig[2];
	if(pipe2(fdsig,O_CLOEXEC) == -1){
		fprintf(stderr,"create pipe error.%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
		*wfd = -1;

		return NULL;
	}

	*wfd = fdsig[1]; 
	make_socket_non_blocking(fdsig[0]);

	struct connection * conn = freeconnlist_getconn();
	if(conn) {
		connection_init(conn, fdsig[0], CONNSOCKETCMD);
		connrbtree_insert(conn);
	}
	return conn;
}

int createpipe2(int *wfd){
	int fdsig[2];
	if(pipe2(fdsig,O_CLOEXEC) == -1){
		fprintf(stderr,"create pipe error.%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
		*wfd = -1;

		return -1;
	}

	*wfd = fdsig[1]; 

	return fdsig[0];
}

int sendnonblocking(int fd, void * buf, int buflen){ 
	int n = 0;
	for(;;){
		n = write(fd, buf, buflen);
		if (n == -1) {
			if(errno == EINTR) continue;
			else if(errno == EAGAIN) break;
			else {
				fprintf(stdout, "errno %d error msg %s\n", errno,strerror(errno));
				printf("fd is %d\n", fd);
				//assert(0);
				break;
			}
		} else if (n != buflen) {
			fprintf(stdout, "Oh, no, n != buflen\n");
			//assert(0);
			break;
		} else { break; }
	}

	return n;
}

int readnonblocking(int fd, void * buf, int buflen){
	int n = -1;
	for(;;){
		n = read(fd, buf, buflen);
		if (n == -1) {
			if(errno == EINTR) continue;
			else if(errno == EAGAIN) break;
			else {
				assert(0);
				break;
			}
		} else if ((size_t)n != buflen) {
			break;
		} else { break; }
	}

	return n;
}

int broadcast(unsigned char * buf, unsigned int buflen){
	struct list_head *pos, *n;
	list_for_each_safe(pos, n, connlist_get()){
		struct connection *c = list_entry(pos, struct connection, list);
		if(c && (connection_gettype(c) == CONNSOCKETCLIENT || connection_gettype(c) == CONNSOCKETSERVER)) {
			sendnonblocking(connection_getfd(c), buf, buflen);
		}
	}
	printf("broadcast:\n");
	for(int i = 0; i < buflen; i++)
		printf("%02x ", buf[i]);
	printf("\n");

	return 0;
}
