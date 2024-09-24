#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>    // bool
#include <ctype.h>
#include <sys/types.h> // recv
#include <sys/socket.h>
#include <netinet/in.h> // sockaddr_in
#include <unistd.h>
#include <arpa/inet.h>

#define Message_Max_len 1024
#define Max_client_num 15

struct Client_info{
    int sockfd;
    char name[15];
    struct sockaddr_in details;
    bool come_in;
    int ban;
};
struct Client_info client_info_array[Max_client_num];


void send_to_all(char yell_message[], int except){
    for(int i = 0 ; i < Max_client_num ; i++){
        if(i == except){
            continue;
        }
        else if(client_info_array[i].come_in == true){
            send(client_info_array[i].sockfd, yell_message, strlen(yell_message), 0);
        }
    }
}

int legal(char name[]){
    // not anonymous
    if(strcmp(name, "anonymous") == 0){
        return 1;
    }

    // unique
    for(int i = 0 ; i < Max_client_num ; i++){
        if(client_info_array[i].come_in && strcmp(name, client_info_array[i].name) == 0){
            return 2;
        }
    }

    // **2~12** English letters.
    if(strlen(name) < 2 || strlen(name) > 12) return 3;
    for(int i = 0 ; i < strlen(name) ; i++){
        if(!isalpha(name[i])) return 3;
    }

    return 4;
}

void handle_tell(int client_num, char name[], char tell_message[]){
    char response[Message_Max_len];
    bool self = 0, whom = 0;

    // if self is anonymous
    if(strcmp(client_info_array[client_num].name, "anonymous") == 0){
        self = 1;
    }

    // if who you sent is anonymous
    if(strcmp(name, "anonymous") == 0){
        whom = 1;
    }

    if(self && !whom){
        sprintf(response, "ERROR: You are anonymous.\n");
        send(client_info_array[client_num].sockfd, response, sizeof(response), 0);
        return;
    }
    else if(whom && !self){
        sprintf(response, "ERROR: The client to which you sent is anonymous.\n");
        send(client_info_array[client_num].sockfd, response, sizeof(response), 0);
        return;
    }
    else if(self && whom){
        sprintf(response, "ERROR: You are anonymous.\n");
        send(client_info_array[client_num].sockfd, response, sizeof(response), 0);
        sprintf(response, "ERROR: The client to which you sent is anonymous.\n");
        send(client_info_array[client_num].sockfd, response, sizeof(response), 0);
        return;
    }

    // tell message
    for(int i = 0 ; i < Max_client_num ; i++){
        if(strcmp(name, client_info_array[i].name) == 0){
            sprintf(response, "SUCCESS: Your message has been sent.\n");
            send(client_info_array[client_num].sockfd, response, sizeof(response), 0);
            sprintf(response, "%s tell you %s\n", client_info_array[client_num].name, tell_message);
            send(client_info_array[i].sockfd, response, sizeof(response), 0);
            return;
        }
    }

    // receiver doesn't exist
    sprintf(response, "ERROR: The receiver doesn't exist.\n");
    send(client_info_array[client_num].sockfd, response, sizeof(response), 0);
    return;
}

void handle_name(int client_num, char * split){
    char name[17];
    strcpy(name,split);
    printf("new name:%s\n", name);
    char response[Message_Max_len];

    int cases = legal(name);
    if(cases == 4){
        sprintf(response, "You're now known as %s.\n", name);
        send(client_info_array[client_num].sockfd, response, sizeof(response), 0);
        sprintf(response, "%s is now known as %s.\n", client_info_array[client_num].name, name);
        send_to_all(response, client_num);
        strcpy(client_info_array[client_num].name, name);
        return;
    }
    else if(cases == 1){
        sprintf(response, "ERROR: Username cannot be anonymous.\n");
        send(client_info_array[client_num].sockfd, response, sizeof(response), 0);
        return;
    }
    else if(cases == 2){
        sprintf(response, "ERROR: %s has been used by others.\n", name);
        send(client_info_array[client_num].sockfd, response, sizeof(response), 0);
        return;
    }
    else if(cases == 3){
        sprintf(response, "ERROR: Username can only consists of 2~12 English letters.\n");
        send(client_info_array[client_num].sockfd, response, sizeof(response), 0);
        return;
    }

}

