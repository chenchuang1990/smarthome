
SBU_REV= "0.1"

INCLUDE = -I$(PROJ_DIR)../ -I$(PROJ_DIR)../framework/platform/gnu -I$(PROJ_DIR)../framework/rpc/ -I$(PROJ_DIR)../framework/mt/ -I$(PROJ_DIR)../framework/mt/Af -I$(PROJ_DIR)../framework/mt/Zdo -I$(PROJ_DIR)../framework/mt/Sys -I$(PROJ_DIR)../framework/mt/Sapi -I$(PROJ_DIR)../znp/zcl -I$(PROJ_DIR)../znp -I$(PROJ_DIR)../sqlite/include -I$(PROJ_DIR)../protocol -L$(PROJ_DIR)../sqlite/lib

#CC= gcc
CC=arm-arago-linux-gnueabi-gcc

CFLAGS= -c -Wall -g -rdynamic -std=gnu99
LIBS = -lpthread -lrt -lm -ldl
DEFS += -DxCC26xx
PROJ_DIR=

all: gateway.bin

gateway.bin: main.o rpc.o mtParser.o mtZdo.o mtSys.o mtAf.o mtSapi.o dbgPrint.o rpcTransport.o queue.o appinit.o commands.o zcl.o zcl_ss.o ceconf.o cetimer.o connection.o event.o eventhub.o gateway.o kfifo.o  rbtree.o reconn.o socket.o toolkit.o znp.o cJSON.o sqlitedb.o sqlite3.o zcl_down_cmd.o zcl_general.o zcl_register_cluster.o protocol.o protocol_encode_add_del_device.o protocol_encode_alarm.o protocol_encode_arm_feedback.o protocol_encode_response.o protocol_encode_heartbeat.o protocol_encode_login.o protocol_parse_arm.o protocol_parse_identify.o protocol_parse_warning.o protocol_parse_set_device_name.o protocol_parse_get_device_attr.o protocol_parse_del_device.o protocol_encode_set_name_feedback.o protocol_encode_deviceattr.o protocol_encode_del_device_feedback.o protocol_parse_onoff.o protocol_parse_level_ctrl.o protocol_parse_permit_joing.o key.o network_test.o catsegfault.o
	$(CC) main.o appinit.o rpc.o mtParser.o mtZdo.o mtSys.o mtAf.o mtSapi.o dbgPrint.o rpcTransport.o queue.o commands.o zcl.o zcl_ss.o ceconf.o cetimer.o connection.o event.o eventhub.o gateway.o kfifo.o rbtree.o reconn.o socket.o toolkit.o znp.o cJSON.o sqlitedb.o sqlite3.o zcl_down_cmd.o zcl_general.o zcl_register_cluster.o protocol.o protocol_encode_add_del_device.o protocol_encode_alarm.o protocol_encode_arm_feedback.o protocol_encode_response.o protocol_encode_heartbeat.o protocol_encode_login.o protocol_parse_arm.o protocol_parse_identify.o protocol_parse_warning.o protocol_parse_set_device_name.o protocol_parse_get_device_attr.o protocol_parse_del_device.o protocol_encode_set_name_feedback.o protocol_encode_deviceattr.o protocol_encode_del_device_feedback.o protocol_parse_onoff.o protocol_parse_level_ctrl.o protocol_parse_permit_joing.o key.o network_test.o catsegfault.o $(LIBS) -o gateway.bin

# rule for file "main.o".
main.o: ../main.c
	$(CC) $(CFLAGS) $(INCLUDE) $(DEFS) $(PROJ_DIR)../main.c

# rule for file "rpc.o".
rpc.o: $(PROJ_DIR)../framework/rpc/rpc.h $(PROJ_DIR)../framework/rpc/rpc.c
	$(CC) $(CFLAGS) $(INCLUDE) $(DEFS) $(PROJ_DIR)../framework/rpc/rpc.c

