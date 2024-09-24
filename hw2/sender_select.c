#include "headers.h"
#include "handlefile.h"

int select_timeout(int sockfd, int usec){
    fd_set sets;
    struct timeval times;
    FD_ZERO(&sets);
    FD_SET(sockfd, &sets);
    times.tv_sec = usec / 1000000;
    times.tv_usec = usec % 1000000;
    return (select(sockfd+1, &sets, NULL, NULL, &times));
}

int main(int argc, char *argv[]){
    if(argc < 4){
        printf("Usage: ./sender_sengalarm <transfer_filename> <ip_addr> <port>\n");
        exit(EXIT_FAILURE);
    }

    // set socket 
    int sockfd;
    struct sockaddr_in send_addr;
    sockfd = socket(PF_INET, SOCK_DGRAM, 0);    // SOK_DGRAM -> UDP
    if(sockfd < 0){
        printf("socket failed\n");
        exit(EXIT_FAILURE);
    }

    memset(&send_addr, 0, sizeof(send_addr));
    send_addr.sin_family = AF_INET;
    send_addr.sin_addr.s_addr = inet_addr(argv[2]);
    send_addr.sin_port = htons(atoi(argv[3]));

    // handle transfer file
    unsigned int file_size, pkts_nums, pkt_last_size;
    char** transferBuff = handleFile(argv[1], &file_size, &pkts_nums, &pkt_last_size);

    // send and receive packet
    // pkt_init(char type, int len, int num, char *message)
    int alarmtime = 50000;
    int *recv_ack_num = malloc(sizeof(int));
    int lost_serial_num = 0;
    bool end_yet = 0;
    int timeout_cnt = 0;
    for(int i = 0 ; i <= pkts_nums+1 ; i++){
        if(timeout_cnt > 10 && end_yet) break;

        // printf("i: %d\n", i);
        struct packet* pkt;
        // start pkt
        if(i == 0){
            printf("send start\n");
            pkt = pkt_init('s', 6, i, "start\n");
            pkt->length = pkts_nums;
        }
        // end
        else if(i == pkts_nums + 1){
            printf("send end\n");
            end_yet = 1;
            pkt = pkt_init('e', 4, i, "end\n");
            for(int i = 0 ; i < 10 ; i++){
                pkt = pkt_init('e', 4, i, "end\n");
                sendto(sockfd, pkt, sizeof(*pkt), 0, (struct sockaddr*)&send_addr, sizeof(send_addr));
            }
        }
        // internal data
        else{
            if(i == pkts_nums){
                pkt = pkt_init('d', pkt_last_size, i, transferBuff[i-1]);
            }
            else{
                pkt = pkt_init('d', segment_size, i, transferBuff[i-1]);
            }
        }
        sendto(sockfd, pkt, sizeof(*pkt), 0, (struct sockaddr*)&send_addr, sizeof(send_addr));
        int k;
        if(select_timeout(sockfd, alarmtime) == 0){
            i--;
            timeout_cnt++;
            continue;
        }
        else{
            k = recvfrom(sockfd, recv_ack_num, sizeof(int), 0, NULL, NULL);
            timeout_cnt = 0;
        }
    }
    return 0;
}