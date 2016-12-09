#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <time.h>

#include "key.h"
#include "gateway.h"
#include "zcl.h"
#include "zcl_general.h"
#include "Zcl_down_cmd.h"
#include "zcl_datatype.h"
#include "zcl_ha.h"
#include "ceconf.h"
#include "sqlitedb.h"
#include "protocol_cmd_header.h"
#include "socket.h"


#define _USE_DNS
//#define _GNU_SOURCE

#define MAX_NO_PACKETS  1
#define ICMP_HEADSIZE 8 
#define PACKET_SIZE     4096

extern int g_znpwfd;
extern pthread_mutex_t big_mutex;

struct timeval tvsend,tvrecv;   
struct sockaddr_in dest_addr,recv_addr;
int listenfd;
pid_t pid;
char sendpacket[PACKET_SIZE];
char recvpacket[PACKET_SIZE];

void timeout(int signo);
unsigned short cal_chksum(unsigned short *addr,int len);
int pack(int pkt_no,char *sendpacket);
int send_packet(int pkt_no,char *sendpacket);
int recv_packet(int pkt_no,char *recvpacket);
int unpack(int cur_seq,char *buf,int len);
void tv_sub(struct timeval *out,struct timeval *in);
void _CloseSocket();

void restart_wifi(void)
{
	system("ifconfig wlan0 down");
	sleep(1);
	system("ifconfig wlan0 up");
	sleep(1);
	system("./wifi");
}

