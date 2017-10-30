#include "zhwkre/concurrent.h"
#include <unistd.h>
#include <stdio.h>

int i;
qMutex mu;

void* adder(void* a){
    for(int x=0;x<100000;x++){
        mu.lock(mu);
        i++;
        mu.unlock(mu);
    }
    return NULL;
}

int main(void){
    mu=qMutex_constructor();
    for(int k=0;k<4;k++)
        qRun(adder,NULL);
    sleep(5);
    printf("%d\n",i);
}