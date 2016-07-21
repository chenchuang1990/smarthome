
SBU_REV= "0.1"


INCLUDE = -I$(PROJ_DIR)../../ -I$(PROJ_DIR)../framework/platform/gnu -I$(PROJ_DIR)../framework/rpc/ -I$(PROJ_DIR)../framework/mt/ -I$(PROJ_DIR)../framework/mt/Af -I$(PROJ_DIR)../framework/mt/Zdo -I$(PROJ_DIR)../framework/mt/Sys -I$(PROJ_DIR)../framework/mt/Sapi -I$(PROJ_DIR)./zcl -I./

CC= gcc
#CC=/usr/local/angstrom/arm/bin/arm-angstrom-linux-gnueabi-gcc

CFLAGS= -c -Wall -g -std=gnu99
LIBS = -lpthread -lrt
DEFS += -DxCC26xx
PROJ_DIR=

all: dataSendRcv.bin

dataSendRcv.bin: main.o appinit.o rpc.o mtParser.o mtZdo.o mtSys.o mtAf.o mtSapi.o dbgPrint.o rpcTransport.o queue.o commands.o zcl.o zcl_ss.o 
	$(CC) main.o appinit.o rpc.o mtParser.o mtZdo.o mtSys.o mtAf.o mtSapi.o dbgPrint.o rpcTransport.o queue.o commands.o zcl.o zcl_ss.o $(LIBS) -o dataSendRcv.bin

# rule for file "main.o".
main.o: main.c
	$(CC) $(CFLAGS) $(INCLUDE) $(DEFS) $(PROJ_DIR)main.c


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
appinit.o: $(PROJ_DIR)./appinit.h $(PROJ_DIR)./appinit.c
	$(CC) $(CFLAGS) $(INCLUDE) $(DEFS) $(PROJ_DIR)./appinit.c

# rule for file "appinit.o".
appinit.o: $(PROJ_DIR)./appinit.h $(PROJ_DIR)./appinit.c ./commands.o
	$(CC) $(CFLAGS) $(INCLUDE) $(DEFS) $(PROJ_DIR)./appinit.c

# rule for file "commands.o".
commands.o: $(PROJ_DIR)./commands.h $(PROJ_DIR)./commands.c
	$(CC) $(CFLAGS) $(INCLUDE) $(DEFS) $(PROJ_DIR)./commands.c

# rule for file "zcl.o".
zcl.o: $(PROJ_DIR)./zcl/zcl.h $(PROJ_DIR)./zcl/zcl.c
	$(CC) $(CFLAGS) $(INCLUDE) $(DEFS) $(PROJ_DIR)./zcl/zcl.c

# rule for file "zcl_ss.o".
zcl_ss.o: $(PROJ_DIR)./zcl/zcl_ss.h $(PROJ_DIR)./zcl/zcl_ss.c
	$(CC) $(CFLAGS) $(INCLUDE) $(DEFS) $(PROJ_DIR)./zcl/zcl_ss.c

# rule for cleaning files generated during compilations.
clean:
	/bin/rm -f dataSendRcv.bin *.o
