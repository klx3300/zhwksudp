#include "prot.h"
#include "zhwkre/concurrent.h"
#include "zhwkre/list.h"
#include <string.h>
#include <unistd.h>

typedef unsigned int ui;

qListDescriptor succ_sent;
qMutex sendmu;
int sendseri_bottom;
int sendseri_top;
qListDescriptor succ_recv;
qMutex recvmu;
int recvseri_bottom;
int recvseri_top;
char connectedaddr[50];
int checkno;
ui timerv;
ui timer;
ui connect_stat;

#define PTR(x) ((void*)&(x))
#define ZEROINIT(x) memset(&(x),0,sizeof(x))
#define ZEROPINIT(x) memset(x,0,sizeof(*(x)))
#define ZEROPSINIT(x,s) memset(x,0,s)

ui crc(void* t,ui size){
    ui sum=0;
    for(ui i=0;i<size;i++){
        sum*=97;
        sum+=*(unsigned char*)(t+i);
    }
    return sum;
}

void* sudp_timer(void* fakeargs){
    while(timer){
        usleep(1000);
        timerv++;
    }
    return NULL;
}

void* sudp_receiver(void* q_sock){
    qSocket sock = *(qSocket*)q_sock;
    char buffer[1200];
    int recverrn = 0;
    while(connect_stat){
        recverrn = qDatagramSocket_receive(sock,NULL,buffer,1200,0);
        if(recverrn <= 0) continue;
        switch(buffer[0]){
            case SUDP_DAT:
            {
                // check data integrity
                sudp_dat* dat = (sudp_dat*)buffer;
                if(crc(buffer+sizeof(sudp_dat),dat->size)!=dat->checksum){
                    // CRC fail.
                    // send bad
                    sudp_bad bad;
                    bad.operid = SUDP_BAD;
                    bad.serino = dat->serino;
                    qDatagramSocket_send(sock,connectedaddr,PTR(bad),sizeof(bad),0);
                }else{
                    recvmu.lock(recvmu);
                    // then it's a valid package!
                    // check if window is stretched to its maximum size
                    if(recvseri_top - recvseri_bottom >= SUDP_WINDOW_SIZE){
                        if(dat->serino > recvseri_top){
                            // reject
                            recvmu.unlock(recvmu);
                            sudp_bad bad;
                            bad.operid = SUDP_BAD;
                            bad.serino = dat->serino;
                            qDatagramSocket_send(sock,connectedaddr,PTR(bad),sizeof(bad),0);
                        }else if(dat->serino < recvseri_bottom){
                            // ignore
                            recvmu.unlock(recvmu);
                            sudp_ack ack;
                            ack.operid = SUDP_ACK;
                            ack.serino = dat->serino;
                            qDatagramSocket_send(sock,connectedaddr,PTR(ack),sizeof(ack),0);
                        }else{
                            // in between
                            // then that's good! i'll just..
                            qList_foreach(succ_recv,iter){
                                sudp_datapack_stat* dp = (sudp_datapack_stat*)iter->data;
                                if(dp->serino == dat->serino && dp->status == SUDP_PACK_TAKEPLACE){
                                    // good,so i have found that
                                    dp->size = dat->size;
                                    memcpy(dp->content,buffer+sizeof(sudp_dat),dp->size);
                                    dp->status = SUDP_PACK_ACKED;
                                    sudp_ack ack;
                                    ack.operid = SUDP_ACK;
                                    ack.serino = dat->serino;
                                    qDatagramSocket_send(sock,connectedaddr,PTR(ack),sizeof(ack),0);
                                    break;
                                }
                            }
                            recvmu.unlock(recvmu);
                        }
                    }else{
                        // then it's not full yet.
                        if(dat->serino - recvseri_bottom > SUDP_WINDOW_SIZE){
                            // if stretch like that, things will not work properly
                            // so f*ck it.
                            recvmu.unlock(recvmu);
                            sudp_bad bad;
                            bad.operid = SUDP_BAD;
                            bad.serino = dat->serino;
                            qDatagramSocket_send(sock,connectedaddr,PTR(bad),sizeof(bad),0);
                        }else if(dat->serino > recvseri_top){
                            // then this can be stretched properly.
                            recvseri_top = dat->serino;
                            sudp_datapack_stat* curr_pack = (sudp_datapack_stat*)succ_recv.tail->data;
                            int curr_seri = curr_pack->serino;
                            for(int i=curr_seri;i<=recvseri_top;i++){
                                sudp_datapack_stat tmppack;
                                ZEROINIT(tmppack);
                                tmppack.serino = i;
                                tmppack.init_time = timerv;
                                qList_push_back(succ_recv,tmppack);
                            }
                            // modify the last elem
                            curr_pack = (sudp_datapack_stat*)succ_recv.tail->data;
                            curr_pack->status = SUDP_PACK_ACKED;
                            memcpy(curr_pack->content,buffer+sizeof(sudp_dat),dat->size);
                            curr_pack->size = dat->size;
                            recvmu.unlock(recvmu);
                            sudp_ack ack;
                            ack.operid = SUDP_ACK;
                            ack.serino = dat->serino;
                            qDatagramSocket_send(sock,connectedaddr,PTR(ack),sizeof(ack),0);
                        }else if(dat->serino < recvseri_bottom){
                            // ignore
                            recvmu.unlock(recvmu);
                            sudp_ack ack;
                            ack.operid = SUDP_ACK;
                            ack.serino = dat->serino;
                            qDatagramSocket_send(sock,connectedaddr,PTR(ack),sizeof(ack),0);
                        }else{
                            // in between
                            qList_foreach(succ_recv,iter){
                                sudp_datapack_stat* dp = (sudp_datapack_stat*)iter->data;
                                if(dp->serino == dat->serino){
                                    // good,so i have found that
                                    dp->size = dat->size;
                                    memcpy(dp->content,buffer+sizeof(sudp_dat),dp->size);
                                    dp->status = SUDP_PACK_ACKED;
                                    sudp_ack ack;
                                    ack.operid = SUDP_ACK;
                                    ack.serino = dat->serino;
                                    qDatagramSocket_send(sock,connectedaddr,PTR(ack),sizeof(ack),0);
                                    break;
                                }
                            }
                            recvmu.unlock(recvmu);
                        }
                    }
                }
            }
            break;
            case SUDP_ACK:
            {
                sudp_ack* ack = (sudp_ack*)buffer;
                sendmu.lock(sendmu);
                if(ack->serino < sendseri_bottom){
                    // ignore
                }else if(ack->serino > sendseri_top){
                    // ignore
                }else{
                    qList_foreach(succ_sent,iter){
                        sudp_datapack_stat* pack = (sudp_datapack_stat*)iter->data;
                        if(pack->serino == ack->serino){
                            pack->status = SUDP_PACK_ACKED;
                            break;
                        }
                    }
                }
                sendmu.unlock(sendmu);
            }
            break;
            case SUDP_FIN:
            // ignore
            break;
            case SUDP_RST: 
            connect_stat = 0;
            break;
            case SUDP_BAD:
            {
                sudp_bad* bad = (sudp_bad*)buffer;
                sendmu.lock(sendmu);
                if(bad->serino < sendseri_bottom){
                    // ignore
                }else if(bad->serino > sendseri_top){
                    // ignore
                }else{
                    qList_foreach(succ_sent,iter){
                        sudp_datapack_stat* pack = (sudp_datapack_stat*)iter->data;
                        if(pack->serino == bad->serino){
                            pack->status = SUDP_PACK_WAITSEND;
                            break;
                        }
                    }
                }
                sendmu.unlock(sendmu);
            }
            break;
            default: 
            // ignore the pack
            break;
        }
    }
    return NULL;
}

