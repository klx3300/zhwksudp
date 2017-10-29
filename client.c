#include "prot.h"
#include <stdio.h>

int main(void){
    sudp_init();
    qSocket s = sudp_open();
    sudp_connect(s,"127.0.0.1:13789");
    sudp_send(s,"FUCK!",5);
    return 0;
}