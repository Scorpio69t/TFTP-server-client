#include "../include/packet.hpp"

void send_first_request(int sock, char* filepath, struct sockaddr_in sockad, struct opts o, int operation) {
    char buffer[1024];
    char mode[10] = MODE;
    int buffer_len = 0;
    uint16_t opcode = htons(operation);
    uint8_t end_string = 0;
    int i;
    int n;
    char c[2];

    // opcode

    memcpy(buffer, &opcode, 2);
    buffer_len += 2;
    if (operation == WRQ) {
        fprintf(stdout, "WRQ ");
    } else if (operation == RRQ) {
        fprintf(stdout, "RRQ ");
    }

    // filename

    strcpy(buffer+buffer_len, filepath);
    buffer_len += strlen(filepath);

    memcpy(buffer+buffer_len, &end_string, 1);
    buffer_len++;

    fprintf(stdout, "%s ", filepath);

    // mode

    strcpy(buffer+buffer_len, mode);
    buffer_len += strlen(mode);

    memcpy(buffer+buffer_len, &end_string, 1);
    buffer_len++;

    fprintf(stdout, "%s ", mode);

    // block size

    strcpy(buffer+buffer_len, "blksize");
    buffer_len += strlen("blksize");

    memcpy(buffer+buffer_len, &end_string, 1);
    buffer_len++;

    for (int i = 0; i < strlen(o.blksize); i++) {
        
        convert_to_ASCII(o.blksize[i], c);
        // fprintf(stdout, "blksize - block: %d ascii: %s\n", i, c);
        // fprintf(stdout, "number: %c\n", convert_from_ASCII(c));
        strcpy(buffer+buffer_len, c);
        buffer_len += strlen(c);
    }

    memcpy(buffer+buffer_len, &end_string, 1);
    buffer_len++;

    fprintf(stdout, "blksize %s ", o.blksize);

    // timeout

    strcpy(buffer+buffer_len, "timeout");
    buffer_len += strlen("timeout");

    memcpy(buffer+buffer_len, &end_string, 1);
    buffer_len++;

    for (int i = 0; i < strlen(o.timeout); i++) {
        convert_to_ASCII(o.timeout[i], c);
        // fprintf(stdout, "timeout - block: %d ascii: %s\n", i, c);
        strcpy(buffer+buffer_len, c);
        buffer_len += strlen(c);
    }

    memcpy(buffer+buffer_len, &end_string, 1);
    buffer_len++;

    fprintf(stdout, "timeout %s ", o.timeout);

    // file's size

    strcpy(buffer+buffer_len, "tsize");
    buffer_len += strlen("tsize");

    memcpy(buffer+buffer_len, &end_string, 1);
    buffer_len++;

    for (int i = 0; i < strlen(o.tsize); i++) {
        convert_to_ASCII(o.tsize[i], c);
        // fprintf(stdout, "tsize - block: %d ascii: %s\n", i, c);
        strcpy(buffer+buffer_len, c);
        buffer_len += strlen(c);
    }

    memcpy(buffer+buffer_len, &end_string, 1);
    buffer_len++;

    fprintf(stdout, "tsize %s\n", o.tsize);

    if (buffer_len > 512) {
        fprintf(stderr, "Too long request packet.\n");
        exit(-1);
    }

    if (sendto(sock, buffer, buffer_len, MSG_CONFIRM, (const struct sockaddr *)&sockad, sizeof(sockad)) < 0) {
        exit(-1);
    }
}

void send_DATA(int sock, uint16_t packet_number, char* data, int len, struct sockaddr_in sockad) {
    int opcode = htons(DATA);
    char buffer[len];
    int buffer_len = 0;

    memcpy(buffer, &opcode, 2);
    buffer_len += 2;
    fprintf(stdout, "DATA %d ", packet_number);

    packet_number = htons(packet_number);
    memcpy(buffer+buffer_len, &packet_number, 2);
    buffer_len += 2;

    fprintf(stdout, "len_data: %ld\n", strlen(data));
    strcpy(buffer+buffer_len, data);
    buffer_len += strlen(data);


    if (sendto(sock, buffer, buffer_len, MSG_CONFIRM, (const struct sockaddr *)&sockad, sizeof(sockad)) < 0) {
        exit(2);
    }
}

void send_ACK(int sock, uint16_t packet_number, struct sockaddr_in sockad) {
    int opcode = htons(ACK);
    char buffer[PACKET_SIZE];
    int buffer_len = 0;

    memcpy(buffer, &opcode, 2);
    buffer_len += 2;

    fprintf(stdout, "ACK %d\n", packet_number);

    packet_number = htons(packet_number);
    memcpy(buffer+buffer_len, &packet_number, 2);
    buffer_len += 2;

    if (sendto(sock, buffer, buffer_len, MSG_CONFIRM, (const struct sockaddr *)&sockad, sizeof(sockad)) < 0) {
        exit(2);
    }
}

