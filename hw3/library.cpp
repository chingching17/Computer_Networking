#include "header.hpp"

file_pkt file_to_pkt(char * filepath, int filesize, char* filename){
    FILE * fp = fopen(filepath, "rb");

    int offset = 0;
    char content_buffer[SEG_SIZE];
    memset(&content_buffer, '\0', SEG_SIZE);
    int number = 0;
    //count file size
    file_pkt file_pkts = *new file_pkt(filename);
    while(offset < filesize){
        // fseek(fp, offset, SEEK_SET);
        fread(content_buffer, 1, SEG_SIZE, fp);
        file_pkts.pkts.push_back(*new PKT_struct(filename, offset, filesize, number, content_buffer));
        //PKT_struct(char * filename_in, int file_offset, int filesize, int sn, char * content_in)
        offset += SEG_SIZE;
        number += 1;
    }
    // FILE * ch = Fopen("check", "wb");

    int pktsnum = file_pkts.pkts.size();
    // printf("make pkt num : %d !!!!!!\n", pktsnum);

    if(pktsnum == 1){
        file_pkts.pkts[0].flag = ONLY_ONE;
        return file_pkts;
    }

    for(int i=0;i<pktsnum;i++){
        // fwrite(file_pkts.pkts[i].content, SEG_SIZE, 1, ch);
        if(i == 0){
            file_pkts.pkts[i].flag = BEGIN_FILE;
        }
        else if(i == pktsnum-1){
            file_pkts.pkts[i].flag = FIN_FILE;
        }
        else{
            file_pkts.pkts[i].flag = MID_FILE;
        }
    }
    // fclose(ch);
    return file_pkts;
}

int printProgress(uint32_t sent, uint32_t total)
{
    int num = ((double) sent / total) * 20;
    if(num >= 20) num = 20;
    int left = 20 - num;
    int i;

    fprintf(stdout, "Progress : [");
    for(i = 0; i < num; i++) fprintf(stdout, "#");
    for(i = 0; i < left; i++) fprintf(stdout, " ");
    fprintf(stdout, "]");

    fflush(stdout);
    if(num == 20)
        printf("\n");
    else
        printf("\r");
    return num == 20;
}