#include "parser.h"
#include "../deps/hashmap/sc_map.h"
#include "../deps/log/log.h"
#include "config.h"
#include "utils.h"
#include <string.h>

int parse_headers(char *buf, struct sc_map_str *header_map) {
    const char *key, *value;
    char *temp_buf = buf;
    char *line;
    while ((line = strsep(&temp_buf, "\n"))) {//TODO: find better implementation
        if (strcmp(line, "") == 0 || strcmp(line, " ") == 0 || strcmp(line, "\r") == 0 || strcmp(line, "\r\n") == 0) break;
        if (!(key = strtok(line, ": ")) || !(value = trim(strtok(NULL, ": ")))) return -1;
        sc_map_put_str(header_map, key, value);
    }
    // const char *keyy, *valuee;
    // sc_map_foreach(header_map, keyy, valuee) {
    //     log_debug("KEY: [%s] | VALUE: [%s]", keyy, valuee);
    // }
    return temp_buf - buf;
}

int parse_request_line(char *buff, int buf_length, request_info *req_i) {
    char *original_buff = buff;

    req_i->req_type = trim(strsep(&buff, " "));
    char *url = trim(strsep(&buff, " "));
    log_debug("URL:%s", url);
    req_i->file_path = trim(strsep(&url, "?"));
    req_i->query = strsep(&url, " ");//kei whitespace...
    log_debug("FILE_PATH: %s", req_i->file_path);
    log_debug("QUERY: %s", req_i->query);

    //resolve real path
    char *real_path = malloc(sizeof(char) * 1024);
    const char *mapped_path = sc_map_get_str(&conf->resources[0].remaps, req_i->file_path);
    sprintf(real_path, "%s%s", conf->resources[0].root, mapped_path ? mapped_path : req_i->file_path);//TODO: what if query is in file path
    req_i->real_path = real_path;
    log_debug("REAL PATH: %s", real_path);

    req_i->version = trim(strsep(&buff, "\n"));

    if (!req_i->req_type || !req_i->file_path || !req_i->version) return -1;
    log_info("%s %s %s", req_i->req_type, req_i->file_path, req_i->version);

    return buff - original_buff;//TODO: return error if this number is too big
}

// int resolve_path(request_info *req_i) {
//     if (req_i->file_path == NULL) return NULL;
//     char real_path[1024];
//     strcpy(real_path, conf->resources[0].root);
//     const char *mapped_path = sc_map_get_str(&conf->resources[0].remaps, req_i->file_path);
//     if (mapped_path == NULL) return NULL;
//     strcat(real_path, mapped_path);
// }
