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
#include <vector>
#include <cstring>
#include <iostream>
using namespace std;

#define Max_len 1024
#define Max_client_num 15


struct ClientInfo_bank{
    bool come_in;
    int sockfd;
    int balance;
    // char * history[Max_len];
    vector<string> history;
    int his_count;
    struct sockaddr_in details;
};
struct ClientInfo_bank cli_account[Max_client_num];

int count_balance(int client_num, int dollars, char type[]){
    int ntd = dollars;
    int balance_new;
    if(strcmp(type,"USD") == 0){
        ntd = dollars * 30;
    }
    balance_new = cli_account[client_num].balance - ntd;
    printf("withdraw:%d\n",balance_new);

    return balance_new;
}

int count_balance_deposit(int client_num, int dollars, char type[]){
    int ntd = dollars;
    int balance_new;
    if(strcmp(type,"USD") == 0){
        ntd = dollars * 30;
    }
    balance_new = cli_account[client_num].balance + ntd;
    printf("deposit:%d\n",balance_new);
    return balance_new;
}

void handle_message(int client_num, char inputbuf[]){
    char cmd[Max_len];
    char remain_message[Max_len];
    char response[Max_len];

    inputbuf[strlen(inputbuf)-1] = '\0';
    sscanf(inputbuf, "%s", cmd);
    printf("cmd: %s\n", cmd);

    strncpy(remain_message, inputbuf + strlen(cmd) + 1, strlen(inputbuf) - strlen(cmd));
    remain_message[strlen(inputbuf) - strlen(cmd)] = '\0';
    printf("remain_message:%s\n", remain_message);

    if(strcmp(cmd, "HISTORY") == 0){
        if(cli_account[client_num].his_count < 0){
            sprintf(response, "%%%%%% No commands received so far\n");
            send(cli_account[client_num].sockfd, response, sizeof(response), 0);
            sprintf(response,"$$$ BALANCE: 0 NTD\n");
            send(cli_account[client_num].sockfd, response, sizeof(response), 0);
            return;
        }
        else{
            // haven't solved
            for(int i = 0 ; i <= cli_account[client_num].his_count ; i++){
                // printf("%s", cli_account[client_num].history[i]);
                cout << cli_account[client_num].history[i] << endl;
                sprintf(response,"%s",cli_account[client_num].history[i].c_str());
                send(cli_account[client_num].sockfd, response, sizeof(response), 0);
           
            }
            sprintf(response,"$$$ BALANCE: %d NTD\n",cli_account[client_num].balance);
            send(cli_account[client_num].sockfd, response, sizeof(response), 0);
            return;
        }
    }

    char dollars[Max_len];
    char type[Max_len];

    if(strcmp(cmd,"WITHDRAW") == 0){
        sscanf(remain_message, "%s", dollars);
        dollars[strlen(dollars)] = '\0';
        printf("dollars:%s\n",dollars);
        strncpy(type, remain_message + strlen(dollars) + 1, strlen(remain_message) - strlen(dollars));
        printf("type:%s\n", type);
        int dollars_int;
        dollars_int = atoi(dollars);
        printf("dollars int:%d\n",dollars_int);
        int now_balance = count_balance(client_num,dollars_int, type);
        if(now_balance > 0){
            cli_account[client_num].balance = now_balance;
            cli_account[client_num].his_count++;
            printf("count: %d\n", cli_account[client_num].his_count);

            char his2[Max_len];
            sprintf(his2,"### %s\n",inputbuf);
            string str_his = his2;
            cout << "his_str:" << str_his << endl;
            cli_account[client_num].history.push_back(str_his);
            sprintf(response,"### BALANCE: %d NTD\n",cli_account[client_num].balance);
            send(cli_account[client_num].sockfd, response, sizeof(response), 0);
            return;

        }
        else{
            cli_account[client_num].his_count++;
            printf("count: %d\n", cli_account[client_num].his_count);
            char his[Max_len];
            sprintf(his,"### %s(FAILED)\n",inputbuf);
            string str_his = his;
            cout << "his_str:" << str_his << endl;
            cli_account[client_num].history.push_back(str_his);
            sprintf(response,"!!! FAILED: Not enough money in the account\n");
            send(cli_account[client_num].sockfd, response, sizeof(response), 0);
            return;
        }

    }

    if(strcmp(cmd,"DEPOSIT") == 0){
        sscanf(remain_message, "%s", dollars);
        printf("dollars:%s\n",dollars);
        strncpy(type, remain_message + strlen(dollars) + 1, strlen(remain_message) - strlen(dollars));
        printf("type:%s\n", type);
        int dollars_int;
        dollars_int = atoi(dollars);
        printf("dollars int:%d\n",dollars_int);
        int now_balance_d = count_balance_deposit(client_num, dollars_int, type);
        if(now_balance_d > 0){
            cli_account[client_num].balance = now_balance_d;
            cli_account[client_num].his_count++;
            printf("count: %d\n", cli_account[client_num].his_count);

            char his3[Max_len];
            sprintf(his3,"### %s\n",inputbuf);
            string str_his = his3;
            cout << "his_str:" << str_his << endl;
            cli_account[client_num].history.push_back(str_his);
            sprintf(response,"### BALANCE: %d NTD\n",cli_account[client_num].balance);
            send(cli_account[client_num].sockfd, response, sizeof(response), 0);
            return;
        }
        

    }


}

