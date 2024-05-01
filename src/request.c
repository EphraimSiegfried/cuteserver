#include "request.h"
#include "../deps/log/log.h"
#include "cgi_handler.h"
#include "parser.h"
#include "response.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int handle_cgi_request(int *sock, request_info req_i) {
    char *buffer;
    int len = run_cgi_script(req_i, &buffer);
    send_ok_buf(*sock, buffer, "text/html", len);
    return 1;
}

int handle_get_request(int *sock, request_info req_i) {
    log_debug("FILE PATH: %s", req_i.file_path);
    if (strlen(req_i.file_path) == 1 && strcmp(req_i.file_path, "/") == 0) {
        sprintf(req_i.file_path, "%s", "/index.html");
    }
    if (access(req_i.file_path, F_OK) != 0) {
        log_error("File not found: ", req_i.file_path);
        send_error(*sock, NOTFOUND);
        return -1;
    }
    if (strcmp(req_i.file_path, "./data/cgi-bin/cgi") == 0) {
        handle_cgi_request(sock, req_i);
    }

    send_ok(*sock, req_i.file_path);// TODO: handle other mime types
    return 1;
}
