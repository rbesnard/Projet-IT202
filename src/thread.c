#include "thread.h"
//#include <glib.h>
#include <stdlib.h>
#include <ucontext.h>


/*
 *TODO Q -> liste de thread
 *          Ready
 *          Zombie
 */

struct thread {
    ucontext_t uc;
    /*glib list sleeping*/
    void *retval;
};


thread_t thread_self(void) {
    //return ready list head


}

int thread_create(thread_t *newthread, void *(*func)(void *), void *funcarg) {
    if(getcontext(&newthread->uc) == -1)
	return -1;
    newthread->uc.uc_stack.ss_size = 64*1024;
    newthread->uc.uc_stack.ss_sp = malloc(uc.uc_stack.ss_size);
    uc.uc_link = NULL;
    
    if(makecontext(newthread->uc, (void (*)(void)) func, 1, funcarg) == -1)
	return -1;
    //list_add(ready_list, newthread);
    
    return 0;
}


int thread_yield(void) {
    
    thread_t next, current = list_head(ready_list);
    //remove_head(ready_list);
    //add_tail(ready_list, t);
    next = list_head(ready_list);
    return swapcontext(&current->uc, &next->uc);
    
}

int thread_join(thread_t thread, void **retval) {
    thread_t next, current = list_head(ready_list);
    //remove_head(ready_list);
    //add_tail(thread->sleeping_list, t);
    next = list_head(ready_list);
    
    if(swapcontext(&current->uc, &next->uc) == -1)
	return -1;

    retval=&current->retval;
    return 0;
}

void thread_exit(void *retval) {    
    thread_t t;

    //pour chaque thread t dans sleeping list de la tete
    t->retval=retval;
    //placer thread dans ready_list
    
    //placer tete ready dans zombie

    setcontext
}
