#include "parser.h"
#include "../deps/log/log.h"
#include <string.h>


int parse_request(char *buff, int buf_length, request_info *req_i) {
    int request_line_indices[3];
    int index = 0;
    log_debug("parse request");
    //TODO: error handling for reading from buffer
    for (int i = 0; i < buf_length; i++) {
        if (buff[i] == ' ') {//TODO: refactor
            buff[i] = '\0';
            request_line_indices[index] = i;
            index++;
        }
        if (buff[i] == '\r') {
            buff[i] = '\0';
            request_line_indices[index] = i;
            break;
        }
    }
    strcpy(req_i->file_path, &buff[request_line_indices[0] + 1]);
    strcpy(req_i->version, &buff[request_line_indices[1] + 1]);
    log_info("%s %s %s", &buff[0], req_i->file_path, req_i->version);
    //TODO rest of buffer
    if (strcmp(&buff[0], "GET") == 0) {
        req_i->type = GET;
    } else if (strcmp(&buff[0], "PUT") == 0) {
        req_i->type = PUT;
    } else if (strcmp(&buff[0], "POST") == 0) {
        req_i->type = POST;
    } else {
        return -1;
    }
    return 1;
}