# rule for file "mtParser.o".
mtParser.o: $(PROJ_DIR)../framework/mt/mtParser.h $(PROJ_DIR)../framework/mt/mtParser.c
	$(CC) $(CFLAGS) $(INCLUDE) $(DEFS) $(PROJ_DIR)../framework/mt/mtParser.c

# rule for file "mtZdo.o".
mtZdo.o: $(PROJ_DIR)../framework/mt/Zdo/mtZdo.h $(PROJ_DIR)../framework/mt/Zdo/mtZdo.c
	$(CC) $(CFLAGS) $(INCLUDE) $(DEFS) $(PROJ_DIR)../framework/mt/Zdo/mtZdo.c

# rule for file "mtSys.o".
mtSys.o: $(PROJ_DIR)../framework/mt/Sys/mtSys.h $(PROJ_DIR)../framework/mt/Sys/mtSys.c
	$(CC) $(CFLAGS) $(INCLUDE) $(DEFS) $(PROJ_DIR)../framework/mt/Sys/mtSys.c

# rule for file "mtAf.o".
mtAf.o: $(PROJ_DIR)../framework/mt/Af/mtAf.h $(PROJ_DIR)../framework/mt/Af/mtAf.c
	$(CC) $(CFLAGS) $(INCLUDE) $(DEFS) $(PROJ_DIR)../framework/mt/Af/mtAf.c

# rule for file "mtSapi.o".
mtSapi.o: $(PROJ_DIR)../framework/mt/Sapi/mtSapi.h $(PROJ_DIR)../framework/mt/Sapi/mtSapi.c
	$(CC) $(CFLAGS) $(INCLUDE) $(DEFS) $(PROJ_DIR)../framework/mt/Sapi/mtSapi.c

# rule for file "dbgPrint.o".
dbgPrint.o: $(PROJ_DIR)../framework/platform/gnu/dbgPrint.h $(PROJ_DIR)../framework/platform/gnu/dbgPrint.c
	$(CC) $(CFLAGS) $(INCLUDE) $(DEFS) $(PROJ_DIR)../framework/platform/gnu/dbgPrint.c

# rule for file "rpcTransport.o".
rpcTransport.o: $(PROJ_DIR)../framework/platform/gnu/rpcTransport.h $(PROJ_DIR)../framework/platform/gnu/rpcTransport.c $(PROJ_DIR)../framework/platform/gnu/rpcTransportUart.c
	$(CC) $(CFLAGS) $(INCLUDE) $(DEFS) $(PROJ_DIR)../framework/platform/gnu/rpcTransport.c

# rule for file "queue.o".
queue.o: $(PROJ_DIR)../framework/rpc/queue.h $(PROJ_DIR)../framework/rpc/queue.c
	$(CC) $(CFLAGS) $(INCLUDE) $(DEFS) $(PROJ_DIR)../framework/rpc/queue.c

# rule for file "appinit.o".
appinit.o: $(PROJ_DIR)../znp/appinit.h $(PROJ_DIR)../znp/appinit.c
	$(CC) $(CFLAGS) $(INCLUDE) $(DEFS) $(PROJ_DIR)../znp/appinit.c

# rule for file "commands.o".
commands.o: $(PROJ_DIR)../znp/commands.h $(PROJ_DIR)../znp/commands.c
	$(CC) $(CFLAGS) $(INCLUDE) $(DEFS) $(PROJ_DIR)../znp/commands.c

# rule for file "zcl.o".
zcl.o: $(PROJ_DIR)../znp/zcl/zcl.h $(PROJ_DIR)../znp/zcl/zcl.c
	$(CC) $(CFLAGS) $(INCLUDE) $(DEFS) $(PROJ_DIR)../znp/zcl/zcl.c

# rule for file "zcl_ss.o".
zcl_ss.o: $(PROJ_DIR)../znp/zcl/zcl_ss.h $(PROJ_DIR)../znp/zcl/zcl_ss.c
	$(CC) $(CFLAGS) $(INCLUDE) $(DEFS) $(PROJ_DIR)../znp/zcl/zcl_ss.c

