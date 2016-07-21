#ifndef __CETIMER_H_H_
#define __CETIMER_H_H_

struct cetimer; 
struct cetimer * cetimer_create(unsigned int nextvalue, unsigned int interval, int wfd, int reconnwfd);
void cetimer_start(struct cetimer * timer);


#endif
