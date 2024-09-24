#ifndef _HEADER_H_
#define _HEADER_H_
#include <math.h>
#include <ctype.h>
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string>
#include <string.h>
#include <signal.h>
#include <strings.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <vector>

#define NAME_SIZE 256
#define SEG_SIZE  1024

#define DELI " \a\t\r\n"

#define NOT_SET -1

// type of PKT
#define HELLO               0
#define STDOUT_MESSAGE      1
#define FILE_PKT            2
#define BYE                 3

// flag of PKT_struct
#define BEGIN_FILE  0
#define FIN_FILE    1
#define MID_FILE    2
#define NOT_SET     -1
#define ONLY_ONE   3

#define DEBUG 0

class Client{
    public:
        std::vector<int> sockfd;
        int flags = NOT_SET;
        char name[NAME_SIZE];
        Client(char * name_in){
            strcpy(name, name_in);
        }
};

class PKT_struct{
    public:
        int type = NOT_SET; // hello
        char filename[NAME_SIZE*4+4];
        int filesize;
        int file_offset;
        int sn = 0; // may not used
        int flag = NOT_SET; 
        char content[SEG_SIZE];
        PKT_struct(int type):type(type){}
        PKT_struct(char * filename_in, int file_offset, int filesize, int sn, char * content_in)\
        :file_offset(file_offset), filesize(filesize), sn(sn){
            type = FILE_PKT;
            strcpy(filename, filename_in);
            memcpy(content, content_in, SEG_SIZE);
        }
        // void printPKT();
};

class file_pkt{
    public:
        char filename[NAME_SIZE*4+4];
        std::vector<PKT_struct> pkts;
        file_pkt(char *filename_in){
            strcpy(filename, filename_in);
        }
};

file_pkt file_to_pkt(char * filepath, int filesize, char* filename);

#endif