# rule for file "ceconf.o".
ceconf.o: $(PROJ_DIR)../ceconf.h $(PROJ_DIR)../ceconf.c
	$(CC) $(CFLAGS) $(INCLUDE) $(DEFS) $(PROJ_DIR)../ceconf.c

# rule for file "cetimer.o".
cetimer.o: $(PROJ_DIR)../cetimer.h $(PROJ_DIR)../cetimer.c
	$(CC) $(CFLAGS) $(INCLUDE) $(DEFS) $(PROJ_DIR)../cetimer.c

# rule for file "connection.o".
connection.o: $(PROJ_DIR)../connection.h $(PROJ_DIR)../connection.c
	$(CC) $(CFLAGS) $(INCLUDE) $(DEFS) $(PROJ_DIR)../connection.c
	
# rule for file "event.o".
event.o: $(PROJ_DIR)../event.h $(PROJ_DIR)../event.c
	$(CC) $(CFLAGS) $(INCLUDE) $(DEFS) $(PROJ_DIR)../event.c

# rule for file "eventhub.o".
eventhub.o: $(PROJ_DIR)../eventhub.h $(PROJ_DIR)../eventhub.c
	$(CC) $(CFLAGS) $(INCLUDE) $(DEFS) $(PROJ_DIR)../eventhub.c
	
# rule for file "gateway.o".
gateway.o: $(PROJ_DIR)../gateway.h $(PROJ_DIR)../gateway.c
	$(CC) $(CFLAGS) $(INCLUDE) $(DEFS) $(PROJ_DIR)../gateway.c

# rule for file "kfifo.o".
kfifo.o: $(PROJ_DIR)../kfifo.h $(PROJ_DIR)../kfifo.c
	$(CC) $(CFLAGS) $(INCLUDE) $(DEFS) $(PROJ_DIR)../kfifo.c

# rule for file "rbtree.o".
rbtree.o: $(PROJ_DIR)../rbtree.h $(PROJ_DIR)../rbtree.c
	$(CC) $(CFLAGS) $(INCLUDE) $(DEFS) $(PROJ_DIR)../rbtree.c

# rule for file "reconn.o".
reconn.o: $(PROJ_DIR)../reconn.h $(PROJ_DIR)../reconn.c
	$(CC) $(CFLAGS) $(INCLUDE) $(DEFS) $(PROJ_DIR)../reconn.c
	
# rule for file "socket.o".
socket.o: $(PROJ_DIR)../socket.h $(PROJ_DIR)../socket.c
	$(CC) $(CFLAGS) $(INCLUDE) $(DEFS) $(PROJ_DIR)../socket.c

# rule for file "toolkit.o".
toolkit.o: $(PROJ_DIR)../toolkit.h $(PROJ_DIR)../toolkit.c
	$(CC) $(CFLAGS) $(INCLUDE) $(DEFS) $(PROJ_DIR)../toolkit.c

# rule for file "parseserver.o".
parseserver.o: $(PROJ_DIR)../parseserver.h $(PROJ_DIR)../parseserver.c
	$(CC) $(CFLAGS) $(INCLUDE) $(DEFS) $(PROJ_DIR)../parseserver.c

# rule for file "znp.o".
znp.o: $(PROJ_DIR)../znp.h $(PROJ_DIR)../znp.c
	$(CC) $(CFLAGS) $(INCLUDE) $(DEFS) $(PROJ_DIR)../znp.c

# rule for file "cJSON.o".
cJSON.o: $(PROJ_DIR)../cJSON.h $(PROJ_DIR)../cJSON.c
	$(CC) $(CFLAGS) $(INCLUDE) $(DEFS) $(PROJ_DIR)../cJSON.c

# rule for file "sqlitedb.o".
sqlitedb.o: $(PROJ_DIR)../sqlitedb.h $(PROJ_DIR)../sqlitedb.c
	$(CC) $(CFLAGS) $(INCLUDE) $(DEFS) $(PROJ_DIR)../sqlitedb.c

