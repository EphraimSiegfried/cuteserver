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
    while ((line = strsep(&temp_buf, "\n"))) {//TODO: find better implementation
        if (strcmp(line, "") == 0 || strcmp(line, " ") == 0 || strcmp(line, "\r") == 0) continue;
        if (!(key = strtok(line, ": ")) || !(value = trim(strtok(NULL, ": ")))) return -1;
        sc_map_put_str(&map, key, value);
    }
    req_i->headers = map;
    // sc_map_foreach(&map, key, value) {
    //     log_debug("KEY: [%s] | VALUE: [%s]", key, value);
    // }
    return 1;
}

int parse_request_line(char *buff, int buf_length, request_info *req_i) {
    char *original_buff = buff;

    char *typ = trim(strsep(&buff, " "));
    req_i->file_path = trim(strsep(&buff, " "));
    req_i->version = trim(strsep(&buff, "\n"));

    if (!typ || !req_i->file_path || !req_i->version) return -1;
    log_info("%s %s %s", typ, req_i->file_path, req_i->version);

    if (strcmp(typ, "GET") == 0) {
        req_i->type = GET;
    } else if (strcmp(&buff[0], "PUT") == 0) {
        req_i->type = PUT;
    } else if (strcmp(&buff[0], "POST") == 0) {
        req_i->type = POST;
    } else {
        return -1;
    }
    return buff - original_buff;//TODO: return error if this number is too big
}
