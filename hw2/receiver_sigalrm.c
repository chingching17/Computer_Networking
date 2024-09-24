#include "headers.h"
#include <sys/time.h> // gettimeofday()

int pkts_nums;
char **segments;
int *segment_sizes;
int start_cnt = 0;

bool record_msg(struct packet* message, struct sockaddr_in* client, int sockfd){
    int *ack_num = malloc(sizeof(int));
    *ack_num = message->serial_number;
    // printf("acknum %d\n", *ack_num);
    if(message->type == 's'){
        printf("receive start \n");

        // send ack
        sendto(sockfd, ack_num, sizeof(*ack_num), 0, (struct sockaddr*)client, sizeof(*client));

        // malloc space for segments
        if(!start_cnt){
            segments = (char **)malloc(message->length * sizeof(char*));
            segment_sizes = (int *)malloc(message->length * sizeof(int));
        }
        start_cnt++;

        return false;
    }
    else if(message->type == 'd'){
        // record data
        printf("receive data serial num %d\n", message->serial_number);
        segments[message->serial_number-1] = (char *)malloc(segment_size * sizeof(char));
        memcpy(segments[message->serial_number-1], message->message, message->length);
        segment_sizes[message->serial_number-1] = message->length;
        // send ack
        sendto(sockfd, ack_num, sizeof(*ack_num), 0, (struct sockaddr*)client, sizeof(*client));
        return false;
    }
    else if(message->type == 'e'){
        printf("received end\n");
        printf("receive data serial num %d\n", message->serial_number);
        // send ack
        sendto(sockfd, ack_num, sizeof(*ack_num), 0, (struct sockaddr*)client, sizeof(*client));
        return true;
    }
    else{
        printf("there is a bug\n");
        return true;
    }
    return false;
}

int main(int argc, char *argv[]){
    if(argc < 3){
        printf("Usage: ./receiver_sigalarm.out <save file name> <bind port>\n");
        exit(EXIT_FAILURE);
    }

    int sockfd;
    struct sockaddr_in serveraddr, clientaddr;
    // build socket
    sockfd = socket(PF_INET, SOCK_DGRAM, 0);
    memset(&serveraddr, 0, sizeof(serveraddr));
    memset(&clientaddr, 0, sizeof(clientaddr));

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(atoi(argv[2]));

    // bind
    bind(sockfd, (struct sockaddr*)& serveraddr, sizeof(serveraddr));

    // record message
    struct packet* recv_msg = (struct packet*)malloc(sizeof(struct packet));
    int client_size = sizeof(clientaddr);
    int recv_err = 0;
    struct timeval start_t, end_t;
    gettimeofday(&start_t, 0);
    while(1){
        int n = recvfrom(sockfd, recv_msg, sizeof(*recv_msg), 0, (struct sockaddr*)&clientaddr, &client_size);
        if(n < 0){
            if(errno == EINTR){
                    // printf("here\n");
                    recv_err++;
                }
            if(recv_err > 10) break;
            printf("receive err\n");
        }
        else{
            if(recv_msg->type == 's'){
                pkts_nums = recv_msg->length;
            }
            if(record_msg(recv_msg, &clientaddr, sockfd)) break;
        }
    }
    printf("total needed pkt %d\n", pkts_nums);
    gettimeofday(&end_t, 0);
    int sec = end_t.tv_sec - start_t.tv_sec;
    int usec = end_t.tv_usec - start_t.tv_usec;
    printf("Total elapsed time %.4f sec\n", sec+(usec/1000000.0));

    // write file
    FILE *file = fopen(argv[1], "w");
    for(int i = 0 ; i < pkts_nums ; i++){
        size_t k = fwrite(segments[i], 1, segment_sizes[i], file);
        if(k < 0){
            printf("fwrite err\n");
            exit(EXIT_FAILURE);
        }
    }
    fclose(file);
    free(segment_sizes);
    free(segments);
    return 0;
    
}