#include "../thread.h"
#include <assert.h>
#include <stdio.h>

int i = 0;

int fibonacci(int n){
    if (n == 1 || n == 2){
	thread_exit(1);
	return 1;
    }

    else{
	thread_t thread1, thread2;
	void * retval1, *retval2;
	int err;
	int a = n-1;
	int b = n-2;
	err = thread_create(&thread1, (void * (*) (void *))fibonacci, (void *) a);
	assert(!err);
	i++;
	err = thread_create(&thread2, (void * (*) (void *))fibonacci, (void *) b);
	assert(!err);
	err = thread_join(thread2, &retval2);
	assert(!err);
	err = thread_join(thread1, &retval1);
	assert(!err);
	int c = (int) retval1;
	int d = (int) retval2;
	thread_exit(c+d);
	return c+d;
    }
}

int main(int argc, char **argv) {
    thread_t threadfibo;
    void * retval;
    int err;
    int n = atoi(argv[1]);
    err = thread_create(&threadfibo, (void * (*) (void *))fibonacci,(void*) n);
    assert(!err);
    err = thread_join(threadfibo, &retval);
    printf("fibonacci(%d)=%d\n",n,(int)retval);
    return 0;
}

