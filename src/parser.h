#ifndef WEBSERVER_PARSER_H
#define WEBSERVER_PARSER_H

#include <netinet/in.h>
#include "../deps/hashmap/sc_map.h"

typedef struct {
    char *req_type;
    char *file_path;
    char *query;
    char *real_path;
    char *version;
    struct sockaddr_in client_addr;
    struct sc_map_str headers;
    char *request_body;
} request_info;

int parse_request_line(char *buff, int buf_length, request_info *req_info);
int parse_headers(char *buf, struct sc_map_str *header_map);
int resolve_real_path(request_info *req_i);

request_info *allocate_request_info();
void free_request_info(request_info *req_i);
#endif//WEBSERVER_PARSER_H
