#include "prot.h"
#include <stdio.h>
#include <errno.h>


int main(void){
    sudp_init();
    qSocket s = sudp_open();
    int binderrno = sudp_bind(s,"127.0.0.1:13789");
    printf("bind: %d\n",binderrno);
    int accepterrno = sudp_accept(s);
    printf("accept %d\n",accepterrno);
    int k=0;
    char buffer[1000];
    int recvpacks=0;
    do{
        k=sudp_recv(s,buffer,1000);
        //printf("pack %d received.\n",recvpacks);
        /*for(int i=0;i<1000;i++){
            if(buffer[i]!=0) printf("integrity failed at pack %d\n",recvpacks);
        }*/
        recvpacks++;
    }while(k>0);
    return 0;
}