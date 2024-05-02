#ifndef WEBSERVER_RESPONSE_H
#define WEBSERVER_RESPONSE_H

#include "../deps/hashmap/sc_map.h"
typedef struct {
    char *req_type;
    int status_code;
    char *status_msg;
    char *version;
    struct sc_map_str headers;
} response_info;
int send_ok(int socket_fd, char *file_path);
int send_ok_buf(int socket_fd, char **content_buffer, char *mime, long size);
int send_error(int socket_fd, short unsigned int type);
int send_response(int socket_fd, response_info *response_i, char *request_body);

#endif//WEBSERVER_RESPONSE_H
