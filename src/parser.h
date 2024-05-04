#ifndef WEBSERVER_PARSER_H
#define WEBSERVER_PARSER_H

#include <netinet/in.h>
#define TODO_SIZE 6342
#include "../deps/hashmap/sc_map.h"

typedef struct {
    char *req_type;
    char *file_path;
    char *query;
    char *real_path;
    char *version;
    struct sockaddr_in client_addr;
    struct sc_map_str headers;
} request_info;

int parse_request_line(char *buff, int buf_length, request_info *req_info);

int parse_headers(char *buf, struct sc_map_str *header_map);
#endif//WEBSERVER_PARSER_H