# rule for file "sqlite3.o".
sqlite3.o: $(PROJ_DIR)../sqlite/include/sqlite3.h $(PROJ_DIR)../sqlite/sqlite3.c
	$(CC) $(CFLAGS) $(INCLUDE) $(DEFS) $(PROJ_DIR)../sqlite/sqlite3.c

# rule for file "zcl_down_cmd.o".
zcl_down_cmd.o: $(PROJ_DIR)../znp/zcl/zcl_down_cmd.h $(PROJ_DIR)../znp/zcl/zcl_down_cmd.c
	$(CC) $(CFLAGS) $(INCLUDE) $(DEFS) $(PROJ_DIR)../znp/zcl/zcl_down_cmd.c

# rule for file "zcl_general.o".
zcl_general.o: $(PROJ_DIR)../znp/zcl/zcl_general.h $(PROJ_DIR)../znp/zcl/zcl_general.c
	$(CC) $(CFLAGS) $(INCLUDE) $(DEFS) $(PROJ_DIR)../znp/zcl/zcl_general.c

# rule for file "zcl_register_cluster.o".
zcl_register_cluster.o: $(PROJ_DIR)../znp/zcl/zcl_register_cluster.h $(PROJ_DIR)../znp/zcl/zcl_register_cluster.c
	$(CC) $(CFLAGS) $(INCLUDE) $(DEFS) $(PROJ_DIR)../znp/zcl/zcl_register_cluster.c

# rule for file "protocl.o".
protocol.o: $(PROJ_DIR)../protocol/protocol.h $(PROJ_DIR)../protocol/protocol.c
	$(CC) $(CFLAGS) $(INCLUDE) $(DEFS) $(PROJ_DIR)../protocol/protocol.c

# rule for file "protocol_encode_add_del_device.o".
protocol_encode_add_del_device.o: $(PROJ_DIR)../protocol/protocol_encode_add_del_device.c
	$(CC) $(CFLAGS) $(INCLUDE) $(DEFS) $(PROJ_DIR)../protocol/protocol_encode_add_del_device.c

# rule for file "protocol_encode_alarm.o".
protocol_encode_alarm.o: $(PROJ_DIR)../protocol/protocol_encode_alarm.c
	$(CC) $(CFLAGS) $(INCLUDE) $(DEFS) $(PROJ_DIR)../protocol/protocol_encode_alarm.c

# rule for file "protocol_encode_arm_feedback.o".
protocol_encode_arm_feedback.o: $(PROJ_DIR)../protocol/protocol_encode_arm_feedback.c
	$(CC) $(CFLAGS) $(INCLUDE) $(DEFS) $(PROJ_DIR)../protocol/protocol_encode_arm_feedback.c

# rule for file "protocol_encode_response.o".
protocol_encode_response.o: $(PROJ_DIR)../protocol/protocol_encode_response.c
	$(CC) $(CFLAGS) $(INCLUDE) $(DEFS) $(PROJ_DIR)../protocol/protocol_encode_response.c
	
# rule for file "protocol_encode_del_device_feedback.o".
protocol_encode_del_device_feedback.o: $(PROJ_DIR)../protocol/protocol_encode_del_device_feedback.c
	$(CC) $(CFLAGS) $(INCLUDE) $(DEFS) $(PROJ_DIR)../protocol/protocol_encode_del_device_feedback.c

# rule for file "protocol_encode_deviceattr.o".
protocol_encode_deviceattr.o: $(PROJ_DIR)../protocol/protocol_encode_deviceattr.c
	$(CC) $(CFLAGS) $(INCLUDE) $(DEFS) $(PROJ_DIR)../protocol/protocol_encode_deviceattr.c

# rule for file "protocol_encode_heartbeat.o".
protocol_encode_heartbeat.o: $(PROJ_DIR)../protocol/protocol_encode_heartbeat.c
	$(CC) $(CFLAGS) $(INCLUDE) $(DEFS) $(PROJ_DIR)../protocol/protocol_encode_heartbeat.c

