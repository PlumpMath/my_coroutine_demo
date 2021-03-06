
#include "coroutine.h"

struct coroutine *create_co( cfunc func, ucontext_t *uctx_main) {
	struct coroutine *co = (struct coroutine *)malloc(sizeof(struct coroutine));
	if( !co)
		return NULL;
	co->uctx.uc_stack.ss_sp = co->stack_;
	co->uctx.uc_stack.ss_size = STACK_SIZE;
	co->uctx.uc_link = uctx_main;
	makecontext(&co->uctx, (void (*)(void))func, 0);

	return co;
}

int free_co( struct coroutine *co) {
	if(co && co->stack_)
		free(co->stack_);
	free(co);
	return 1;
}