void* sudp_sender(void* q_sock){
    qSocket sock = *(qSocket*)q_sock;
    int senderrn = 0;
    char buffer[1200];
    while(connect_stat){
        // remove acked packages to allow window shrink
        sendmu.lock(sendmu);
        qListIterator iter = succ_sent.head;
        sudp_datapack_stat* dpiter = iter->data;
        while(dpiter->status == SUDP_PACK_ACKED){
            qListIterator tmpnext = iter->next;
            qList_erase_elem(succ_sent,iter);
            iter=tmpnext;
            dpiter=iter->data;
        }
        // check all packages in the window: wait send to sent,too long reset
        qList_foreach(succ_sent,riter){
            sudp_datapack_stat* pack = riter->data;
            if(pack->status == SUDP_PACK_WAITSEND){
                // send using dat
                sudp_dat dat;
                ZEROINIT(dat);
                dat.operid = SUDP_DAT;
                dat.serino = pack->serino;
                dat.size = pack->size;
                dat.checksum = crc(pack->content,pack->size);
                memcpy(buffer,&dat,sizeof(dat));
                memcpy(buffer+sizeof(dat),pack->content,pack->size);
                qDatagramSocket_send(sock,connectedaddr,buffer,pack->size+sizeof(dat),0);
                pack->init_time = timerv;
            }else if(pack->status == SUDP_PACK_SENT){
                // check timeout
                if(timerv - pack->init_time > SUDP_RESEND_INTERVAL){
                    // timed out
                    pack->status = SUDP_PACK_WAITSEND;
                }
            }
        }
        // send work completed.
        sendmu.unlock(sendmu);
        usleep(SUDP_SENDCHECK_INTERVAL); 
    }
    return NULL;
}

