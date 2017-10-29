#include "prot.h"
#include <stdio.h>

int main(void){
    sudp_init();
    qSocket s = sudp_open();
    sudp_bind(s,"127.0.0.1:13789");
    sudp_accept(s);
    int k=0;
    char buffer[100];
    do{
        k=sudp_recv(s,buffer,100);
        buffer[k]='\0';
        printf("%s\n",buffer);
    }while(k>0);
    return 0;
}