#include "parser.h"
#include "../deps/hashmap/sc_map.h"
#include "../deps/log/log.h"
#include "config.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>

int parse_headers(char *buf, struct sc_map_str *header_map) {
    const char *key, *value;
    char *temp_buf = buf;
    char *line;
    while ((line = strsep(&temp_buf, "\n"))) {
        if (strcmp(line, "") == 0 || strcmp(line, " ") == 0 || strcmp(line, "\r") == 0 || strcmp(line, "\r\n") == 0) break;
        key = trim(strsep(&line, ": "));
        value = trim(line);
        if (!key || !value) return -1;
        sc_map_put_str(header_map, key, value);
    }

    return temp_buf - buf;
}

int parse_request_line(char *buff, int buf_length, request_info *req_i) {
    char *original_buff = buff;

    req_i->req_type = trim(strsep(&buff, " "));
    char *url = trim(strsep(&buff, " "));
    req_i->file_path = trim(strsep(&url, "?"));
    req_i->query = strsep(&url, " ");

    // resolve real path
    char *real_path = malloc(sizeof(char) * 1024);
    const char *mapped_path = sc_map_get_str(&conf->resources[0].remaps, req_i->file_path);
    sprintf(real_path, "%s%s", conf->resources[0].root, mapped_path ? mapped_path : req_i->file_path);
    req_i->real_path = real_path;

    req_i->version = trim(strsep(&buff, "\n"));

    if (!req_i->req_type || !req_i->file_path || !req_i->version) return -1;
    log_debug("Request: %s File Path: %s Real Path: %s HTTP Version: %s QUERY: %s",
              req_i->req_type, req_i->file_path, req_i->real_path, req_i->version, req_i->query);

    return buff - original_buff;
}

int resolve_real_path(request_info *req_i) {
    const char *domain = sc_map_get_str(&req_i->headers, "Host");
    if (!domain) return 0;
    uint64_t id = sc_map_get_s64(&conf->identifier, domain);
    if (!sc_map_found(&conf->identifier)) return -1;
    const char *mapped_path = sc_map_get_str(&conf->resources[id].remaps, req_i->file_path);
    sprintf(req_i->real_path, "%s%s", conf->resources[id].root, mapped_path ? mapped_path : req_i->file_path);
    return 1;
}