void handle_who(int client_num){
    char response[Message_Max_len];

    for(int i = 0 ; i < Max_client_num ; i++){
        if(client_num == i){
            sprintf(response, "%s %s:%d ->me\n", client_info_array[i].name, inet_ntoa(client_info_array[i].details.sin_addr), ntohs(client_info_array[i].details.sin_port));
            send(client_info_array[client_num].sockfd, response, sizeof(response), 0);
        }
        else if(client_info_array[i].come_in == true){
            sprintf(response, "%s %s:%d\n", client_info_array[i].name, inet_ntoa(client_info_array[i].details.sin_addr), ntohs(client_info_array[i].details.sin_port));
            send(client_info_array[client_num].sockfd, response, sizeof(response), 0);
        }
    }
    return;
}

void handle_message(int client_num, char inputbuf[]){
    char message_ori[Message_Max_len];
    char cmd[Message_Max_len];
    char remain_message[Message_Max_len];
    char response[Message_Max_len];
    if(strlen(inputbuf) == 0){
        sprintf(response, "ERROR: Error command.\n");
        send(client_info_array[client_num].sockfd, response, sizeof(response), 0);
        return;
    }

    inputbuf[strlen(inputbuf)-1] = '\0';
    strcpy(message_ori, inputbuf);
    char * split = strtok(message_ori," \n");
    strcpy(cmd, split);
    strncpy(remain_message, inputbuf + strlen(cmd) + 1, strlen(inputbuf) - strlen(cmd));
    remain_message[strlen(inputbuf) - strlen(cmd)] = '\0';

    if(strcmp(split, "name") == 0){
        printf("token name: %s\n",split);
        split = strtok(NULL," \n");
        handle_name(client_num, split);
        printf("name successfully\n");
        return;
    }

    else if(strcmp(split, "yell") == 0){
        printf("yell successfully\n");
        if(strcmp(client_info_array[client_num].name,"anonymous") == 0){
            sprintf(response, "ERROR: You are anonymous.\n");
            send(client_info_array[client_num].sockfd, response, sizeof(response), 0);
            return;
        }
        sprintf(response, "%s yell %s\n", client_info_array[client_num].name, remain_message);
        send_to_all(response, -1);
        return;
    }

    else if(strcmp(split, "who") == 0){
        printf("token who: %s\n",split);
        handle_who(client_num);
        printf("who successfully\n");
        return;
    }

    else if(strcmp(split, "tell") == 0){
        printf("token tell: %s\n",split);
        char name[15];
        char tell_message[Message_Max_len];
        split = strtok(NULL, " \n");
        strcpy(name, split);
        strncpy(tell_message, remain_message + strlen(name) + 1, strlen(remain_message) - strlen(name));
        printf("tell message: %s\n", tell_message);
        tell_message[strlen(remain_message) - strlen(name)] = '\0';
        handle_tell(client_num, name, tell_message);
        printf("tell successfully\n");
        return;
    }

    else if(strcmp(split,"ban") == 0){
        client_info_array[client_num].ban = 1;
    }

    else{
        sprintf(response, "ERROR: Error command.\n");
        send(client_info_array[client_num].sockfd, response, sizeof(response), 0);
        return;
    }
}

