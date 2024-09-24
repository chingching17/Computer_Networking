#ifndef HANDLEFILE_H
#define HANDLEFILE_H

char** handleFile(const char *filename, unsigned int *file_size, unsigned int *pkts_nums, unsigned int *pkt_last_size){
    // open file
    FILE *file = fopen(filename, "r"); // r means reading existing file, b means binary file
    if(file == NULL){
        printf("cannot open file\n");
        return NULL;
    }

    // get size of file
    fseek(file, 0, SEEK_END);
    *file_size = ftell(file);
    // rewind(file);   // if seeking to go to the beginning of transfer file
    fseek(file, 0, SEEK_SET);
    printf("the size of transfer file is %d\n", *file_size);

    // calculate number of pkts
    *pkts_nums = *file_size / segment_size;
    *pkt_last_size = segment_size;
    if(*file_size % segment_size != 0){
        *pkt_last_size = *file_size % segment_size;
        (*pkts_nums)++;
    }
    printf("number of pkts: %d\n", *pkts_nums);

    // split transfer file
    char **transferData = (char **)malloc(*pkts_nums * sizeof(char *));
    for(int i = 0 ; i < *pkts_nums ; i++){
        // allocate space for reading data
        transferData[i] = (char *)malloc(segment_size * sizeof(char));

        // memset data
        memset(transferData[i], 0, segment_size);

        // read data to packet
        int suc = fread(transferData[i], 1, segment_size, file);
        if(suc < 0){
            printf("fread failed\n");
            exit(EXIT_FAILURE);
        }
    }
    fclose(file);
    return transferData;
}

#endif