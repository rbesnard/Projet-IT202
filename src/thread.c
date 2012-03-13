#include "thread.h"
#include <glib.h>
#include <stdlib.h>
#include <ucontext.h>


/*
 *TODO Q -> liste de thread
 *          Ready
 *          Zombie
 */

struct GList * ready_list=NULL;
struct GList * zombie_list=NULL;

struct thread {
    ucontext_t uc;
    struct GList * sleeping_list;
    void *retval;
};


thread_t thread_self(void) {
    return (thread_t)g_list_nth_data(ready_list, 0);
}

int thread_create(thread_t *newthread, void *(*func)(void *), void *funcarg) {
    if (ready_list == NULL)
	ready_list = g_list_alloc();
    if (zombie_list == NULL)
	zombie_list = g_list_alloc();

    if(getcontext(&newthread->uc) == -1)
	return -1;
    newthread->uc.uc_stack.ss_size = 64*1024;
    newthread->uc.uc_stack.ss_sp = malloc(uc.uc_stack.ss_size);
    uc.uc_link = NULL;

    if(makecontext(newthread->uc, (void (*)(void)) func, 1, funcarg) == -1)
	return -1;
    ready_list = g_list_append(ready_list, newthread);

    return 0;
}


int thread_yield(void) {
    thread_t next, current = g_list_nth_data(ready_list, 0);

    ready_list = g_list_remove(ready_list, current);
    ready_list = g_list_append(ready_list, current);

    next = list_head(ready_list);
    return swapcontext(&current->uc, &next->uc);
}

int thread_join(thread_t thread, void **retval) {
    thread_t next, current = g_list_nth_data(ready_list, 0);

    ready_list = g_list_remove(ready_list, current);
    ready_list = g_list_append(thread->sleeping_list, current);

    next = list_head(ready_list);

    if(swapcontext(&current->uc, &next->uc) == -1)
	return -1;

    retval=&current->retval;
    return 0;
}


void wakeup_func(gpointer data, gpointer user_data) {
    (thread_t)data->retval = user_data;
    g_list_append(ready_list, data);
}


void thread_exit(void *retval) {

    thread_t head = g_list_nth_data(ready_list, 0);

    g_list_foreach(head,
		   wakeup_func,
		   retval);

    g_list_free(head->sleeping);

    zombie_list = g_list_append(zombie_list, head);
    ready_list = g_list_remove(ready_list, head);

    setcontext(g_list_nth_data(ready_list, 0)->uc);
}
