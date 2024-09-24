#include "headers.hpp"
#include <vector>
// #include <filesystem>
#include <dirent.h>
using namespace std;

class sockfd_name{
    public:
        char name[NAME_SIZE] = "\0";
        int sockfd;
        bool operator==(int rhs){ return this->sockfd == rhs;}
        sockfd_name(int sockfd):sockfd(sockfd){}
};
vector<sockfd_name> sockfd_lis;
vector<send_work> work_buf;
vector<Client> client;

int stablishConnect(int listenfd){ 
    struct sockaddr_in cli_addr;
    socklen_t cli_len = sizeof(cli_addr);

    int connfd;
    if((connfd = accept(listenfd, (struct sockaddr *) &cli_addr, &cli_len)) < 0)
    {
        // perror("Accept error");
        return -1;
    }
    if(DEBUG) printf("[Server] New connect from: %s:%d\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
    
    sockfd_lis.push_back(*new sockfd_name(connfd));

    int flags = fcntl(connfd, F_GETFL, 0);
    fcntl(connfd, F_SETFL, flags | O_NONBLOCK);
    return connfd;
}
bool del_from_client(char* name, int sockfd){
    for(int i=0;i<client.size();i++){
        if(strcmp(client[i].name, name) == 0){
            for(int j=0;j<client[i].sockfd.size();j++){
                if(client[i].sockfd[j] == sockfd){
                    client[i].sockfd.erase(client[i].sockfd.begin()+j);
                    return true;
                }
            }
        }
    }
    return false;
}
void send_all_connect(char * filepath, int filesize, int pos, char * filename,int sockfd){
    PKT_struct rebuff(NOT_SET);
    memset(&rebuff, '\0', sizeof(rebuff));
    file_pkt fpkt = file_to_pkt(filepath, filesize, filename);
    for(int i=0;i<client[pos].sockfd.size();i++){
        for(int j=0;j<fpkt.pkts.size();j++){
            if(client[pos].sockfd[i] == sockfd) continue;
            int sendlen = send(client[pos].sockfd[i], &(fpkt.pkts[j]), sizeof(fpkt.pkts[j]), 0);
            if(sendlen == -1 && errno == EWOULDBLOCK){
                if(DEBUG) printf("[SERVER] send block need to handle\n");
            }
        }
    }
    return; 
}

void send_to_new_client(char * filepath, int filesize, int pos, char * filename,int sockfd){
    PKT_struct rebuff(NOT_SET);
    memset(&rebuff, '\0', sizeof(rebuff));
    file_pkt fpkt = file_to_pkt(filepath, filesize, filename);
    for(int i=0;i<client[pos].sockfd.size();i++){
        for(int j=0;j<fpkt.pkts.size();j++){
            if(client[pos].sockfd[i] != sockfd) continue;
            int sendlen = send(client[pos].sockfd[i], &(fpkt.pkts[j]), sizeof(fpkt.pkts[j]), 0);
            if(sendlen == -1 && errno == EWOULDBLOCK){
                if(DEBUG) printf("[SERVER] send block need to handle\n");
            }
        }
    }
    return; 
}

void add_to_client(char* name, int sockfd){
    for(int i=0;i<client.size();i++){
        if(strcmp(client[i].name, name) == 0){
            printf("old register name %s\n", name);
            client[i].sockfd.push_back(sockfd);
            DIR *dir;
            struct dirent *ent;
            char path[3+NAME_SIZE];
            sprintf(path, "./%s", name);
            int count = 0;
            if ((dir = opendir (path)) != NULL) {
            /* print all the files and directories within directory */
                while ((ent = readdir (dir)) != NULL) {
                    printf ("%s\n", ent->d_name);
                    printf("%d\n", count);
                    char filename[NAME_SIZE*2+4];
                    sprintf(filename, "./%s/%s", name, ent->d_name);
                    printf("filename: %s\n",filename);
                    if(count == 0 || count == 1){
                        count++;
                        continue;
                    }
                    else{
                        FILE *file = fopen(filename, "r");
                        if(file == NULL){
                            printf("cannot open file\n");
                        }

                        // get size of file
                        fseek(file, 0, SEEK_END);
                        int file_size = ftell(file);
                        printf("file size: %d\n", file_size);
                        send_to_new_client(filename, file_size, i, ent->d_name, sockfd);
                    }
                }
                closedir (dir);
            } 
            else {
            /* could not open directory */
                perror ("can't open direcory");
            }
            return;
        }
    }
    if(DEBUG) printf("[SERVER] New register name %s\n", name);
    client.push_back(*new Client(name));
    client.back().sockfd.push_back(sockfd);

    char direc[3+NAME_SIZE];
    sprintf(direc, "./%s", name);
    struct stat str = {0};
    if (stat(direc, &str) == -1) {
        mkdir(direc, 0700);
    }
    return;
}
int find_client(char * name){
    for(int i=0;i<client.size();i++){
        if(strcmp(client[i].name, name) == 0){
            return i;
        }
    }
    return -1;
}

void server(int listenfd){

    ssize_t nRead, nWritten;
    while(1){
        //accept
        int connfd = stablishConnect(listenfd);
        int recvlen = 0;
        PKT_struct buffer(NOT_SET); // any way can dynamic assign recv size?
        memset(&buffer, '\0', sizeof(buffer));
        for(int i=0;i<sockfd_lis.size();i++){
            recvlen = recv(sockfd_lis[i].sockfd, &buffer, sizeof(buffer), 0);
            if(recvlen == -1){
                if(errno != EWOULDBLOCK){
                    perror("read error");
                    exit(-1);
                }
            }
            else if(recvlen == 0){
                // client leave
                bool flag = del_from_client(sockfd_lis[i].name, sockfd_lis[i].sockfd);
                if(flag && DEBUG) printf("[SERVER] Connect %d of client name %s left\n", sockfd_lis[i].sockfd, sockfd_lis[i].name);
                else if(DEBUG) printf("[SERVER] Connect %d of anonymous left\n", sockfd_lis[i].sockfd);
                close(sockfd_lis[i].sockfd);
                sockfd_lis.erase(sockfd_lis.begin() + i);
                i--;
            }
            else{
                // if(DEBUG) printf("recv buffer size %d %d\n", recvlen, i);
                //del_with_msg
                PKT_struct rebuff(NOT_SET);
                memset(&rebuff, '\0', sizeof(rebuff));
                char name[NAME_SIZE];
                if(buffer.type == HELLO){
                    printf("first\n");
                    strcpy(name, buffer.content);
                    strcpy(sockfd_lis[i].name, name);
                    rebuff.type = STDOUT_MESSAGE;
                    sprintf(rebuff.content, "Welcome to the dropbox-like server: %s\n", name);
                    int sendlen = send(sockfd_lis[i].sockfd, &rebuff, sizeof(rebuff), 0);
                    if(sendlen == -1 && errno == EWOULDBLOCK){
                        if(DEBUG) printf("[SERVER] send block need to handle\n");
                    }
                    add_to_client(name, sockfd_lis[i].sockfd);
                }
                else if(buffer.type == FILE_PKT){
                    // printf("send\n");
                    int pos = find_client(sockfd_lis[i].name);
                    if(pos == -1){
                        if(DEBUG) printf("%s name not register", sockfd_lis[i].name);
                    }
                    else{
                        int pktnumssss = 0;
                        if(buffer.flag == BEGIN_FILE) pktnumssss = 0;
                        pktnumssss += 1;                        
                        // put command
                        char filename[NAME_SIZE*2+4];
                        sprintf(filename, "./%s/%s", sockfd_lis[i].name, buffer.filename);
                        FILE * fp;
                        if(buffer.flag == ONLY_ONE){
                            printf("only one\n");
                            fp = Fopen(filename, "wb");
                            fseek(fp, 0, SEEK_SET);
                            fwrite(buffer.content, buffer.filesize % SEG_SIZE, 1, fp);
                            fclose(fp);
                            if(DEBUG && buffer.filesize == buffer.file_offset + sizeof(buffer.content)){
                                printf("[Server] end of file the same!!! get %d pkts\n", pktnumssss);
                            }
                            else if(DEBUG){
                                printf("[Server] flag end of file, but size didn't match\n");
                            }
                            send_all_connect(filename, buffer.filesize, pos, buffer.filename, sockfd_lis[i].sockfd);
                        }
                        else if(buffer.flag == BEGIN_FILE || buffer.flag == FIN_FILE || buffer.flag == MID_FILE){
                            if(buffer.flag == BEGIN_FILE){
                            // || buffer.flag == FIN_FILE
                                fp = Fopen(filename, "wb");
                            }
                            else{
                                fp = Fopen(filename, "ab");
                            }
                            fseek(fp, buffer.file_offset, SEEK_SET);
                            if(buffer.flag != FIN_FILE){
                                fwrite(buffer.content, SEG_SIZE, 1, fp);
                            }
                            else{
                                fwrite(buffer.content, buffer.filesize % SEG_SIZE, 1, fp);
                            }
                            fclose(fp);
                            // printf("fclose\n");
                            // to all connect in same name
                            if(buffer.flag == FIN_FILE){
                                if(DEBUG && buffer.filesize == buffer.file_offset + sizeof(buffer.content)){
                                    printf("[Server] end of file the same!!! get %d pkts\n", pktnumssss);
                                }
                                else if(DEBUG){
                                    printf("[Server] flag end of file, but size didn't match\n");
                                }
                                send_all_connect(filename, buffer.filesize, pos, buffer.filename, sockfd_lis[i].sockfd);
                            }
                        }
                    }
                }
                else{
                    if(DEBUG) printf("Error message type from %d of name %s", sockfd_lis[i].sockfd, sockfd_lis[i].name);
                }
                
            }
        }


    }
}

int main(int argc, char  **argv){
    if(argc < 2)
    {
        printf("[Server] Usage: ./server <port>\n");
        exit(-1);
    }

    int listenfd;
    struct sockaddr_in server_addr;
    char ip[INET_ADDRSTRLEN];

    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(atoi(argv[1]));

    if(bind(listenfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
    {
        perror("Bind error");
        exit(-1);
    }

    if(listen(listenfd, LISTENQ) < 0)
    {
        perror("Listen error");
        exit(-1);
    }

    inet_ntop(AF_INET, &(server_addr.sin_addr), ip, INET_ADDRSTRLEN);
    if(DEBUG) printf("[Server] Server is listening on %s:%s\n", ip, argv[1]);

    int flags;

    flags = fcntl(listenfd, F_GETFL, 0);
    fcntl(listenfd, F_SETFL, flags | O_NONBLOCK);


    server(listenfd);
    return 0;
}