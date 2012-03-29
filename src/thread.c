#include "thread.h"
#include <glib.h>
#include <stdlib.h>
#include <ucontext.h>

//to remove
#include<stdio.h>

GList * ready_list=NULL;
GList * zombie_list=NULL;

struct thread {
    ucontext_t uc;
    GList * sleeping_list;
    void *retval;
};


thread_t thread_self(void) {
    return (thread_t)g_list_nth_data(ready_list, 0);
}

int thread_create(thread_t *newthread, void *(*func)(void *), void *funcarg) {
    if (ready_list == NULL){
    	thread_t main_thread;
    	main_thread=malloc(sizeof(struct thread));
    	//getcontext(&main_thread->uc);
    	ready_list = g_list_append(ready_list, main_thread);
    }

    *newthread = malloc(sizeof(struct thread));
    (*newthread)->sleeping_list = NULL;

    if(getcontext(&((*newthread)->uc)) == -1)
	return -1;
    (*newthread)->uc.uc_stack.ss_size = 64*1024;
    (*newthread)->uc.uc_stack.ss_sp = malloc((*newthread)->uc.uc_stack.ss_size);
    (*newthread)->uc.uc_link = NULL;

    makecontext(&(*newthread)->uc, (void (*)(void)) func, 1, funcarg);
    //return -1;
    ready_list = g_list_append(ready_list, *newthread);


    return 0;
}


int thread_yield(void) {
    thread_t next, current = g_list_nth_data(ready_list, 0);

    ready_list = g_list_remove(ready_list, current);
    ready_list = g_list_append(ready_list, current);

    next = g_list_nth_data(ready_list, 0);
    return swapcontext(&current->uc, &next->uc);
}

int thread_join(thread_t thread, void **retval) {

    int found = 0;
    unsigned int i;
    for(i = 0; i < g_list_length(ready_list); i++) {
	thread_t t = g_list_nth_data(ready_list, i);
	if(thread == t) 
	    found = 1;
	else {
	    if(g_list_find(t->sleeping_list, thread) != NULL)
		found = 1;
	}
    }

    if (found){

	thread_t next, current = g_list_nth_data(ready_list, 0);

	ready_list = g_list_remove(ready_list, current);

	thread->sleeping_list = g_list_append(thread->sleeping_list, current);

	next = g_list_nth_data(ready_list, 0);

	if(swapcontext(&current->uc, &next->uc) == -1)
	    return -1;
	
	*retval = current->retval;

	if (g_list_index(zombie_list, thread) != -1){
	    zombie_list = g_list_remove(zombie_list,thread);
	    free(thread->uc.uc_stack.ss_sp);
	    /* free(thread->retval); */
	    free(thread);

	}
    }
    else if (g_list_index(zombie_list,thread)!=-1){

	    thread_t waiter = g_list_nth_data(zombie_list,(g_list_index(zombie_list,
									thread)));
	    *retval = waiter->retval;
	    zombie_list = g_list_remove(zombie_list,thread);
	    free(thread->uc.uc_stack.ss_sp);
	    /* free(thread->retval); */
	    free(thread);
	}
    else {
	*retval = NULL;
	fprintf(stderr, "le thread %p n'existe pas\n", thread);
	return -1;
    }
    return 0;
}


static void wakeup_func(thread_t data, gpointer user_data) {
    if(data != NULL) {
	data->retval = user_data;
	ready_list = g_list_append(ready_list, data);
    }
}


void thread_exit(void *retval) {

    thread_t head = g_list_nth_data(ready_list, 0);

    g_list_foreach(head->sleeping_list,
		   (GFunc)wakeup_func,
		   retval);

    g_list_free(head->sleeping_list);


    head->retval=retval;
    zombie_list = g_list_append(zombie_list, head);
    ready_list = g_list_remove(ready_list, head);

    setcontext(&(((thread_t)g_list_nth_data(ready_list, 0))->uc));

    exit(0);
}
