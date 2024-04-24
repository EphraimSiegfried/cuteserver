#include "parser.h"
#include "../deps/hashmap/sc_map.h"
#include "../deps/log/log.h"
#include "utils.h"
#include <string.h>

int parse_headers(char *buf, request_info *req_i) {
    struct sc_map_str map;
    const char *key, *value;
    sc_map_init_str(&map, 0, 0);
    char *temp_buf = buf;
    char *line;
    while ((line = strsep(&temp_buf, "\n")) != NULL) {//TODO: find better implementation
        if (strcmp(line, "") == 0 || strcmp(line, " ") == 0 || strcmp(line, "\r") == 0) continue;
        if (((key = strtok(line, ": ")) == NULL) || ((value = trim(strtok(NULL, ": "))) == NULL)) return -1;
        sc_map_put_str(&map, key, value);
    }
    req_i->headers = map;
    // sc_map_foreach(&map, key, value) {
    //     log_debug("KEY: [%s] | VALUE: [%s]", key, value);
    // }
    return 1;
}

int parse_request_line(char *buff, int buf_length, request_info *req_i) {
    int request_line_indices[3];
    int index = 0;
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
    return request_line_indices[2] + 2;
}
