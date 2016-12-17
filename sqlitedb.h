#ifndef __CE_SQLITE_H_H
#define __CE_SQLITE_H_H

#define DBPATH "./gateway.db"

#define BOX_VERSION      	1
#define PROTOCOL_VERSION	5

struct sqlitedb;

struct sqlitedb * sqlitedb_create(char * filepath);
//void sqlitedb_exec(struct sqlitedb *, char *sql);
//void sqlitedb_close_table(struct sqlitedb * sdb);
//void sqlitedb_insert_record(struct sqlitedb *,char * table,int DID,int  CID,char * name,char *end);
//char **sqlitedb_search_all(struct sqlitedb *,char * table);
//char **sqlitedb_search_id(struct sqlitedb *,char * table,int id);
//void sqlitedb_delete_id(struct sqlitedb *,char * table,int id);
//void sqlitedb_delete_all(struct sqlitedb *,char * table);
//void updata_data(sqlite3 * db);
//void sqlitedb_updata_data(struct sqlitedb *,int terminalID,char *terminalname);

void sqlitedb_table_build(char * filepath);
int sqlitedb_add_gateway(unsigned long long ieee, char * name);
int sqlitedb_load_gateway_name(char * filepath, unsigned long long mac);
void sqlitedb_load_device();
int sqlitedb_insert_device_ieee(unsigned long long ieee, unsigned short shortaddr);
struct device;
int sqlitedb_update_device_endpoint(struct device * d);
int sqlitedb_update_device_attr(struct device * d);
int sqlitedb_update_device_status(struct device * d);
int sqlitedb_update_device_online(struct device * d);
int sqlitedb_update_device_endpoint_zonetype(struct device * d, unsigned char endpoint, unsigned short zonetype);

struct protocol_cmdtype_arm;
int sqlitedb_update_device_arm(unsigned long long ieee, unsigned char endpoint, struct protocol_cmdtype_arm * arm);
int sqlitedb_update_device_shortaddr(unsigned long long ieee, unsigned short shortaddr);
int sqlitedb_update_gatewayname(unsigned long long mac, char * name);
int sqlitedb_update_devicename(unsigned long long ieee, char * name);
void sqlitedb_delete_device(unsigned long long ieee);
int sqlitedb_update_device_seq(unsigned long long ieee, unsigned char endpoint, char seq);
int sqlitedb_update_device_state(unsigned long long ieee, unsigned char endpoint, char state);

#endif
