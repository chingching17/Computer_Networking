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
    if(argc != 2){
        printf("error input\n");
        return 0;
    }

    // build socket
    int sockfd = 0;
    sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if(sockfd < 0){
        printf("socket build err\n");
    }
    else{
        printf("socket build ok\n");
    }

    // sockaddr_in
    struct sockaddr_in serverInfo;
    serverInfo.sin_family = AF_INET;
    serverInfo.sin_addr.s_addr = INADDR_ANY;
    serverInfo.sin_port = htons(atoi(argv[1]));

    // bind
    if((bind(sockfd, (struct sockaddr*)&serverInfo, sizeof(serverInfo))) < 0){
        printf("Error: socket binding failed\n");
        close(sockfd);
        return EXIT_FAILURE;
    }

    // listen 
    if(-1 == listen(sockfd, 10)){
        printf("listen failed\n");
        close(sockfd);
        return EXIT_FAILURE;

    }

    struct sockaddr_in clientInfo;
    int len;
    int connfd;
    char receive_message[100];
    char response[100];
    // accept
    for(;;){
        len = sizeof(clientInfo);
        connfd = accept(sockfd, (struct sockaddr*)&clientInfo, &len);
        printf("connect from %s, port %d\n", inet_ntoa(clientInfo.sin_addr), ntohs(clientInfo.sin_port));
        int recv_len;
        for(;;){
            recv_len = recv(connfd, receive_message, 100, 0);

            if(recv_len > 0){
                printf("from client: %s", receive_message);
                sprintf(response,"it's message from server");
                send(connfd, response, sizeof(response), 0);
                // break;
            }
            else{
                break;
            }
            
        }

        // if(recv_len <= 0){
        //     break;
        // }


    }
    printf("close sockfd\n");
    close(connfd);

    return 0;

}