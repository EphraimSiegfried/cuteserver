#ifndef CONFIG_H
#define CONFIG_H

#include "../deps/hashmap/sc_map.h"

typedef struct {
    char *domain;
    char *root;
    char *logs;
    char *cgi_bin_dir;
    struct sc_map_str remaps;
} resource;

typedef struct {
    int workers;
    int port;
    resource resources[];

} config;
int parse_config(config *toml_conf, char *path);

#endif//CONFIG_H