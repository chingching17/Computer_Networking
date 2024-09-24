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

struct ClientInfo_bank{
    int sockfd;
    int balance;
    // char * history[Max_len];
    vector<string> history;
    int his_count;
    struct sockaddr_in details;


};
struct ClientInfo_bank cli_account;

int count_balance(int dollars, char type[]){
    int ntd = dollars;
    int balance_new;
    if(strcmp(type,"USD") == 0){
        ntd = dollars * 30;
    }
    balance_new = cli_account.balance - ntd;
    printf("withdraw:%d\n",balance_new);

    return balance_new;
}

int count_balance_deposit(int dollars, char type[]){
    int ntd = dollars;
    int balance_new;
    if(strcmp(type,"USD") == 0){
        ntd = dollars * 30;
    }
    balance_new = cli_account.balance + ntd;
    printf("deposit:%d\n",balance_new);
    return balance_new;
}

void handle_message(char inputbuf[]){
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
        if(cli_account.his_count < 0){
            sprintf(response, "%%%%%% No commands received so far\n");
            send(cli_account.sockfd, response, sizeof(response), 0);
            sprintf(response,"$$$ BALANCE: 0 NTD\n");
            send(cli_account.sockfd, response, sizeof(response), 0);
            return;
        }
        else{
            // haven't solved
            for(int i = 0 ; i <= cli_account.his_count ; i++){
                // printf("%s", cli_account.history[i]);
                cout << cli_account.history[i] << endl;
                sprintf(response,"%s",cli_account.history[i].c_str());
                send(cli_account.sockfd, response, sizeof(response), 0);
           
            }
            sprintf(response,"$$$ BALANCE: %d NTD\n",cli_account.balance);
            send(cli_account.sockfd, response, sizeof(response), 0);
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
        int now_balance = count_balance(dollars_int, type);
        if(now_balance > 0){
            cli_account.balance = now_balance;
            cli_account.his_count++;
            printf("count: %d\n", cli_account.his_count);

            char his2[Max_len];
            sprintf(his2,"### %s\n",inputbuf);
            string str_his = his2;
            cout << "his_str:" << str_his << endl;
            cli_account.history.push_back(str_his);
            sprintf(response,"### BALANCE: %d NTD\n",cli_account.balance);
            send(cli_account.sockfd, response, sizeof(response), 0);
            return;

        }
        else{
            cli_account.his_count++;
            printf("count: %d\n", cli_account.his_count);
            char his[Max_len];
            sprintf(his,"### %s(FAILED)\n",inputbuf);
            string str_his = his;
            cout << "his_str:" << str_his << endl;
            cli_account.history.push_back(str_his);
            sprintf(response,"!!! FAILED: Not enough money in the account\n");
            send(cli_account.sockfd, response, sizeof(response), 0);
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
        int now_balance_d = count_balance_deposit(dollars_int, type);
        if(now_balance_d > 0){
            cli_account.balance = now_balance_d;
            cli_account.his_count++;
            printf("count: %d\n", cli_account.his_count);

            char his3[Max_len];
            sprintf(his3,"### %s\n",inputbuf);
            string str_his = his3;
            cout << "his_str:" << str_his << endl;
            cli_account.history.push_back(str_his);
            sprintf(response,"### BALANCE: %d NTD\n",cli_account.balance);
            send(cli_account.sockfd, response, sizeof(response), 0);
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
    listen(sockfd, 5);
    char recv_message[Max_len];
    while(1){
        bzero(&client_info, sizeof(client_info));
        connfd = accept(sockfd, (struct sockaddr*)&client_info, &c_addrlen);
        if(connfd > 0){
            printf("accept ok\n");
        }
        cli_account.sockfd = connfd;
        cli_account.his_count = -1;
        cli_account.balance = 0;
        cli_account.details = client_info;
        cli_account.history.clear();
        // for(int i = 0 ; i < Max_len ; i++){
        //     cli_account.history[i] = NULL;
        // }
        int recv_len;
        for(;;){
            recv_len = recv(connfd, recv_message, Max_len, 0);
            if(recv_len > 0){
                printf("from client: %s", recv_message);
                handle_message(recv_message);
            }
            else{
                printf("doesn't receive\n");
                close(connfd);
                break;
            }
        }
    }

}