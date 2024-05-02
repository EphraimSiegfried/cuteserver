
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
    char *cgi_output = malloc(sizeof(char) * 5000);
    if (cgi_output == NULL) return -1;
    int cgi_output_len = run_cgi_script(req_i, &cgi_output);

    int hdr_len = parse_headers(cgi_output, &req_i);
    char *content = cgi_output + hdr_len;

    response_info response_i;

    response_i.status_msg = "Ok";
    response_i.status_code = 200;
    response_i.version = "HTTP/1.1";
    response_i.req_type = req_i.req_type;
    sc_map_init_str(&response_i.headers, 0, 0);
    //TODO:
    char str[20];
    sprintf(str, "%d", cgi_output_len - hdr_len);
    sc_map_put_str(&response_i.headers, "Content-Length", str);
    sc_map_put_str(&response_i.headers, "Content-Type", "text/html");
    sc_map_put_str(&response_i.headers, "Connection", "keep-alive");
    send_response(*sock, &response_i, content);

    free(cgi_output);
    return 1;
}

int handle_get_request(int *sock, request_info req_i) {
    char *real_path = req_i.real_path;
    log_debug("%s", real_path);
    if (access(real_path, F_OK) != 0) {
        log_error("File not found: ", req_i.file_path);
        send_error(*sock, NOTFOUND);
        return -1;
    }
    if (strcmp(req_i.real_path, "./data/cgi-bin/cgi") == 0) {
        handle_cgi_request(sock, req_i);
    }

    // send_ok(*sock, real_path);// TODO: handle other mime types
    return 1;
}