void sudp_init(){
    qList_initdesc(succ_sent);
    qList_initdesc(succ_recv);
    qMutex_constructor(sendmu);
    qMutex_constructor(recvmu);
    timer=1;
    qRun(sudp_timer,NULL);
    // int values are automatically set to 0 -- glob vars
}

qSocket sudp_open(){
    qSocket sock = qSocket_constructor(qIPv4,qDefaultProto,qDatagramSocket);
    qSocket_open(sock);
    return sock;
}

int sudp_bind(qSocket sock,const char* addrxport){
    return qSocket_bind(sock,addrxport);
}

int sudp_accept(qSocket sock){
    char buffer[50];
    int recverrn = 0;
    over_again_accept:
    recverrn=qDatagramSocket_receive(sock,connectedaddr,buffer,50,0);
    if(recverrn <= 0){
        return -1;
    }
    if(buffer[0]!=SUDP_SYN){
        // send rst
        buffer[0]=SUDP_RST;
        qDatagramSocket_send(sock,connectedaddr,buffer,1,0);
        // im not gonna wait that!
        goto over_again_accept;
    }else{
        // then that's correct!
        sudp_syn* syn = (sudp_syn*)buffer;
        sudp_ack ack;
        ack.operid = SUDP_ACK;
        ack.serino = syn->checkno+1;
        checkno = ack.serino;
        qDatagramSocket_send(sock,connectedaddr,PTR(ack),sizeof(ack),0);
    }
    // wait for next ack
    recverrn = qDatagramSocket_receive(sock,connectedaddr,buffer,50,0);
    if(buffer[0]!=SUDP_ACK){
        buffer[0]=SUDP_RST;
        qDatagramSocket_send(sock,connectedaddr,buffer,1,0);
        goto over_again_accept;
    }else{
        sudp_ack* ack=(sudp_ack*)buffer;
        if(ack->serino != checkno+1){
            buffer[0]=SUDP_RST;
            qDatagramSocket_send(sock,connectedaddr,buffer,1,0);
            goto over_again_accept;
        }
        // ack success. connection established.
        // start receiver and sender
        qRun(sudp_receiver,&sock);
        qRun(sudp_sender,&sock);
    }
    return 0;
}

