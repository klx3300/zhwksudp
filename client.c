#include "prot.h"
#include <stdio.h>

char some_garbage[1000];

int main(void){
    sudp_init();
    qSocket s = sudp_open();
    sudp_connect(s,"127.0.0.1:13789");
    int cnt = 0;
    while(1){
        sudp_send(s,some_garbage,1000);
        //printf("pack %d sent\n",cnt);
        cnt++;
    }
    return 0;
}