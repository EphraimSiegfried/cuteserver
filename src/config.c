#include "config.h"
#include "../deps/log/log.h"
#include "../deps/tomlparser/toml.h"
#include "hashmap/sc_map.h"
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void handle_error(char *missing_field) {
    log_error("Missing %s", missing_field);
    exit(1);
}

int get_int(char *field, toml_table_t *tbl) {
    toml_datum_t data = toml_int_in(tbl, field);
    if (!data.ok) handle_error(field);
    return data.u.i;
}
char *get_str(char *field, toml_table_t *tbl) {
    toml_datum_t data = toml_string_in(tbl, field);
    if (!data.ok) handle_error(field);
    return data.u.s;
}

config *conf = NULL;

int parse_config(char *path) {
    FILE *fp;
    char errbuf[200];

    // 1. Read and parse toml file
    fp = fopen(path, "r");
    if (!fp) {
        log_error("cannot open config file %s: %s", path, strerror(errno));
    }
    toml_table_t *toml = toml_parse_file(fp, errbuf, sizeof(errbuf));
    fclose(fp);

    if (!toml) {
        log_error("cannot parse - ", errbuf);
    }

    toml_table_t *server = toml_table_in(toml, "server");
    if (!server) handle_error("server");

    conf = malloc(sizeof(config));//TODO:free
    conf->address = get_str("address", server); 
    conf->port = get_int("port", server);
    conf->workers = get_int("workers", server);
    conf->log_file = get_str("log_file", server); 

    toml_table_t *resources = toml_table_in(toml, "resource");
    if (!resources) handle_error("cute resource");

    int num_resources = toml_table_ntab(resources);
    if (num_resources < 1) handle_error("Must contain at least one configuration for a resource");

    // Allocate memory for the resources array
    conf->resources = malloc(num_resources * sizeof(resource));//TODO: free

    struct sc_map_s64 identifier;
    sc_map_init_s64(&identifier, 0, 0);
    for (int j = 0;; j++) {
        const char *name_id = toml_key_in(resources, j);
        if (!name_id) break;
        toml_table_t *domain_config = toml_table_in(resources, name_id);
        if (!domain_config) handle_error("resource");

        char *domain = get_str("domain", domain_config);
        conf->resources[j].domain = domain;
        conf->resources[j].root = get_str("root", domain_config);
        conf->resources[j].cgi_bin_dir = get_str("cgi_bin_dir", domain_config);
        sc_map_put_s64(&identifier, domain, j);

        toml_table_t *remaps = toml_table_in(domain_config, "remaps");
        if (!remaps) handle_error("remaps");
        struct sc_map_str remaps_map;
        sc_map_init_str(&remaps_map, 0, 0);
        sc_map_put_str(&remaps_map, "/", "/index.html");
        for (int i = 0;; i++) {
            const char *key = toml_key_in(remaps, i);
            if (!key) break;
            sc_map_put_str(&remaps_map, key, toml_string_in(remaps, key).u.s);
        }
        conf->resources[j].remaps = remaps_map;
    }
    conf->identifier = identifier;

    // const char *key;
    // const char *value;
    // sc_map_foreach(&conf->resources[0].remaps, key, value) {
    //     printf("Key:[%s], Value:[%s] \n", key, value);
    // }
    return 1;
}

void cleanup_config() {
    free(conf->resources);
    free(conf);
}