int main(int argc, char *argv[]){
    if(argc != 2){
        printf("It's not the correct commend\n");
        printf("Usage: ./server <SERVER PORT>\n");
        return 0;
    }

    for(int i = 0 ; i < Max_client_num ; i++){
        cli_account[i].come_in = false;
    }

    int sockfd = 0;
    if((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1){
        printf("fail to build socket\n");
    }
    else{
        printf("build socket successfully\n");
    }

    struct sockaddr_in server_info, client_info;
    bzero(&server_info, sizeof(server_info));
    // memset(&server_info, 0, sizeof(struct sockaddr_in));
    server_info.sin_family = AF_INET;
    server_info.sin_addr.s_addr = INADDR_ANY;   // 指任何連上來的 address。如果要接受所有來自 internet 的 connection 可使用
    server_info.sin_port = htons(atoi(argv[1]));

    if((bind(sockfd, (struct sockaddr*)&server_info, sizeof(server_info))) < 0) {
        perror("ERROR: Socket binding failed");
        return EXIT_FAILURE;
    }
    else{
        printf("bind ok\n");
    }

    socklen_t c_addrlen = sizeof(client_info);
    int connfd = 0;
    listen(sockfd, 10);
    char recv_message[Max_len];
    int maxfd = 0, num_ready = 0, recv_len = 0, newclifd = 0;
    fd_set ready_set, client_set;
    listen(sockfd,10);
    FD_ZERO(&client_set);
    FD_SET(sockfd, &client_set);
    maxfd = sockfd;
    while(1){
        ready_set = client_set;
        num_ready = select(maxfd + 1 , &ready_set, NULL, NULL, NULL);
        if(num_ready < 0){
            printf("select failed\n");
        }

        if(FD_ISSET(sockfd, &ready_set)){
            printf("num_ready: %d -> in fd_isset\n", num_ready);
            socklen_t c_addr_len = sizeof(client_info);
            // new client comming
            bzero(&client_info,sizeof(client_info));
            newclifd = accept(sockfd,(struct sockaddr*) &client_info, &c_addr_len);
            // printf("accept sockfd: %d\n", sockfd);
            if(newclifd < 0)
            {
                perror("error accept, accept failed");
                close(newclifd);
                exit(EXIT_FAILURE);
            }
            if( maxfd < newclifd) maxfd = newclifd;
            for(int i = 0 ; i < Max_client_num ; i++){
                if(cli_account[i].come_in == false){
                    cli_account[i].come_in = true;
                    cli_account[i].sockfd = newclifd;
                    cli_account[i].his_count = -1;
                    cli_account[i].balance = 0;
                    cli_account[i].details = client_info;
                    cli_account[i].history.clear();
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

            if(cli_account[i].come_in == false){
                continue;
            }
            else if(FD_ISSET(cli_account[i].sockfd, &ready_set)){
                // printf("num_ready: %d -> in handle message\n", num_ready);

                memset(recv_message, '\0', Max_len);
                recv_len = recv(cli_account[i].sockfd, recv_message, Max_len, 0);
                if(recv_len == 0){
                    // client leave
                    /* 
                        關連線
                        從 set 中移除
                        將 come_in 改為 false
                        告訴大家他離開了
                    */
                    close(cli_account[i].sockfd);
                    FD_CLR(cli_account[i].sockfd, &client_set);
                    cli_account[i].come_in = false;
                }
                else if(recv_len == -1){
                    printf("recv_error");
                    close(cli_account[i].sockfd);
                }
                else{
                    printf("from client: %s", recv_message);
                    handle_message(i,recv_message);
                }

                num_ready -= 1;
                if(num_ready <= 0){
                    break;
                }
            }
        }
        // bzero(&client_info, sizeof(client_info));
        // connfd = accept(sockfd, (struct sockaddr*)&client_info, &c_addrlen);
        // if(connfd > 0){
        //     printf("accept ok\n");
        // }
        // cli_account.sockfd = connfd;
        // cli_account.his_count = -1;
        // cli_account.balance = 0;
        // cli_account.details = client_info;
        // cli_account.history.clear();
        // // for(int i = 0 ; i < Max_len ; i++){
        // //     cli_account.history[i] = NULL;
        // // }
        // int recv_len;
        // for(;;){
        //     recv_len = recv(connfd, recv_message, Max_len, 0);
        //     if(recv_len > 0){
        //         printf("from client: %s", recv_message);
        //         handle_message(recv_message);
        //     }
        //     else
        //     {
        //         printf("doesn't receive\n");
        //         close(connfd);
        //         break;
        //     }
        // }
    }

}