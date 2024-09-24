#include "headers.h"
#include "handlefile.h"

static void sig_alarm(int signal){
    // printf("sig_alarm\n");
    return;
}

int main(int argc, char *argv[]){
    if(argc < 4){
        printf("Usage: ./sender_sengalarm <transfer_filename> <ip_addr> <port>\n");
        exit(EXIT_FAILURE);
    }

    signal(SIGALRM, sig_alarm);
    siginterrupt(SIGALRM, 1);

    // set socket 
    int sockfd;
    struct sockaddr_in send_addr;
    sockfd = socket(PF_INET, SOCK_DGRAM, 0);    // UDP
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
    int alarmtime = 10000;
    int *recv_ack_num = malloc(sizeof(int));
    int lost_serial_num = 0;
    int ack_record[pkts_nums+5];
    memset(ack_record, 0, sizeof(ack_record));
    int ack_num = 0;
    int timeout_cnt = 0;
    bool send_yet = 0;

    for(int i = 0 ; i <= pkts_nums+1 ; i++){
        struct packet* pkt;

        // printf("i: %d\n", i);
        if(ack_record[i] == 1) continue;
        if(ack_record[pkts_nums+1]) break;
        if(timeout_cnt > 10 && send_yet){
            break;
        }

        // start pkt
        if(i == 0){
            printf("send start\n");
            pkt = pkt_init('s', 6, i, "start\n");
            pkt->length = pkts_nums;
        }
        // end
        else if(i == pkts_nums + 1){
            if(lost_serial_num < pkts_nums+1){
                i = lost_serial_num - 1;
                continue;
            }
            printf("send end\n");
            send_yet = 1;
            pkt = pkt_init('e', 4, i, "end\n");
            for(int i = 0 ; i < 10 ; i++){
                pkt = pkt_init('e', 4, i, "end\n");
                sendto(sockfd, pkt, sizeof(*pkt), 0, (struct sockaddr*)&send_addr, sizeof(send_addr));
            }
        }
        // internal data
        else{
            if(ack_record[0] == 0){
                i = -1;
                continue;
            }
            // printf("send data\n");
            if(i == pkts_nums){
                pkt = pkt_init('d', pkt_last_size, i, transferBuff[i-1]);
            }
            else{
                pkt = pkt_init('d', segment_size, i, transferBuff[i-1]);
            }
        }
        sendto(sockfd, pkt, sizeof(*pkt), 0, (struct sockaddr*)&send_addr, sizeof(send_addr));
        ualarm(alarmtime, 0);
        int k;
        while(1){
            k = recvfrom(sockfd, recv_ack_num, sizeof(int), 0, NULL, NULL);
            if(k < 0){
                if(errno == EINTR){
                    // printf("timout\n");
                    timeout_cnt++;
                    if(timeout_cnt > 10 && send_yet) break;
                    for(int j = lost_serial_num ; j <= pkts_nums+1 ; j++){
                        if(ack_record[j] == 0){
                            lost_serial_num = j;
                            break;
                        }
                    }
                }
                // printf("receive err\n");
                break;
            }
            else{
                // ack received
                timeout_cnt = 0;
                // printf("ack num: %d\n", *recv_ack_num);
                ack_num++;
                ack_record[*recv_ack_num] = 1;
            }
        }
    }
    return 0;
}