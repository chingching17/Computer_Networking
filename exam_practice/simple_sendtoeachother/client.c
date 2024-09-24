#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>


int main(int argc, char *argv[]){
    if(argc != 3){
        printf("ERROR cmd, Usage: ./client <ip> <port>\n");
        return 0;
    }

    // construct socket
    int sockfd = 0;
    sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if(sockfd < 0){
        printf("construct socket failed\n");
    }

    // connect
    struct sockaddr_in serverInfo;
    bzero(&serverInfo, sizeof(serverInfo));
    serverInfo.sin_family = AF_INET;
    serverInfo.sin_addr.s_addr = inet_addr(argv[1]);
    serverInfo.sin_port = htons(atoi(argv[2]));

    if(-1 == (connect(sockfd, (struct sockaddr*)&serverInfo,sizeof(serverInfo)))){
        printf("connect err\n");
    }

    char recv_message[100];
    char request[100];
    for(;;){
        if(fgets(request, 100, stdin) != NULL){
            fputs(request, stdin);
            // sprintf(request,"hello from client");
            send(sockfd, request, sizeof(request), 0);
        }
        
        int recv_len = recv(sockfd, recv_message, 100, 0);
        if(recv_len > 0){
            printf("from server: %s\n", recv_message);
            // break;
        }
    }

    printf("close socket\n");
    close(sockfd);
    return 0;


}