int main(int argc, char *argv[]){
    if(argc != 2){
        printf("It's not the correct commend\n");
        printf("Usage: ./server <SERVER PORT>\n");
        return 0;
    }

    for(int i = 0 ; i < Max_client_num ; i++){
        client_info_array[i].come_in = false;
    }


    // construct socket
    // int socket(int domain, int type, int protocol);
    /*
        AF_INET 表示 IPv4 網路協定
        SOCK_STREAM 提供雙向, 可靠的, 連線導向的串流連線 (TCP)
        0，即根據選定的domain和type選擇使用預設協定
    */
    int sockfd = 0;
    if((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1){
        printf("fail to build socket\n");
    }
    else{
        printf("build socket successfully\n");
    }

    // 建立連線，使用 bind
    // int bind(int sockfd, const struct sockaddr *address, socklen_t address_len);
    // return 0 for successful; otherwise, -1 if error 
    struct sockaddr_in server_info;
    bzero(&server_info, sizeof(server_info));
    // memset(&server_info, 0, sizeof(struct sockaddr_in));
    server_info.sin_family = AF_INET;
    server_info.sin_addr.s_addr = INADDR_ANY;   // 指任何連上來的 address。如果要接受所有來自 internet 的 connection 可使用
    server_info.sin_port = htons(atoi(argv[1]));

    if((bind(sockfd, (struct sockaddr*)&server_info, sizeof(server_info))) < 0) {
        perror("ERROR: Socket binding failed");
        return EXIT_FAILURE;
    }

    // start select
    struct sockaddr_in client_info;
    char message[Message_Max_len];   // send to client
    char recvBuffer[Message_Max_len];  // message from client
    int maxfd = 0, num_ready = 0, recv_len = 0, newclifd = 0;
    int c_addr_len = sizeof(client_info);
    fd_set ready_set, client_set;
    listen(sockfd,10);
    FD_ZERO(&client_set);
    FD_SET(sockfd, &client_set);
    maxfd = sockfd;
    printf("sockfd: %d\n", sockfd);
    printf("maxfd: %d\n", maxfd);
    for(;;){
        ready_set = client_set;
        num_ready = select(maxfd + 1 , &ready_set, NULL, NULL, NULL);
        printf("num_ready: %d\n", num_ready);
        if(num_ready < 0){
            printf("select failed\n");
        }

        // check 有沒有新的連線進來
        if(FD_ISSET(sockfd, &ready_set)){
            printf("num_ready: %d -> in fd_isset\n", num_ready);

            // new client comming
            bzero(&client_info,sizeof(client_info));
            newclifd = accept(sockfd,(struct sockaddr*) &client_info, &c_addr_len);
            printf("accept sockfd: %d\n", sockfd);
            if(newclifd < 0)
            {
                perror("error accept, accept failed");
                close(newclifd);
                exit(EXIT_FAILURE);
            }
            if( maxfd < newclifd) maxfd = newclifd;
            sprintf(message, "Hello, anonymous! From: %s:%d\n", inet_ntoa(client_info.sin_addr), ntohs(client_info.sin_port));
            send(newclifd, message, sizeof(message), 0);
            send_to_all("Someone is coming!\n", -1);
            for(int i = 0 ; i < Max_client_num ; i++){
                if(client_info_array[i].come_in == false){
                    client_info_array[i].come_in = true;
                    strcpy(client_info_array[i].name, "anonymous");
                    client_info_array[i].sockfd = newclifd;
                    client_info_array[i].details = client_info;
                    client_info_array[i].ban = 1;
                    break;
                }
            }

            FD_SET(newclifd, &client_set);
            num_ready -= 1;
            if(num_ready <= 0 ){
                printf("no more work\n");
                continue;
            }
        }


        // 處理有新的資訊送過來
        for(int i = 0 ; i < Max_client_num ; i++){

            if(client_info_array[i].come_in == false){
                continue;
            }
            else if(FD_ISSET(client_info_array[i].sockfd, &ready_set)){
                printf("num_ready: %d -> in handle message\n", num_ready);

                memset(recvBuffer, '\0', Message_Max_len);
                recv_len = recv(client_info_array[i].sockfd, recvBuffer, Message_Max_len, 0);
                if(recv_len == 0){
                    // client leave
                    /* 
                        關連線
                        從 set 中移除
                        將 come_in 改為 false
                        告訴大家他離開了
                    */
                    close(client_info_array[i].sockfd);
                    FD_CLR(client_info_array[i].sockfd, &client_set);
                    client_info_array[i].come_in = false;
                    sprintf(message, "%s is offline.\n", client_info_array[i].name);
                    send_to_all(message, -1);
                }
                else if(recv_len == -1){
                    printf("recv_error");
                    close(client_info_array[i].sockfd);
                }
                else{
                    if(strcmp(recvBuffer, "\n") != 0){
                        handle_message(i, recvBuffer);
                    }
                }

                num_ready -= 1;
                if(num_ready <= 0){
                    break;
                }
            }
        }
    }
}