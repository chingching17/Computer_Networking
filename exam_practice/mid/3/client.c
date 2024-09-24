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
#define Max_len 1024

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
    else{
        printf("socket ok\n");
    }

    // connect
    struct sockaddr_in serverInfo;
    bzero(&serverInfo, sizeof(serverInfo));
    serverInfo.sin_family = AF_INET;
    serverInfo.sin_addr.s_addr = inet_addr(argv[1]);
    serverInfo.sin_port = htons(atoi(argv[2]));
    // printf("the Server Connect to: %s:%d\n", inet_ntoa(serverInfo.sin_addr), ntohs(serverInfo.sin_port));
    // printf("You are: %s:%d\n", inet_ntoa(clientInfo.sin_addr), ntohs(clientInfo.sin_port));


    if(-1 == (connect(sockfd, (struct sockaddr*)&serverInfo, sizeof(serverInfo)))){
        printf("connect err\n");
        return 1;
    }
    else{
        printf("connect ok\n");
    }

    char request[Max_len];
    char recv_message[Max_len];
    fd_set total_set, ready_set;
    FD_ZERO(&total_set);
    FD_SET(STDIN_FILENO, &total_set);
    FD_SET(sockfd, &total_set);
    int exit_flag = 0, nready = 0;

    // listen
    while(1){
        ready_set = total_set;
        nready = select(sockfd+1, &ready_set, NULL, NULL, NULL);

        // send message to server
        if(FD_ISSET(STDIN_FILENO, &ready_set)){
            memset(request, '\0', sizeof(request));
            fgets(request, Max_len, stdin);
            if(strcmp(request, "EXIT\n") == 0){
                exit_flag = 1;
                break;
            }
            else{
                send(sockfd, request, sizeof(request), 0);
            }
        }

        // receive message from server
        if(FD_ISSET(sockfd, &ready_set)){
            memset(recv_message, '\0', sizeof(recv_message));
            int recv_len = recv(sockfd, recv_message, Max_len, 0);
            if(recv_len > 0){
                printf("%s", recv_message);
            }
            // server closed
            else if(recv_len == 0){
                exit_flag = 1;
                // break;
            }
            else{
                printf("recv err\n");
            }
        }
        if(exit_flag == 1) break;
    }
    
    close(sockfd);
    return 0;


}