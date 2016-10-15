#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <ucontext.h>

#include "coroutine.h"

#define handle_error(msg) \
	do { perror(msg); exit(EXIT_FAILURE); } while (0)

struct msgbuf {
	char buf[STACK_SIZE ];
	int len;
	int pos;
	int last;
};

static ucontext_t uctx_main;

struct coroutine *producer, *consumer;
struct msgbuf *buffer;

void produce() {
	while(1) {
		printf("switch to producer\n");
		if(buffer->last > 0) { // buffer not empty
			printf("buffer->last > 0, produce but buffer not empty\n");
			swapcontext(&producer->uctx, &consumer->uctx);
		}
		int n = read(0, buffer + buffer->last, buffer->len - buffer->last);
		if(n < 0) {
			// error
		} else if (n == 0) { //ctrl + D
			// switch to uctx_main, then end the process
		} else {
			// switch to consumer
			buffer->last += n;
			printf("producer %d bytes\n", n);
			swapcontext(&producer->uctx, &consumer->uctx);
		}
	}
}

void consume() {
	while(1) {
		printf("switch to consumer\n");
		if(buffer->last - buffer->pos > 0) { // not empty
			int n = write(1, buffer + buffer->pos, buffer->last - buffer->pos);
			if(n <= 0) {
				printf("consumer error %s\n", strerror(errno));
				// error and switch uctx_main, then end the process
			} else {
				if (n == buffer->last - buffer->pos) {
					buffer->pos = buffer->last = 0;
					swapcontext(&consumer->uctx, &producer->uctx);
				} else {
					buffer->pos += n;
				}
			}
		} else {	// empty
			printf("consume but empty\n");
			swapcontext(&consumer->uctx, &producer->uctx);
		}
	}
}

int main()
{
//	producer = create_co(produce, uctx_main);
//	consumer = create_co(consume, uctx_main);
//
//	if( !consumer || !producer)
//		handle_error("create producer or consumer error");

	producer = (struct coroutine *)malloc(sizeof(struct coroutine));
	if( !producer)
		handle_error("create producer error");
	consumer = (struct coroutine *)malloc(sizeof(struct coroutine));
	if( !consumer)
		handle_error("create consumer error");

	buffer = (struct msgbuf *)malloc(sizeof(struct msgbuf));
	if( !buffer)
		handle_error("create msgbuf error");
	buffer->pos = 0;
	buffer->len = sizeof(buffer->buf);
	buffer->last = 0;


	getcontext(&producer->uctx);
	producer->uctx.uc_stack.ss_sp = producer->stack_;
	producer->uctx.uc_stack.ss_size = STACK_SIZE;
	producer->uctx.uc_link = &consumer->uctx;
//	producer->uctx.uc_link = &uctx_main;

	getcontext(&consumer->uctx);
	consumer->uctx.uc_stack.ss_sp = consumer->stack_;
	consumer->uctx.uc_stack.ss_size = STACK_SIZE;
	consumer->uctx.uc_link = &producer->uctx;
//	consumer->uctx.uc_link = &uctx_main;

	makecontext(&(producer->uctx), produce, 0);
	makecontext(&(consumer->uctx), consume, 0);

	swapcontext(&uctx_main, &(producer->uctx));
}

