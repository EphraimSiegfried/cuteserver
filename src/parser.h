#ifndef WEBSERVER_PARSER_H
#define WEBSERVER_PARSER_H

#define TODO_SIZE 6342
#include "../deps/hashmap/sc_map.h"

#define FOREACH_TYPE(TYPE) \
    TYPE(GET)              \
    TYPE(POST)             \
    TYPE(PUT)

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

enum REQ_TYPE {
    FOREACH_TYPE(GENERATE_ENUM)
};

static const char *REQ_TYPE_STRING[] = {
        FOREACH_TYPE(GENERATE_STRING)};

typedef struct {
    enum REQ_TYPE type;
    char *file_path;
    char *version;
    struct sc_map_str headers;
} request_info;

int parse_request_line(char *buff, int buf_length, request_info *req_info);

int parse_headers(char *buf, request_info *req_info);
#endif//WEBSERVER_PARSER_H
