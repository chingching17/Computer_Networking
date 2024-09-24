#ifndef HEADERS_H
#define HEADERS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include <signal.h> // siginturrupt()
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/select.h>
#define segment_size 932

struct packet{
    char type;
    unsigned int length;
    int serial_number;
    char message[segment_size];
};

struct packet* pkt_init(char type, int len, int serial_num, char *message){
    struct packet* new_pkt = (struct packet*)malloc(sizeof(struct packet));
    new_pkt->type = type;
    new_pkt->length = len;
    new_pkt->serial_number = serial_num;
    memcpy(new_pkt->message, message, len);
    return new_pkt;
}
#endif