#include "thread.h"
#include <stdlib.h>
#include <ucontext.h>

thread_t thread_self(void) {



}

int thread_create(thread_t *newthread, void *(*func)(void *), void *funcarg) {

}


int thread_yield(void) {


}

int thread_join(thread_t thread, void **retval) {

}

void thread_exit(void *retval) __attribute__ ((__noreturn__)) {

}
