/*
 * A simple kernel FIFO implementation.
 *
 * Copyright (C) 2004 Stelian Pop <stelian@popies.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */
#ifndef _LINUX_KFIFO_H
#define _LINUX_KFIFO_H

struct kfifo {
	unsigned char *buffer;	/* the buffer holding the data */
	unsigned int size;	/* the size of the allocated buffer */
	unsigned int in;	/* data is added at offset (in % size) */
	unsigned int out;	/* data is extracted from off. (out % size) */
};

extern struct kfifo *kfifo_init( unsigned int size);
				
extern void kfifo_free(struct kfifo *fifo);
extern unsigned int kfifo_put(struct kfifo *fifo,
				unsigned char *buffer, unsigned int len);
extern unsigned int kfifo_get(struct kfifo *fifo,
				unsigned char *buffer, unsigned int len);

extern unsigned int kfifo_getahead(struct kfifo *fifo,
			 unsigned char *buffer, unsigned int len);

unsigned int kfifo_pop(struct kfifo * fifo);

/**
 * __kfifo_reset - removes the entire FIFO contents, no locking version
 * @fifo: the fifo to be emptied.
 */
static inline void kfifo_reset(struct kfifo *fifo)
{
	fifo->in = fifo->out = 0;
}

/**
 * __kfifo_len - returns the number of bytes available in the FIFO, no locking version
 * @fifo: the fifo to be used.
 */
static inline unsigned int kfifo_len(struct kfifo *fifo)
{
	return fifo->in - fifo->out;
}


#endif

