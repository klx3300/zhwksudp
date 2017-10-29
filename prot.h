#ifndef Q_ZHWK_SUDP_PROT_H
#define Q_ZHWK_SUDP_PROT_H

#include "zhwkre/network.h"

#define SUDP_PACK_TRUNC 1000
#define SUDP_WINDOW_SIZE 10
#define SUDP_RESEND_INTERVAL 5000 // unit:ms
#define SUDP_SENDCHECK_INTERVAL 10000 // unit:us

void sudp_init();
qSocket sudp_open();
int sudp_bind(qSocket sock,const char* addrxport);
int sudp_accept(qSocket sock);
int sudp_connect(qSocket sock,const char* addrxport);
int sudp_send(qSocket sock,const char* payload,int length);
int sudp_recv(qSocket sock,char* buffer,int limit);

#define SUDP_SYN 0
typedef struct sudp_syn_st{
    char operid;
    int checkno;
}sudp_syn;

#define SUDP_ACK 1
typedef struct sudp_ack_st{
    char operid;
    int serino;
}sudp_ack;

#define SUDP_DAT 2
typedef struct sudp_dat_st{
    char operid;
    int serino;
    int size;
    unsigned int checksum;
}sudp_dat;

#define SUDP_FIN 3
typedef struct sudp_fin_st{
    char operid;
    int checkno;
}sudp_fin;
// note: FIN is literally no use: I'm not going to implement
// full-two-way connection. so using RST is just enough.

#define SUDP_RST 4
typedef struct sudp_rst_st{
    char operid;
}sudp_rst;

#define SUDP_BAD 5
typedef struct sudp_bad_st{
    char operid;
    int serino;
}sudp_bad;

#define SUDP_PACK_TAKEPLACE 0
#define SUDP_PACK_WAITSEND 1
#define SUDP_PACK_SENT 2
#define SUDP_PACK_ACKED 3
#define SUDP_PACK_USED 4
typedef struct sudp_datapack_stat_st{
    int serino;
    int status;
    int size;
    unsigned int init_time;
    char content[SUDP_PACK_TRUNC];
}sudp_datapack_stat;

#endif