# rule for file "protocol_encode_login.o".
protocol_encode_login.o: $(PROJ_DIR)../protocol/protocol_encode_login.c
	$(CC) $(CFLAGS) $(INCLUDE) $(DEFS) $(PROJ_DIR)../protocol/protocol_encode_login.c

# rule for file "protocol_encode_set_name_feedback.o".
protocol_encode_set_name_feedback.o: $(PROJ_DIR)../protocol/protocol_encode_set_name_feedback.c
	$(CC) $(CFLAGS) $(INCLUDE) $(DEFS) $(PROJ_DIR)../protocol/protocol_encode_set_name_feedback.c

# rule for file "protocol_parse_arm.o".
protocol_parse_arm.o: $(PROJ_DIR)../protocol/protocol_parse_arm.c
	$(CC) $(CFLAGS) $(INCLUDE) $(DEFS) $(PROJ_DIR)../protocol/protocol_parse_arm.c

# rule for file "protocol_parse_del_device.o".
protocol_parse_del_device.o: $(PROJ_DIR)../protocol/protocol_parse_del_device.c
	$(CC) $(CFLAGS) $(INCLUDE) $(DEFS) $(PROJ_DIR)../protocol/protocol_parse_del_device.c
	
# rule for file "protocol_parse_get_device_attr.o".
protocol_parse_get_device_attr.o: $(PROJ_DIR)../protocol/protocol_parse_get_device_attr.c
	$(CC) $(CFLAGS) $(INCLUDE) $(DEFS) $(PROJ_DIR)../protocol/protocol_parse_get_device_attr.c

# rule for file "protocol_parse_identify.o".
protocol_parse_identify.o: $(PROJ_DIR)../protocol/protocol_parse_identify.c
	$(CC) $(CFLAGS) $(INCLUDE) $(DEFS) $(PROJ_DIR)../protocol/protocol_parse_identify.c

# rule for file "protocol_parse_set_device_name.o".
protocol_parse_set_device_name.o: $(PROJ_DIR)../protocol/protocol_parse_set_device_name.c
	$(CC) $(CFLAGS) $(INCLUDE) $(DEFS) $(PROJ_DIR)../protocol/protocol_parse_set_device_name.c

# rule for file "protocol_parse_warning.o".
protocol_parse_warning.o: $(PROJ_DIR)../protocol/protocol_parse_warning.c
	$(CC) $(CFLAGS) $(INCLUDE) $(DEFS) $(PROJ_DIR)../protocol/protocol_parse_warning.c

# rule for file "protocol_parse_onoff.o".
protocol_parse_onoff.o: $(PROJ_DIR)../protocol/protocol_parse_onoff.c
	$(CC) $(CFLAGS) $(INCLUDE) $(DEFS) $(PROJ_DIR)../protocol/protocol_parse_onoff.c

# rule for file "protocol_parse_level_ctrl.o".
protocol_parse_level_ctrl.o: $(PROJ_DIR)../protocol/protocol_parse_level_ctrl.c
	$(CC) $(CFLAGS) $(INCLUDE) $(DEFS) $(PROJ_DIR)../protocol/protocol_parse_level_ctrl.c

# rule for file "protocol_parse_permit_joing.o".
protocol_parse_permit_joing.o: $(PROJ_DIR)../protocol/protocol_parse_permit_joing.c
	$(CC) $(CFLAGS) $(INCLUDE) $(DEFS) $(PROJ_DIR)../protocol/protocol_parse_permit_joing.c

# rule for file "key.o".
key.o: ../key.c
	$(CC) $(CFLAGS) $(INCLUDE) $(DEFS) $(PROJ_DIR)../key.c

# rule for file "network_test.o".
network_test.o: ../network_test.c
	$(CC) $(CFLAGS) $(INCLUDE) $(DEFS) $(PROJ_DIR)../network_test.c

# rule for file "catsegfault.o".
catsegfault.o: ../catsegfault.c
	$(CC) $(CFLAGS) $(INCLUDE) $(DEFS) $(PROJ_DIR)../catsegfault.c
# rule for cleaning files generated during compilations.
clean:
	/bin/rm -f gateway.bin *.o
