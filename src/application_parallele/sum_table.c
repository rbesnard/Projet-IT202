#include <stdlib.h>
#include "../thread.h"
#include <assert.h>
#include <stdio.h>


struct tableau{
    int i;
    int j;
    int *ptr;
};

int
sum(struct tableau *T) {

    thread_t thread1, thread2;
    int retval1, retval2;
    int err;
    if(T->i!=T->j){
	struct tableau T1,T2;
	T1.ptr=T2.ptr=T->ptr;
	T1.i=T->i;
	T1.j=(T->i+T->j)/2;
	T2.i=(T->i+T->j)/2+1;
	T2.j=T->j;
	err = thread_create(&thread1,(void *(*)(void *)) sum, (void *)&T1);
	assert(!err);
	err = thread_create(&thread2,(void *(*)(void *)) sum, (void *)&T2);
	assert(!err);

	err = thread_join(thread2, (void **)&retval2);
	assert(!err);
	err = thread_join(thread1, (void **)&retval1);
	assert(!err);

	return retval1+retval2;
    }
    else
	return T->ptr[T->i];
}


int
main(int argc, char **argv) {
    (void)argc;
    int i,size=atoi(argv[1]);
    int T[size];

    printf("T={ ");
    for(i=0;i<size;i++) {
	T[i]=1;
	printf("%d ",T[i]);
    }
    printf("}\n");

    struct tableau tab;
    tab.ptr=T;
    tab.i=0;
    tab.i=size-1;

    printf("sum=%d\n",sum(&tab));

    return EXIT_SUCCESS;
}
