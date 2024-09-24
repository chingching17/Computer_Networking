#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#define LENGTH_MSG 1024

int main(int argc, char *argv[]){
    if(argc != 3){
        printf("Error cmd, Usage: ./client <SERVER IP> <SERVER PORT>\n");
        return 0;
    }

    int sockfd = 0;
    sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if(sockfd < 0){
        printf("fail to build a socket\n");
    }

    // socket 連線
    struct sockaddr_in serverInfo, clientInfo;
    bzero(&serverInfo, sizeof(serverInfo));
    bzero(&clientInfo, sizeof(clientInfo));

    serverInfo.sin_family = AF_INET;
    serverInfo.sin_addr.s_addr = inet_addr(argv[1]);    // ip
    serverInfo.sin_port = htons(atoi(argv[2])); // port

    socklen_t cLen = sizeof(clientInfo);
    socklen_t sLen = sizeof(serverInfo);
    getsockname(sockfd, (struct sockaddr*) &clientInfo, &cLen);
    getpeername(sockfd, (struct sockaddr*) &serverInfo, &sLen);
    // printf("client: %s: %d\nServer: %s: %d\n", inet_ntoa(clientInfo.sin_addr), ntohs(clientInfo.sin_port), inet_ntoa(serverInfo.sin_addr), ntohs(serverInfo.sin_port));
    // printf("the Server Connect to: %s:%d\n", inet_ntoa(serverInfo.sin_addr), ntohs(serverInfo.sin_port));
    // printf("You are: %s:%d\n", inet_ntoa(clientInfo.sin_addr), ntohs(clientInfo.sin_port));

    if(-1 == connect(sockfd, (struct sockaddr*)&serverInfo, sizeof(serverInfo))){
        printf("connect error\n");
    }
    
    fd_set total_set, ready_set;
    FD_ZERO(&total_set);
    FD_SET(0, &total_set);
    FD_SET(sockfd, &total_set);
    int num_ready = 0;
    // printf("sockfd: %d\n", sockfd);
    char recv_message[LENGTH_MSG];
    char message[LENGTH_MSG];
    //Send a stdinput message to server
    while(1){
        ready_set = total_set;
        num_ready = select(sockfd+1, &ready_set, NULL, NULL, NULL);
        // send message to server
        if(FD_ISSET(0, &ready_set)){
            memset(message, '\0', sizeof(message));
            fgets(message, LENGTH_MSG, stdin);
                
            if( strcmp(message, "exit\n") == 0 ){
                break;
            }
            else{
                //printf("send message to server %s", message);
                send(sockfd, message, LENGTH_MSG, 0);
            }
        }

        // receive message from server
        if(FD_ISSET(sockfd, &ready_set)){
            memset(recv_message, '\0', sizeof(recv_message));
            int recv_len = recv(sockfd, recv_message, LENGTH_MSG, 0);
                // printf("recv_len %d\n", recv_len);
            if(recv_len > 0){
                printf("[Server] %s", recv_message);
            }
            else if(recv_len == 0){
                //sever closed
                break;
            }
            else{
                printf("receive error\n");
                return 0;
            }
        }
        
    }


    // printf("close socket\n");
    close(sockfd);
    return 0;
}