#include "../deps/hashmap/sc_map.h"
#include "../deps/log/log.h"
#include "../deps/tomlparser/toml.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char *root;
    char *logs;
    char *cgi_bin_dir;
    struct sc_map_str remaps;
} resource;

typedef struct {
    int workers;
    int port;
    char *domain;
    resource resources[];

} config;

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

    config conf;

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

    conf.port = get_int("port", server);
    conf.workers = get_int("workers", server);


    toml_table_t *resource = toml_table_in(toml, "resource");
    if (!resource) handle_error("cute resource");

    // TODO: Dynamic reading of resource tables. ressource table has to be >=1

    toml_table_t *cute = toml_table_in(resource, "cute");
    if (!cute) handle_error("cute resource");
    printf("%s", get_str("domain", cute));

    return 1;
}


int main() {
    parse_config("../config.toml");
}
