#ifndef WEBSERVER_PARSER_H
#define WEBSERVER_PARSER_H

#define TODO_SIZE 6342
#include "../deps/hashmap/sc_map.h"

enum req_type {
    GET,
    POST,
    PUT
};

typedef struct {
    enum req_type type;
    char file_path[TODO_SIZE];
    char version[TODO_SIZE];
    struct sc_map_str headers;
} request_info;

int parse_request_line(char *buff, int buf_length, request_info *req_info);

int parse_headers(char *buf, request_info *req_info);
#endif//WEBSERVER_PARSER_H
