#include "config.h"
#include "../deps/log/log.h"
#include "../deps/tomlparser/toml.h"
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


int parse_config(char *path) {
    FILE *fp;
    char errbuf[200];

    // 1. Read and parse toml file
    fp = fopen(path, "r");
    if (!fp) {
        log_error("cannot open config file: ", strerror(errno));
    }
    toml_table_t *toml = toml_parse_file(fp, errbuf, sizeof(errbuf));
    fclose(fp);

    if (!toml) {
        log_error("cannot parse - ", errbuf);
    }

    toml_table_t *server = toml_table_in(toml, "server");
    if (!server) handle_error("server");

    conf->port = get_int("port", server);
    conf->workers = get_int("workers", server);

    toml_table_t *resource = toml_table_in(toml, "resource");
    if (!resource) handle_error("cute resource");

    int num_resources = 1;// Assuming at least one resource for now

    // Allocate memory for the resources array
    conf->resources = malloc(num_resources * 2000);//TODO: free

    // TODO: Dynamic reading of resource tables. ressource table has to be >=1

    toml_table_t *cute = toml_table_in(resource, "cute");
    if (!cute) handle_error("cute resource");

    conf->resources[0].domain = get_str("domain", cute);
    conf->resources[0].root = get_str("root", cute);
    conf->resources[0].cgi_bin_dir = get_str("cgi_bin_dir", cute);
    // conf->resources[0].root = get_str("root", cute);

    toml_table_t *remaps = toml_table_in(cute, "remaps");
    if (!remaps) handle_error("cute remaps");
    // conf->resources[0].remaps;
    struct sc_map_str remaps_map;
    sc_map_init_str(&remaps_map, 0, 0);
    //add default remapping / to /index.html
    sc_map_put_str(&remaps_map, "/", "/index.html");
    for (int i = 0;; i++) {
        const char *key = toml_key_in(remaps, i);
        if (!key) break;
        sc_map_put_str(&remaps_map, key, toml_string_in(remaps, key).u.s);
    }
    conf->resources[0].remaps = remaps_map;

    // const char *key;
    // const char *value;
    // sc_map_foreach(&conf->resources[0].remaps, key, value) {
    //     printf("Key:[%s], Value:[%s] \n", key, value);
    // }
    return 1;
}
