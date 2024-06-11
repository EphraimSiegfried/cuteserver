#include "response.h"
#include "../deps/log/log.h"
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFFER_LEN 80000

int send_ok_buf(int socket_fd, char **content_buffer, char *mime, long size) {
    char response_buffer[BUFFER_LEN];
    sprintf(response_buffer, "HTTP/1.1 200 OK\nConnection: keep-alive\nContent-Length: %ld\nContent-Type: %s\n\n%s", size, mime, *content_buffer);

    send(socket_fd, response_buffer, strlen(response_buffer), 0);
    log_info("Sending buffer over socket: %d", socket_fd);
    return 1;
}
int send_error(int socket_fd, short unsigned int type) {
    char content[102];
    char response[102 + 2 + 79];
    short unsigned int content_len; 
    sprintf(content, "\n\n<html><head>\n<title>Error %hu</title>\n</head><body>\n<img src=https://http.cat/%hu /></body></html>\n", type, type);
    content_len = strlen(content);
    sprintf(response, "HTTP/1.1 %hu\nContent-Length: %hu\nConnection: close\nContent-Type: text/html\n\n%s", type, content_len, content);
    long response_len = strlen(response);
    write(socket_fd, response, response_len);
    log_error("Sending error: %d", type);

    sleep(1); //TODO: sleep necessary ? 
    return 1;
}

int send_response(int socket_fd, response_info *res_i, char *request_body) {
    // send request_body
    send(socket_fd, request_body, strlen(request_body), 0);

    return 1;
}

//takes open file descriptor. fd has to be closed outside of this function
int send_file(int socket_fd, int file_fd) {
    int num_bytes;
    char buffer[BUFFER_LEN];
    /* send file in 8KB block - last block may be smaller */
    while ((num_bytes = read(file_fd, buffer, BUFFER_LEN)) > 0) {
        write(socket_fd, buffer, num_bytes);
    }
    return 1;
}

int send_request_info(int socket_fd, response_info *res_i) {
    const char *key;
    const char *value;
    char response_line[BUFFER_LEN];
    char *header_line = malloc(sizeof(char) * 100);

    // send request line
    sprintf(response_line, "%s %d %s\n", res_i->version, res_i->status_code, res_i->status_msg);
    send(socket_fd, response_line, strlen(response_line), 0);

    // send headers
    sc_map_foreach(&res_i->headers, key, value) {
        sprintf(header_line, "%s:%s\n", key, value);
        send(socket_fd, header_line, strlen(header_line), 0);
    }
    send(socket_fd, "\n", 1, 0);
    free(header_line);
    return 1;
}