void send_ERR(int sock, uint16_t error_code, struct sockaddr_in sockad) {
    int opcode = htons(ERROR);
    char buffer[PACKET_SIZE];
    int buffer_len = 0;
    int end_string = 0;
    char error_msg[PACKET_SIZE-4];

    memcpy(buffer, &opcode, 2);
    buffer_len += 2;

    fprintf(stdout, "ERR ");

    fprintf(stdout, "%d ", error_code);

    switch (error_code) {
        case 0:
            strcpy(error_msg, "Not defined.\n");
            break;
        case 1:
            strcpy(error_msg, "File not found.\n");
            break;
        case 2:
            strcpy(error_msg, "Access violation.\n");
            break;
        case 3:
            strcpy(error_msg, "Disk full or allocation exceeded.\n");
            break;
        case 4:
            strcpy(error_msg, "Illegal TFTP operation.\n");
            break;
        case 5:
            strcpy(error_msg, "Unknown transfer ID.\n");
            break;
        case 6:
            strcpy(error_msg, "File already exists.\n");
            break;
        case 7:
            strcpy(error_msg, "No such user.\n");
            break;
        case 8:
            strcpy(error_msg, "Transfer should be terminated due to option negotiation.\n");
            break;
    }

	error_code = htons(error_code);

    memcpy(buffer+buffer_len, &error_code, 2);
    buffer_len += 2;

    strcpy(buffer+buffer_len, error_msg);
    buffer_len += strlen(error_msg);

    fprintf(stdout, "%s\n", error_msg);

    memcpy(buffer+buffer_len, &end_string, 1);
    buffer_len++;

    if (sendto(sock, buffer, buffer_len, MSG_CONFIRM, (const struct sockaddr *)&sockad, sizeof(sockad)) < 0) {
        exit(2);
    }
}

void send_OACK(int sock, struct opts o, struct sockaddr_in sockad) {
    char buffer[1024];
    int buffer_len = 0;
    uint16_t opcode = htons(OACK);
    uint8_t end_string = 0;
    int i;
    int n;
    char c[2];

    // opcode

    memcpy(buffer, &opcode, 2);
    buffer_len += 2;

    fprintf(stdout, "OACK ");

    // block size

    if (strcmp(o.blksize, "-1") != 0) {
        strcpy(buffer+buffer_len, "blksize");
        buffer_len += strlen("blksize");

        memcpy(buffer+buffer_len, &end_string, 1);
        buffer_len++;

        for (int i = 0; i < strlen(o.blksize); i++) {
            convert_to_ASCII(o.blksize[i], c);
            // fprintf(stdout, "blksize - block: %d ascii: %s\n", i, c);
            // fprintf(stdout, "number: %c\n", convert_from_ASCII(c));
            strcpy(buffer+buffer_len, c);
            buffer_len += strlen(c);
        }

        memcpy(buffer+buffer_len, &end_string, 1);
        buffer_len++;

        fprintf(stdout, "blksize %s ", o.blksize);
    }

    // timeout

    if (strcmp(o.timeout, "-1") != 0) {
        strcpy(buffer+buffer_len, "timeout");
        buffer_len += strlen("timeout");

        memcpy(buffer+buffer_len, &end_string, 1);
        buffer_len++;

        for (int i = 0; i < strlen(o.timeout); i++) {
            convert_to_ASCII(o.timeout[i], c);
            // fprintf(stdout, "timeout - block: %d ascii: %s\n", i, c);
            strcpy(buffer+buffer_len, c);
            buffer_len += strlen(c);
        }

        memcpy(buffer+buffer_len, &end_string, 1);
        buffer_len++;

        fprintf(stdout, "timeout %s ", o.timeout);
    }

    // file's size

    if (strcmp(o.tsize, "-1") != 0) {
        strcpy(buffer+buffer_len, "tsize");
        buffer_len += strlen("tsize");

        memcpy(buffer+buffer_len, &end_string, 1);
        buffer_len++;

        for (int i = 0; i < strlen(o.tsize); i++) {
            convert_to_ASCII(o.tsize[i], c);
            // fprintf(stdout, "tsize - block: %d ascii: %s\n", i, c);
            strcpy(buffer+buffer_len, c);
            buffer_len += strlen(c);
        }

        memcpy(buffer+buffer_len, &end_string, 1);
        buffer_len++;

        fprintf(stdout, "tsize %s\n", o.tsize);
    }

    if (sendto(sock, buffer, buffer_len, MSG_CONFIRM, (const struct sockaddr *)&sockad, sizeof(sockad)) < 0) {
        exit(-1);
    }
}