int sudp_connect(qSocket sock,const char* addrxport){
    char buffer[50];
    int recverrn = 0;
    // first send syn
    sudp_syn syn;
    over_again_connect:
    ZEROINIT(syn);
    syn.operid = SUDP_SYN;
    syn.checkno = 233;
    if(qDatagramSocket_send(sock,addrxport,PTR(syn),sizeof(syn),0)!=sizeof(syn)){
        return -1;
    }
    // wait ack
    recverrn = qDatagramSocket_receive(sock,connectedaddr,buffer,50,0);
    if(recverrn <= 0){
        return -1;
    }
    if(buffer[0]!=SUDP_ACK){
        buffer[0]=SUDP_RST;
        qDatagramSocket_send(sock,connectedaddr,buffer,1,0);
        return -1;
    }else{
        sudp_ack *ack = (sudp_ack*)buffer;
        if(ack->serino != 234){
            buffer[0]=SUDP_RST;
            qDatagramSocket_send(sock,connectedaddr,buffer,1,0);
            goto over_again_connect;
        }else{
            // send ack 235
            sudp_ack ack;
            ZEROINIT(ack);
            ack.operid = SUDP_ACK;
            ack.serino = 235;
            qDatagramSocket_send(sock,connectedaddr,PTR(ack),sizeof(ack),0);
        }
    }
    // successive
    qRun(sudp_receiver,&sock);
    qRun(sudp_sender,&sock);
    return 0;
}

int sudp_send(qSocket sock,const char* payload,int length){
    while(sendseri_top - sendseri_bottom >= SUDP_WINDOW_SIZE && connect_stat);
    if(!connect_stat) return 0; // connection close already!
    sendmu.lock(sendmu);
    int curr_pos = 0;
    while(curr_pos != length){
        sendseri_top++;
        sudp_datapack_stat pack;
        ZEROINIT(pack);
        pack.serino = sendseri_top;
        pack.status = SUDP_PACK_WAITSEND;
        if(length - curr_pos <= 1000){
            pack.size = length - curr_pos;
            memcpy(pack.content,payload+curr_pos,pack.size);
            curr_pos=length;
        }else{
            pack.size = 1000;
            memcpy(pack.content,payload+curr_pos,1000);
            curr_pos+=1000;
        }
        qList_push_back(succ_sent,pack);
    }
    sendmu.unlock(sendmu);
    return length;
}

int sudp_recv(qSocket sock,char* buffer,int limit){
    while(1){
        sudp_datapack_stat *tstpack = succ_recv.head->data;
        while(tstpack != NULL && tstpack->status != SUDP_PACK_ACKED && connect_stat);
        if(tstpack != NULL) break;
    }
    if(!connect_stat) return 0;
    recvmu.lock(recvmu);
    // check recvlist
    int received=0;
    qList_foreach(succ_recv,iter){
        sudp_datapack_stat* pack = iter->data;
        if(pack->status != SUDP_PACK_ACKED){
            // Reached limit
            break;
        }
        if(limit-received < pack->size){
            memcpy(buffer+received,pack->content,limit-received);
            char movebuffer[1000];
            memcpy(movebuffer,pack->content,pack->size-(limit-received));
            memcpy(pack->content,movebuffer,pack->size-(limit-received));
            pack->size=pack->size-(limit-received);
            received = limit;
            break;
        }else{
            memcpy(buffer+received,pack->content,pack->size);
            received += pack->size;
            pack->status = SUDP_PACK_USED;
            if(received == limit){
                break;
            }
        }
    }
    // iteration already through all stuff
    // clear used
    qListIterator iter = succ_recv.head;
    sudp_datapack_stat* dpiter = iter->data;
    while(dpiter->status == SUDP_PACK_USED){
        qListIterator tmpnext = iter->next;
        qList_erase_elem(succ_recv,iter);
        iter=tmpnext;
        dpiter=iter->data;
    }
    // unlock
    recvmu.unlock(recvmu);
    return received;
}