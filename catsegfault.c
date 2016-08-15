#include <stdio.h>
#include <stdlib.h>
#include <execinfo.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

static struct sigaction sa_old;

void stack_backtrace(void)
{
#define BUFSIZE 100
	void *buf[BUFSIZE] = {0};
	char **strings;
	int len, i;
	
	printf("segment fault\n");
	len = backtrace(buf, BUFSIZE);
	printf("backtrace: len is %d\n", len);

	strings = backtrace_symbols(buf, len);
	for(i = 0; i < len; i ++)
		printf("%s\n", strings[i]);
	
	free(strings);
}

static void handle_segfault(int signum)
{
	stack_backtrace();
	sigaction(SIGSEGV, &sa_old, NULL);
	exit(1);
}

void sigaction_init(void)
{
	struct sigaction sa_segv;
	sa_segv.sa_flags = 0;
	sa_segv.sa_handler = handle_segfault;

	sigaction(SIGSEGV, &sa_segv, &sa_old);
}


