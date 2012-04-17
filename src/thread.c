#include "thread.h"
#include <glib.h>
#include <stdlib.h>
#include <ucontext.h>
#include <valgrind/valgrind.h>
//to remove
#include<stdio.h>

typedef void(*treat_func)(int);

//initial treatment signal function
void basic_sig_treatment(int sig){
    if(sig<0 || sig>NB_SIG)
	return;
    
    switch(sig){
    case SIG_KILL :
	thread_exit(NULL);
	break;
    case SIG_STOP :
	thread_yield();
	break;
    default:
	printf("signal %d recu\n", sig);
    }
}

GList * ready_list=NULL;
GList * zombie_list=NULL;

struct thread {
    ucontext_t uc;
    GList * sleeping_list;
    void *retval;

    treat_func treat_tab[NB_SIG];
    GList* sig_list;

    /*valgrind stackid*/
    int stackid;
};


thread_t thread_self(void) {
    return (thread_t)g_list_nth_data(ready_list, 0);
}

int thread_create(thread_t *newthread, void *(*func)(void *), void *funcarg) {
    if (ready_list == NULL){
    	thread_t main_thread;
    	main_thread=malloc(sizeof(struct thread));
	main_thread->sleeping_list=NULL;
    	//getcontext(&main_thread->uc);
    	ready_list = g_list_append(ready_list, main_thread);
	
	thread_initSigTab(main_thread);
    }

    *newthread = malloc(sizeof(struct thread));
    (*newthread)->sleeping_list = NULL;

    if(getcontext(&((*newthread)->uc)) == -1)
	return -1;
    (*newthread)->uc.uc_stack.ss_size = 64*1024;
    (*newthread)->uc.uc_stack.ss_sp = malloc((*newthread)->uc.uc_stack.ss_size);

    /* juste après l'allocation de la pile */
    (*newthread)->stackid =
	VALGRIND_STACK_REGISTER((*newthread)->uc.uc_stack.ss_sp,
				(*newthread)->uc.uc_stack.ss_sp +
				(*newthread)->uc.uc_stack.ss_size);

    (*newthread)->uc.uc_link = NULL;

    makecontext(&(*newthread)->uc, (void (*)(void)) func, 1, funcarg);
    //return -1;
    ready_list = g_list_append(ready_list, *newthread);
    
    thread_initSigTab(*newthread);

    return 0;
}


int thread_yield(void) {
    thread_t next, current = g_list_nth_data(ready_list, 0);
    int ok;
    ready_list = g_list_remove(ready_list, current);
    ready_list = g_list_append(ready_list, current);

    next = g_list_nth_data(ready_list, 0);
    ok=swapcontext(&current->uc, &next->uc);
    
    if(!ok)
      thread_sigTreat(current);
    
    return ok;
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
	
	thread_sigTreat(current);

	if (g_list_index(zombie_list, thread) != -1){
	    zombie_list = g_list_remove(zombie_list,thread);
	    free(thread->uc.uc_stack.ss_sp);
	    /* juste avant de libérer la pile */
	    VALGRIND_STACK_DEREGISTER(thread->stackid);

	    free(thread);

	}

	thread_t cur_t =  g_list_nth_data(ready_list, 0);
	if(g_list_length(ready_list)==1 && g_list_length(cur_t->sleeping_list)==0){
	    /* fprintf(stderr, "Total Annihilation\n"); */

	    g_list_free(cur_t->sleeping_list);

	    free(cur_t);

	    g_list_free(ready_list);
	    ready_list=NULL;
	}
    }
    else if (g_list_index(zombie_list,thread)!=-1){

	thread_t waiter = g_list_nth_data(zombie_list,(g_list_index(zombie_list,
								    thread)));
	*retval = waiter->retval;
	zombie_list = g_list_remove(zombie_list,thread);
	free(thread->uc.uc_stack.ss_sp);
	/* juste avant de libérer la pile */
	VALGRIND_STACK_DEREGISTER(thread->stackid);
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

void thread_kill(thread_t thr, int sig){
    int* new_sig;
    if(thr==NULL)
	return;
  
    new_sig=malloc(sizeof(int));
    *new_sig=sig;
    thr->sig_list=g_list_append(thr->sig_list, new_sig);  
}

void thread_signal(thread_t thr, int sig, void (*sig_func)(int)){
    if(thr==NULL)
	return;

    if(sig<0 || sig >= NB_SIG)
	return;

    thr->treat_tab[sig]=sig_func;
}

void thread_initSigTab(thread_t thr){
    int i;
  
    for(i=0; i<NB_SIG; i++)
	thr->treat_tab[i]=basic_sig_treatment;

    thr->sig_list = NULL;
}

void thread_sigTreat(thread_t thr){
    #if 0
    while(g_list_length(thr->sig_list)>0){
	int* sig = g_list_nth_data(thr->sig_list, 0);
    
	if(*sig>=0 && *sig<NB_SIG){
	    (*(thr->treat_tab[*sig])) (*sig);
	}
 
	thr->sig_list=g_list_remove(thr->sig_list, sig);
	free(sig);
    }
    #endif
}
