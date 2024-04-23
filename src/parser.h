#ifndef WEBSERVER_PARSER_H
#define WEBSERVER_PARSER_H

#define TODO_SIZE 6342

enum req_type {
    GET, 
    POST, 
    PUT
};

typedef struct {
    enum req_type type;
    char file_path[TODO_SIZE]; 
    char version[TODO_SIZE]; 
    char rest[TODO_SIZE]; // TODO: unpack rest of buffer 
} request_info;

int parse_request(char *buff, int buf_length, request_info * req_info);

#endif //WEBSERVER_PARSER_H
