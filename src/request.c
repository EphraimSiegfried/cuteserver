
#include "request.h"
#include "../deps/log/log.h"
#include "cgi_handler.h"
#include "parser.h"
#include "response.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

struct {
    char *ext;
    char *mime;
} extensions[] = {
        {"gif", "image/gif"},
        {"jpg", "image/jpg"},
        {"jpeg", "image/jpeg"},
        {"png", "image/png"},
        {"ico", "image/ico"},
        {"zip", "image/zip"},
        {"gz", "image/gz"},
        {"tar", "image/tar"},
        {"htm", "text/html"},
        {"html", "text/html"},
        {"ico", "image/x-icon"},
        {"css", "text/css"},
        {"js", "text/javascript"},
        {0, 0}};

char *get_mime_type(char *file_path) {
    char *extension_p = strrchr(file_path, '.') + 1;
    for (int i = 0; extensions[i].ext != 0; i++) {
        if (strcmp(extension_p, extensions[i].ext) == 0) {
            return extensions[i].mime;
        }
    }
    return "/du failsch ";
}

int handle_dynamic_request(int *sock, request_info *req_i) {
    char *cgi_output = malloc(sizeof(char) * 5000);
    if (cgi_output == NULL) return -1;
    int cgi_output_len = run_cgi_script(req_i, &cgi_output);

    int hdr_len = parse_headers(cgi_output, req_i);
    char *content = cgi_output + hdr_len;

    response_info response_i;

    response_i.status_msg = "Ok";
    response_i.status_code = 200;
    response_i.version = "HTTP/1.1";
    response_i.req_type = req_i->req_type;
    sc_map_init_str(&response_i.headers, 0, 0);
    //TODO:
    char str[20];
    sprintf(str, "%d", cgi_output_len - hdr_len);
    sc_map_put_str(&response_i.headers, "Content-Length", str);
    sc_map_put_str(&response_i.headers, "Content-Type", "text/html");//TODO: check memory allocation
    sc_map_put_str(&response_i.headers, "Connection", "keep-alive");
    send_request_info(*sock, &response_i);
    send(*sock, content, cgi_output_len - hdr_len, 0);

    free(cgi_output);
    return 1;
}

int handle_static_request(int *client_socket, request_info *req_i) {

    bool is_get = strcmp(req_i->req_type, "GET") == 0;
    bool is_head = strcmp(req_i->req_type, "HEAD") == 0;

    if (!is_get && !is_head) {
        log_error("Not a valid request type %s: %s", req_i->req_type, strerror(errno));
        send_error(*client_socket, NOTFOUND);
        return -1;
    }
    response_info response_i;
    long file_len;
    int file_fd;

    if ((file_fd = open(req_i->real_path, O_RDONLY)) == -1) {//open file
        send_error(*client_socket, SERVERERROR);
        log_fatal("Error opening %s: %s", req_i->file_path, strerror(errno));
        return -1;
    }
    file_len = (long) lseek(file_fd, (off_t) 0, SEEK_END);// lseek to the file end to find the length of the file
    lseek(file_fd, (off_t) 0, SEEK_SET);                  // seek back to the file start ready for reading


    response_i.status_msg = "Ok";
    response_i.status_code = 200;
    response_i.version = "HTTP/1.1";
    response_i.req_type = req_i->req_type;
    sc_map_init_str(&response_i.headers, 0, 0);
    char file_len_str[20];
    sprintf(file_len_str, "%ld", file_len);
    sc_map_put_str(&response_i.headers, "Content-Length", file_len_str);
    sc_map_put_str(&response_i.headers, "Content-Type", get_mime_type(req_i->real_path));
    sc_map_put_str(&response_i.headers, "Connection", "keep-alive");

    send_request_info(*client_socket, &response_i);

    if (is_get) {
        send_file(*client_socket, file_fd);
    }
    close(file_fd);
    log_debug("hello");

    return 1;
}