int NetIsOk()
{     
         
   // double rtt;
    struct hostent hostinfo, *phost;
    //struct protoent *protocol;
    int i, iFlag;
	//int recv_status;
	
 	bzero(&dest_addr, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET; 
#ifdef _USE_DNS 
    char hostname[32];
	int ret, h_errno;
	char tempbuf[1024];
	
    sprintf(hostname,"%s","www.baidu.com"); 
   	ret = gethostbyname_r(hostname, &hostinfo, tempbuf, sizeof(tempbuf), &phost, &h_errno);
 	if((0 == ret) && phost)		
		bcopy((char*)phost->h_addr, (char*)&dest_addr.sin_addr, phost->h_length);	
	else {		
		perror("[NetStatus] gethostbyname_r");		
		return -1;	
	}
#else 
    dest_addr.sin_addr.s_addr = inet_addr("220.181.111.188");
#endif
     
 
    if ((listenfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0) 
    {  
        printf("[NetStatus]  error : socket\n");
        return -1;
    }
 
    
    if((iFlag = fcntl(listenfd,F_GETFL,0)) < 0)
    {
        printf("[NetStatus]  error : fcntl(listenfd,F_GETFL,0)\n");
        _CloseSocket();
        return -1;
    }
    iFlag |= O_NONBLOCK;
    if((iFlag = fcntl(listenfd,F_SETFL,iFlag)) < 0)
    {
        printf("[NetStatus]  error : fcntl(listenfd,F_SETFL,iFlag )\n");
        _CloseSocket();
        return -1;
    }
 
    pid=getpid();
    for(i=0;i<MAX_NO_PACKETS;i++)
    {       
     
        if(send_packet(i,sendpacket)<0)
        {
            printf("[NetStatus]  error : send_packet\n");
            _CloseSocket();
            return 0;
        }   
 
        if(recv_packet(i,recvpacket)>0)
        {
            _CloseSocket();
            return 1;
        }
         
    } 
    _CloseSocket();         
    return 0;
}
 
 
 
int send_packet(int pkt_no,char *sendpacket)
{    
    int packetsize;       
    packetsize=pack(pkt_no,sendpacket); 
    gettimeofday(&tvsend,NULL);    
    if(sendto(listenfd,sendpacket,packetsize,0,(struct sockaddr *)&dest_addr,sizeof(dest_addr) )<0)
    {      
        printf("[NetStatus]  error : sendto error\n");
        return -1;
    }
    return 1;
}
 
 
int pack(int pkt_no,char*sendpacket)
{       
    int packsize;
    struct icmp *icmp;
    struct timeval *tval;
    icmp=(struct icmp*)sendpacket;
    icmp->icmp_type=ICMP_ECHO;   
    icmp->icmp_code=0;
    icmp->icmp_cksum=0;
    icmp->icmp_seq=pkt_no;
    icmp->icmp_id=pid;         
    packsize=ICMP_HEADSIZE+sizeof(struct timeval);
    tval= (struct timeval *)icmp->icmp_data;
    gettimeofday(tval,NULL);
    icmp->icmp_cksum=cal_chksum( (unsigned short *)icmp,packsize); 
    return packsize;
}
 
 
unsigned short cal_chksum(unsigned short *addr,int len)
{       
    int nleft=len;
    int sum=0;
    unsigned short *w=addr;
    unsigned short answer=0;
    while(nleft>1)     
    {       
        sum+=*w++;
        nleft-=2;
    }
    if( nleft==1)      
    {
        *(unsigned char *)(&answer)=*(unsigned char *)w;
        sum+=answer;
    }
    sum=(sum>>16)+(sum&0xffff);
    sum+=(sum>>16);
    answer=~sum;
    return answer;
}
 
 
int recv_packet(int pkt_no,char *recvpacket)
{           
    int n,fromlen, ret;
	struct timeval tv;
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(listenfd,&rfds);
    //signal(SIGALRM,timeout);
    fromlen=sizeof(recv_addr);
    //alarm(MAX_WAIT_TIME);
    
    while(1)
    {
    	tv.tv_sec = 5;
		tv.tv_usec = 0;
        ret = select(listenfd+1, &rfds, NULL, NULL, &tv);
		if(-1 == ret) 
			perror("select()");
		else if(0 == ret)
			printf("request time out\n");
		else {
	        if (FD_ISSET(listenfd,&rfds))
	        {  
	            if( (n=recvfrom(listenfd,recvpacket,PACKET_SIZE,0,(struct sockaddr *)&recv_addr,(socklen_t *)&fromlen)) <0)
	            {   
		            if(errno==EINTR)
		                return -1;
		                perror("recvfrom error");
		                return -2;
	            }
	        }
	        gettimeofday(&tvrecv,NULL); 
	        if(unpack(pkt_no,recvpacket,n)==-1)
	            continue;
	        //return 1;
		}
		return ret;
    }
}
 
int unpack(int cur_seq,char *buf,int len)
{    
    int iphdrlen;
    struct ip *ip;
    struct icmp *icmp;
    ip=(struct ip *)buf;
    iphdrlen=ip->ip_hl<<2;     
    icmp=(struct icmp *)(buf+iphdrlen);     
    len-=iphdrlen;
    if( len<8)
        return -1;       
    if( (icmp->icmp_type==ICMP_ECHOREPLY) && (icmp->icmp_id==pid) && (icmp->icmp_seq==cur_seq))
        return 0;   
    else return -1;
}
 
 
void timeout(int signo)
{
    printf("Request Timed Out\n");
}
 
void tv_sub(struct timeval *out,struct timeval *in)
{       
    if( (out->tv_usec-=in->tv_usec)<0)
    {       
        --out->tv_sec;
        out->tv_usec+=1000000;
    }
    out->tv_sec-=in->tv_sec;
}
 
void _CloseSocket()
{
    close(listenfd);
    listenfd = 0;
}

void *network_test_task(void *args)
{
	int is_ok = 0;
	//static int on_edge = 0;
	
	while(1) {
		printf("+++++++[network_test_task]++++++++\n");
		is_ok = NetIsOk();
		//fprintf(stdout, "network is %s\n", is_ok ? "ok" : "wrong");
		if(is_ok > 0) {
			//if(!on_edge) {
				//on_edge = 1;
				set_led_onoff(LED_W, LED_ON);
			//}
		}
		else {
			//if(on_edge) {
				set_led_onoff(LED_W, LED_OFF);
				//on_edge = 0;
			//}
			//restart_wifi();
		}
		
		sleep(30);
	}
	return NULL;
}

void network_test_start(void)
{
	pthread_t tid;
	pthread_create(&tid, NULL, network_test_task, NULL);
}

//#if 0

volatile int access_period = 15 * 60;
//volatile int access_period = 10;

int get_access_period(void)
{
	return access_period;
}

volatile int read_onoff_period = 10;
int get_onoff_period(void)
{
	return read_onoff_period;
}

extern uint8_t initDone;

#define TIMESTAMPOUT 30

int check_device_timeout(struct device *d)
{
	time_t cur_time = time(NULL);
	/*printf("check_device_timeout:\n");
	printf("cur_time:%lx\n", cur_time);
	printf("d->timestamp:%lx\n", d->timestamp);*/
	if(cur_time - d->timestamp > TIMESTAMPOUT)
		return 1;
	else 
		return 0;
}

void *send_read_onoff(void *args)
{
	struct gateway *gw = getgateway();
	struct list_head *pos, *n, *ep_pos, *ep_n;
	struct device *d;
	struct endpoint *ep;
	int need_delay = 0;
	//int interval, 
	//int i;
	zclReadCmd_t readcmd; 
	//zclReadReportCfgCmd_t *read_report;
	
	readcmd.numAttr = 1;
	readcmd.attrID[0] = 0;
	//readcmd.attrID[1] = 0x0001;
	//readcmd.attrID[2] = 0x0010;
	//readcmd.attrID[3] = 0x0011;

#if 0
	//test:
	read_report = malloc(sizeof(zclReadReportCfgCmd_t) + sizeof(zclReadReportCfgRec_t));
	read_report->numAttr = 1;
	read_report->attrList[0].direction = 0;
	read_report->attrList[0].attrID = 0x0000;
#endif
	/*init the timestemp of all active device*/
	//sleep(10);
	list_for_each_safe(pos, n, &gw->head) { 
		d = list_entry(pos, struct device, list); 
		//if(d && (!device_check_status(d, DEVICE_APP_DEL)) && (!device_check_status(d, DEVICE_LEAVE_NET))) {
		if(d && (device_check_status(d, DEVICE_APP_ADD)) && (!device_check_status(d, DEVICE_LEAVE_NET))) {
			//d->timestamp = time(NULL);
			d->accesscnt = 0;
		}
	}

	while(1) {
		if(initDone) {
			pthread_mutex_lock(&big_mutex);
			//printf("[send_read_onoff] lock\n");
			list_for_each_safe(pos, n, &gw->head) { 
			if(!pos || !n) {
				printf("[send_read_onoff]somewhere is delete device list, repoll them\n");
				break;
			}
				d = list_entry(pos, struct device, list); 
				if(d && device_check_status(d, DEVICE_APP_ADD) && 
					(!device_check_status(d, DEVICE_LEAVE_NET)) && (d->accesscnt > 0)) {	
					printf("d->accesscnt = %d\n", d->accesscnt);
					//printf("send read cmd ieee:%llx\n", d->ieeeaddr);
					need_delay = 1;
					list_for_each_safe(ep_pos, ep_n, &d->eplisthead) {
						if(!ep_pos || !ep_n) {
							printf("[send_read_onoff]somewhere is delete ep list, repoll them\n");
							break;
						}
						ep = list_entry(ep_pos, struct endpoint, list);
						if(ep) {							
							if((ep->simpledesc.simpledesc.DeviceID != ZCL_HA_DEVICEID_MAINS_POWER_OUTLET) && 
								(ep->simpledesc.simpledesc.DeviceID != ZCL_HA_DEVICEID_ON_OFF_OUTPUT) && 
								(ep->simpledesc.simpledesc.DeviceID != ZCL_HA_DEVICEID_SHADE)) {
								//(ep->simpledesc.simpledesc.DeviceID != ZCL_HA_DEVICEID_WINDOW_COVERING_DEVICE))
								continue;
							}
							printf("send read cmd endpoint:%d\n", ep->simpledesc.simpledesc.Endpoint);
							//#if 0
							if(check_device_timeout(d)) {
								printf("send_read_onoff:d->accesscnt = 0\n");
								d->accesscnt = 0;
								continue;
							}
							//#endif
							unsigned short cluster_id = (ep->simpledesc.simpledesc.DeviceID == ZCL_HA_DEVICEID_SHADE ? ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL : ZCL_CLUSTER_ID_GEN_ON_OFF);
							//unsigned short cluster_id;
							/*if(cluster_id == ZCL_HA_DEVICEID_SHADE || cluster_id == ZCL_HA_DEVICEID_WINDOW_COVERING_DEVICE)
								cluster_id = ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL;
							else
								cluster_id = ZCL_CLUSTER_ID_GEN_ON_OFF;*/
							unsigned char epnum = ep->simpledesc.simpledesc.Endpoint;
							unsigned short shortaddr = d->shortaddr;
							pthread_mutex_unlock(&big_mutex);
							printf("[send_read_onoff] zcl_SendRead start\n");
							//zcl_SendRead(1, ep->simpledesc.simpledesc.Endpoint, d->shortaddr, cluster_id, &readcmd, ZCL_FRAME_CLIENT_SERVER_DIR,0,get_sequence());
							zcl_SendRead(1, epnum, shortaddr, cluster_id, &readcmd, ZCL_FRAME_CLIENT_SERVER_DIR,0,get_sequence());
							printf("[send_read_onoff] zcl_SendRead end\n");
							pthread_mutex_lock(&big_mutex);
						}
					}					
					//sleep(interval);
				}
				else {
					//printf("DEVICE_APP_DEL:%d\n", (d->status & DEVICE_APP_DEL) > 0);
					//printf("DEVICE_LEAVE_NET:%d\n", (d->status & DEVICE_LEAVE_NET) > 0);
					//printf("d->noneedcheck:%d\n", d->noneedcheck);
				}
			}
			//printf("[send_read_onoff] unlock\n");
			pthread_mutex_unlock(&big_mutex);
			if(need_delay) {
				sleep(3);
				need_delay = 0;
			}
		}
	}
	//free(read_report);
}


static int emergency_device(struct device *d)
{
	struct list_head *pos, *n;
	struct endpoint *ep;

	list_for_each_safe(pos, n, &d->eplisthead) {
		ep = list_entry(pos, struct endpoint, list);
		printf("[emergency_device] zonetype: 0x%04x\n", ep->simpledesc.zonetype);
		if(SS_IAS_ZONE_TYPE_KEY_FOB == ep->simpledesc.zonetype || 
			ZCL_HA_DEVICEID_MAINS_POWER_OUTLET == ep->simpledesc.simpledesc.DeviceID || 
			ZCL_HA_DEVICEID_ON_OFF_OUTPUT == ep->simpledesc.simpledesc.DeviceID ||
			ZCL_HA_DEVICEID_IAS_WARNING_DEVICE == ep->simpledesc.simpledesc.DeviceID ||
			ZCL_HA_DEVICEID_SHADE == ep->simpledesc.simpledesc.DeviceID)
			return 1;
	}
	return 0;
}

//#if 0
void *check_response_timeout(void *args)
{
	struct device *d;
	struct list_head *pos, *n;
	time_t t, diff;
	int need_update = 0;
	
	while(1) {
		if(initDone) {
			sleep(9);
			pthread_mutex_lock(&big_mutex);
			list_for_each_safe(pos, n, &getgateway()->head) { 
				d = list_entry(pos, struct device, list); 
				if(device_check_status(d, DEVICE_APP_ADD) && (!device_check_status(d, DEVICE_LEAVE_NET))) {
					t = time(NULL);
					diff = t - d->timestamp;
					printf("[check_response_timeout] ieee:%llx online = %d, diff = %ld\n", d->ieeeaddr,d->online, diff);
					if(emergency_device(d)) {
						printf("emergency_device\n");
						if(d->online && diff > 300) {
							printf("offline\n");
							d->online = 0;
							need_update = 1;
						}
						else if(!d->online && diff <= 300){
							printf("online\n");
							d->online = 1;
							need_update = 1;
						}
					}
					else {
						if(d->online && diff > 3600) {
							printf("offline\n");
							d->online = 0;
							need_update = 1;
						}
						else if(!d->online && diff <= 3600){
							printf("online\n");
							d->online = 1;
							need_update = 1;
						}
					}
					if(1 == need_update){
						need_update = 0;
						unsigned char sbuf[32] = {0};
						struct protocol_cmdtype_report_online online;
						unsigned int len;
						online.ieee = d->ieeeaddr;
						online.on = d->online;
						sqlitedb_update_device_online(d);
						len = protocol_encode_report_online(sbuf, &online);
						printf("=============\n============\n");
						broadcast(sbuf, len);
					}
				}
			}
			pthread_mutex_unlock(&big_mutex);
		}
	}
	return NULL;
}


static int need_send_request(struct device *d)
{
	struct list_head *pos, *n;
	struct endpoint *ep;

	list_for_each_safe(pos, n, &d->eplisthead) {
		if(!pos || !n) {
			return 0;
		}
		ep = list_entry(pos, struct endpoint, list);
		if(((ZCL_HA_DEVICEID_IAS_ZONE == ep->simpledesc.simpledesc.DeviceID) &&
			(strstr(d->modelidentifier, "A01") == NULL)) || 
			(ZCL_HA_DEVICEID_IAS_ANCILLARY_CONTROL_EQUIPMENT == ep->simpledesc.simpledesc.DeviceID))
			return 0;
	}
	return 1;
}

void *send_basic_request(void *args)
{
	struct gateway *gw = getgateway();
	struct list_head * pos, *n;
	struct device *d;
	zclReadCmd_t readcmd; 
	pthread_t check_tid;
	unsigned short dstaddr;

	readcmd.numAttr = 1;
	readcmd.attrID[0] = ATTRID_BASIC_ZCL_VERSION;
	
	printf("send_basic_request...\n");
	pthread_mutex_lock(&big_mutex);
	list_for_each_safe(pos, n, &gw->head) { 
		d = list_entry(pos, struct device, list); 
		if(device_check_status(d, DEVICE_APP_ADD) && (!device_check_status(d, DEVICE_LEAVE_NET))) {
			//d->timestamp = time(NULL);
			if(d->online)
				d->timestamp = time(NULL);
		}
	}
	pthread_mutex_unlock(&big_mutex);

	pthread_create(&check_tid, NULL, check_response_timeout, NULL);
	while(1) {
		if(initDone) {
			sleep(10);
			printf("[send_basic_request] top poll\n");
			pthread_mutex_lock(&big_mutex);
			list_for_each_safe(pos, n, &getgateway()->head) { 
				d = list_entry(pos, struct device, list); 
				if(!pos || !n) {
					printf("[send_basic_request]somewhere is delete ep list, repoll them\n");
					break;
				}
				if(device_check_status(d, DEVICE_APP_ADD) && (!device_check_status(d, DEVICE_LEAVE_NET))) {
					if(need_send_request(d)) {
						dstaddr = d->shortaddr;
						pthread_mutex_unlock(&big_mutex);
						printf("[send_basic_request] ieee:%llx\n", d->ieeeaddr);
						zcl_SendRead(1, 1, dstaddr, ZCL_CLUSTER_ID_GEN_BASIC, &readcmd, 0, 0, get_sequence());
						sleep(9);
						pthread_mutex_lock(&big_mutex);
					}
				}
			}
			pthread_mutex_unlock(&big_mutex);
		}
	}
	return NULL;
}
//#endif
void send_period_request(void)
{
	pthread_t basic_tid;
	pthread_t onoff_tid;
	pthread_create(&onoff_tid, NULL, send_read_onoff, NULL);
	pthread_create(&basic_tid, NULL, send_basic_request, NULL);
}
//#endif
