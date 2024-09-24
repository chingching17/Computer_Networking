#include "headers.hpp"
using namespace std;

void doSleep(int sec)
{
    fprintf(stdout, "sleep %d\nThe client starts to sleep.\n",  sec);
    int i;
    for(i = 1; i <= sec; i++)
    {
        sleep(1);
        fprintf(stdout, "Sleep %d\n", i);
    }
    printf("Client wakes up.\n"); 
} 

void cli_Upload(int sockfd, int filesize, char * filename){
    file_pkt file_pkts = file_to_pkt(filename, filesize, filename);
    // printf("file_pkts.size: %ld\n", file_pkts.pkts.size());
    FILE * ch = fopen("check", "wb");
    if(file_pkts.pkts.size() == 1){
        fwrite(file_pkts.pkts[0].content, filesize % SEG_SIZE,1, ch);
        int sendlen = send(sockfd, &(file_pkts.pkts[0]), sizeof(file_pkts.pkts[0]), 0);
        // printProgress(file_pkts.pkts[0].file_offset+sizeof(file_pkts.pkts[0].content), filesize);
        printf("Progress : [######################]\n");
        if(sendlen == -1 && errno == EWOULDBLOCK){
            if(DEBUG) printf("\n\nUpload send block need to handle\n\n");
        }
    }
    else{
        for(int i=0;i<file_pkts.pkts.size();i++){
            fwrite(file_pkts.pkts[i].content, SEG_SIZE, 1, ch); 
            // fwrite(file_pkts.pkts[i].content, filesize, 1, ch);

            printProgress(file_pkts.pkts[i].file_offset, filesize);
            int sendlen = send(sockfd, &(file_pkts.pkts[i]), sizeof(file_pkts.pkts[i]), 0);
            // if(file_pkts.pkts[i].flag == FIN_FILE) printf("final\n");
            printProgress(file_pkts.pkts[i].file_offset+sizeof(file_pkts.pkts[i].content), filesize);
            if(sendlen == -1 && errno == EWOULDBLOCK){
                if(DEBUG) printf("\n\nUpload send block need to handle\n\n");
            }
        }
    }
    
    fclose(ch);

    printf("[Upload] %s Finish!\n", filename);
}
void cli(int sockfd){
    int maxfd = sockfd + 1;

    // int state = C_CONNECTING;??
    fd_set rset, allset;
    ssize_t nRead, nWritten;

    int sn;
    char final;
    char filename[256];
    uint32_t fileSize;
    int barFin;
    FD_ZERO(&allset);
    FD_SET(STDIN_FILENO, &allset);
    FD_SET(sockfd, &allset);
    while(1)
    {
        rset = allset;
        select(maxfd, &rset, NULL, NULL, NULL);

        if(FD_ISSET(STDIN_FILENO, &rset))//user cmd
        {
            //read input
            char command[CMD_SIZE];
            fgets(command, CMD_SIZE, stdin);
            // if(DEBUG) printf("get command %s\n", command);
            char tmp[CMD_SIZE];
            strcpy(tmp, command);
            char *cmd = strtok(tmp, DELI);
            if(cmd != NULL && strcmp(cmd, "/exit") == 0){
                close(sockfd);
                printf("Close connection.\n");
                return;
            }
            else if(cmd != NULL && strcmp(cmd, "/sleep") == 0)
            {
                char *t = strtok(NULL, DELI);
                if(t == NULL)
                    printf("You should input the seconds you want to sleep.\n");
                else
                    doSleep(atoi(t));
            }
            else if(cmd != NULL && strcmp(cmd, "/put") == 0)
            {
                char *f = strtok(NULL, DELI);
                if(f == NULL)
                    printf("You should input a filename.\n");
                else
                {
                    strcpy(filename, f);
                    FILE * fp = fopen(filename, "rb");
                    fileSize = getFileSize(fp);
                    fclose(fp);
                    fprintf(stdout, "[Upload] %s Start!\n", filename);
                    cli_Upload(sockfd, fileSize, filename);
                }
            }
            else
                printf("Error command.\n");
        }

        if(FD_ISSET(sockfd, &rset))// sever download
        {
            // printf("in\n");
            PKT_struct buff(NOT_SET);
            memset(&buff, '\0', sizeof(buff));
            int recvlen = recv(sockfd, &buff, sizeof(buff), 0);
            if(recvlen == -1){
                if(DEBUG) printf("recv error\n");
            }
            else if(recvlen == 0){
                close(sockfd);
                if(DEBUG) printf("Close cmd connection.\n");
                return;
            }
            else{
                if(buff.type == FILE_PKT){
                    // cli_Download();
                    FILE * fp;
                    if(buff.flag == ONLY_ONE){
                        fp = fopen(buff.filename, "wb");
                        // printProgress(buff.file_offset, buff.filesize);
                        printf("[Download] %s Start!\n", buff.filename);
                        printf("Progress : [######################]\n");
                        fwrite(buff.content, buff.filesize % SEG_SIZE, 1, fp);
                        printf("[Download] %s Finish!\n", buff.filename);

                    }
                    else{

                        if(buff.file_offset == 0){
                            fp = fopen(buff.filename, "wb");
                        }
                        else{
                            fp = fopen(buff.filename, "ab");
                            fseek(fp, buff.file_offset, SEEK_SET);
                        }
                        if(buff.flag == BEGIN_FILE){
                            printf("[Download] %s Start!\n", buff.filename);
                        }
                        printProgress(buff.file_offset, buff.filesize);
                        if(buff.flag == FIN_FILE || buff.flag == ONLY_ONE){
                            fwrite(buff.content, buff.filesize % SEG_SIZE, 1, fp);
                        }
                        else{
                            fwrite(buff.content, SEG_SIZE, 1, fp);
                        }
                        printProgress(buff.file_offset+sizeof(buff.content), buff.filesize);
                        if(buff.flag == FIN_FILE){
                            printf("[Download] %s Finish!\n", buff.filename);
                            // if(DEBUG) printf("got size %ld, realsize %d\n", buff.file_offset+sizeof(buff.content), buff.filesize);
                        }
                    }
                    fclose(fp);
                }
                else if(buff.type == STDOUT_MESSAGE){
                    printf("%s", buff.content);
                }
                else{
                    if(DEBUG){
                        printf("Doesn't reconize message\n");
                    }
                }
            }
        }

    }
    return;
}

int main(int argc, char **argv){
    if(argc < 4)
    {
        printf("Usage: ./client <ip> <port> <username>.\n");
        exit(-1);
    }

    int sockfd;
    struct addrinfo hints, *servinfo;
    struct sockaddr_in server_addr;
    char *server_ip;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if(getaddrinfo(argv[1], argv[2], &hints, &servinfo))
    {
        perror("Get addr info error");
        exit(-1);
    }

    server_ip = inet_ntoa(((struct sockaddr_in *) servinfo->ai_addr)->sin_addr);
    if(DEBUG) printf("Connecting to %s:%s...\n", server_ip, argv[2]);

    

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));

    if(connect(sockfd, servinfo->ai_addr, servinfo->ai_addrlen) < 0)
    {
        perror("Connect error");
        exit(-1);
    }
    PKT_struct message(HELLO);
    strcpy(message.content, argv[3]);
    send(sockfd, &message, sizeof(message), 0);
    if(DEBUG) printf("connect\n");

    int flags;
    flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
    cli(sockfd);

    return 0;
}