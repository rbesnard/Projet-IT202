#include<stdlib.h>
#include<stdio.h>

#include"../thread.h"

int out=1;

void go_out(int sig){
  out=0;
  fprintf(stderr, "val changed\n");
  return;
}

int func(int val){
  
  fprintf(stderr, "thread launched\n");
  thread_signal(thread_self(), SIG_USR1, go_out); 

  fprintf(stderr, "signal attributed\n");

  while(out){
    thread_yield();
  }

  fprintf(stderr, "out of the first loop\n");
   
  while(7);
  
  thread_exit(val);
}



int main(int argc, char **argv){
    thread_t threadsignal;
    void * retval;
    int err;
 
    err = thread_create(&threadsignal, (void * (*) (void *))func ,(void*) 2);
    fprintf(stderr, "thread create\n");
    thread_yield();
    fprintf(stderr, "thread attendu\n");
    thread_kill(threadsignal, SIG_USR2);
    thread_kill(threadsignal, SIG_USR1);
    thread_kill(threadsignal, SIG_KILL);
    fprintf(stderr, "signal envoye\n");
    err = thread_join(threadsignal, &retval);
    fprintf(stderr, "thread fini\n");
    return 0;
}
