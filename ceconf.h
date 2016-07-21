
#ifndef __CECONFIG_H_H
#define __CECONFIG_H_H

char * ceconf_getlistenport();
char * ceconf_getserveraddr();
char * ceconf_getserverport();
char * ceconf_getserialport();

int ceconf_gettimeout();
void ceconf_load();

#endif
