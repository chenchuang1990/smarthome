
/*
 ********************************************
                   _ooOoo_
                  o8888888o
                  88" . "88
                  (| -_- |)
                  O\  =  /O
               ____/`---'\____
             .'  \\|     |//  `.
            /  \\|||  :  |||//  \
           /  _||||| -:- |||||-  \
           |   | \\\  -  /// |   |
           | \_|  ''\---/''  |   |
           \  .-\__  `-`  ___/-. /
         ___`. .'  /--.--\  `. . __
      ."" '<  `.___\_<|>_/___.'  >'"".
     | | :  `- \`.;`\ _ /`;.`/ - ` : | |
     \  \ `-.   \_ __\ /__ _/   .-` /  /
======`-.____`-.___\_____/___.-`____.-'======
                   `=---='
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
         ·ð×æ±£ÓÓ       ÓÀÎÞBUG
*/


#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "eventhub.h"
#include "connection.h"
#include "toolkit.h"
#include "cetimer.h"
#include "socket.h"
#include "ceconf.h"
#include "connection.h"
#include "reconn.h"
#include "gateway.h"
#include "znp.h"
#include "sqlitedb.h"
#include "key.h"
#include "network_test.h"
#include "sequence.h"
#include "addtion.h"

//#define CATCH_SEGFAULT

#define APP_TIME 0x0823

#define SRCPATH "/media/mmcblk0p1/srcseq.txt"
#define DSTPATH "/home/root/dstseq.txt"

int g_main_to_znp_write_fd = -1;

pthread_mutex_t big_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t conn_mutex = PTHREAD_MUTEX_INITIALIZER;

int main() 
{
	//ceconf_load();
	printf("test version :%04x\n", APP_TIME);
	//system("ntpdate s2m.time.edu.cn");
	#ifdef CATCH_SEGFAULT
	sigaction_init();
	#endif
	//sigpipe_ignore();
	check_usb_function();
	sqlitedb_table_build(DBPATH);
	
	if(0 == access(SRCPATH, F_OK)) {
		printf("toolkit_copy2...\n");
		toolkit_copy2(DSTPATH, SRCPATH);
	}
	unsigned long long mac = load_sequence(DSTPATH);
	printf("mac is %llx\n", mac);
	if(0 == mac) {
		printf("load_sequence error\n");
		return -1;
	}
	//unsigned long long mac = toolkit_getmac();
	if(sqlitedb_load_gateway_name(DBPATH, mac)) { 
		gateway_init(getgateway(), mac, "ç½‘å…³", 1, 2);
		sqlitedb_add_gateway(mac, "ç½‘å…³"); 
	}
	sqlitedb_load_device();

	// create pipe for timer to main
	int wfd;
	struct connection * readconn = createpipe(&wfd);

	// create pipe for timer to reconnect
	int reconnrfd, reconnwfd;
	reconnrfd = createpipe2(&reconnwfd);

	//  create pipe for reconnect to main
	int rmwfd;
	struct connection * mrreadconn = createpipe(&rmwfd);
#if 0
	// create timer
	struct cetimer * timer = cetimer_create(5, 1, wfd, reconnwfd);
	cetimer_start(timer);
#endif
	// create eventhub 
	struct eventhubconf hubconf;
	memset(&hubconf, 0, sizeof(struct eventhubconf));
	memcpy(&hubconf.port, ceconf_getlistenport(),strlen(ceconf_getlistenport())); 
	struct eventhub * hub = eventhub_create(&hubconf);

	// start reconn thread
	reconn_start(reconnrfd, rmwfd,  hub);
	
	// add cmd pipe
	if(readconn){
		eventhub_register(hub, connection_getfd(readconn));
	}

	// add reconn to main rfd
	if( mrreadconn ){
		eventhub_register(hub, connection_getfd(mrreadconn));
	}
	
	// create timer
	struct cetimer * timer = cetimer_create(5, 1, wfd, reconnwfd);
	cetimer_start(timer);

	// create pipe for znp to main
	int mainrfd, znpwfd;
	mainrfd = createpipe2(&znpwfd);
	make_socket_non_blocking(mainrfd);
	pthread_mutex_lock(&conn_mutex);
	printf("[main]freeconnlist_getconn lock\n");
	struct connection * znpconnection = freeconnlist_getconn();
	printf("[main]freeconnlist_getconn unlock\n");
	pthread_mutex_unlock(&conn_mutex);
	if(znpconnection)
		connection_init(znpconnection, mainrfd, CONNZNP);
	else {
		printf("error:znpconnetcion is NULL!\n");
		return -1;
	}

	// create pipe for main to znp
	int mainwfd, znprfd;
	znprfd = createpipe2(&mainwfd);
	make_socket_non_blocking(mainwfd);
	g_main_to_znp_write_fd = mainwfd;
	//fprintf(stdout, "znp main write %d znp read %d\n", g_main_to_znp_write_fd, znprfd);

	// open serial port
	if(znp_start(znpwfd, znprfd, ceconf_getserialport()) == -1){
		return 1;
	}
	//set_led_onoff(LED_Z, LED_ON);
	network_test_start();
	if(znpconnection){
		eventhub_register(hub, connection_getfd(znpconnection));
		pthread_mutex_lock(&conn_mutex);
		printf("[main]znpconnection lock\n");
		connrbtree_insert(znpconnection);
		printf("[main]znpconnection unlock\n");
		pthread_mutex_unlock(&conn_mutex);
	}
	send_period_request();

	eventhub_start(hub);
	pthread_mutex_destroy(&big_mutex);
	pthread_mutex_destroy(&conn_mutex);
}
