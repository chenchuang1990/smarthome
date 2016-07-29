/*
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
         ���汣��       ����BUG
*/
//       ��Ի:    
//             д��¥��д�ּ䣬д�ּ������Ա��    
//             ������Աд�������ó��򻻾�Ǯ��    
//             ����ֻ���������������������ߣ�    
//             ��������ո��գ����������긴�ꡣ    
//             ��Ը�������Լ䣬��Ը�Ϲ��ϰ�ǰ��    
//             ���۱������Ȥ���������г���Ա��    
//             ����Ц��߯��񲣬��Ц�Լ���̫����    
//             ��������Ư���ã��ĸ���ó���Ա��


#include <string.h>
#include <stdio.h>
#include <pthread.h>
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

#define CATCH_SEGFAULT

#define APP_TIME 0x0624

int g_main_to_znp_write_fd = -1;
int main() 
{
	//ceconf_load();
	printf("test version :%04x\n", APP_TIME);
	//system("ntpdate s2m.time.edu.cn");
	#ifdef CATCH_SEGFAULT
	catchsegfault();
	#endif
	sqlitedb_table_build(DBPATH);

	unsigned long long mac = toolkit_getmac();
	if(sqlitedb_load_gateway_name(DBPATH, mac)) { 
		gateway_init(getgateway(), mac, "网关", 1, 1);
		sqlitedb_add_gateway(mac, "网关"); 
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

	// create timer
	struct cetimer * timer = cetimer_create(5, 1, wfd, reconnwfd);
	cetimer_start(timer);

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

	// create pipe for znp to main
	int mainrfd, znpwfd;
	mainrfd = createpipe2(&znpwfd);
	make_socket_non_blocking(mainrfd);
	struct connection * znpconnection = freeconnlist_getconn();
	connection_init(znpconnection, mainrfd, CONNZNP);

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
		connrbtree_insert(znpconnection);
	}
	//send_period_request();

	eventhub_start(hub);
}
