#ifndef CONFIG_H
#define CONFIG_H

#include "../deps/hashmap/sc_map.h"
#include <netinet/in.h>

typedef struct {
    char *domain;
    char *root;
    char *cgi_bin_dir;
    struct sc_map_str remaps;
} resource;

typedef struct {
    int workers;
    in_addr_t address; 
    int port;
    resource *resources;
    char *log_file; 
    struct sc_map_s64 identifier;//maps domain names to resource indices
} config;

extern config *conf;
int parse_config(char *path);

void cleanup_config();

#endif//CONFIG_H
