
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


char *get_mime_type(char *file_path);
void set_default_headers(request_info *req_i, response_info *response_i);

struct {
    char *ext;
    char *mime;
} extensions[] = {
        {"gif", "image/gif"},
        {"jpg", "image/jpg"},
        {"jpeg", "image/jpeg"},
        {"JPG", "image/jpeg"},
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
        {"svg", "image/svg+xml"},
        {0, 0}};


int handle_dynamic_request(int *sock, request_info *req_i) {
    char *cgi_output = NULL;
    int cgi_output_len = run_cgi_script(req_i, &cgi_output);

    response_info response_i;

    log_info("output: %s", cgi_output);

    response_i.status_msg = "Ok";
    response_i.status_code = 200;
    response_i.version = "HTTP/1.1";
    response_i.req_type = req_i->req_type;
    sc_map_init_str(&response_i.headers, 0, 0);
    sc_map_put_str(&response_i.headers, "Content-Type", "text/html");//default value, gets overwritten by cgi-script
    set_default_headers(req_i, &response_i);
    int hdr_len = parse_headers(cgi_output, &response_i.headers);
    log_info("cgi_output: %d und hdr_len: %d", cgi_output_len, hdr_len);
    char *content = cgi_output + hdr_len;
    char str[20];
    sprintf(str, "%d", cgi_output_len - hdr_len);
    sc_map_put_str(&response_i.headers, "Content-Length", str);
    log_info("CONTENT LENGTH: %s", str);
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
    set_default_headers(req_i, &response_i);

    send_request_info(*client_socket, &response_i);

    if (is_get) {
        send_file(*client_socket, file_fd);
    }
    close(file_fd);

    return 1;
}

char *get_mime_type(char *file_path) {
    char *extension_p = strrchr(file_path, '.') + 1;
    for (int i = 0; extensions[i].ext != 0; i++) {
        if (strcmp(extension_p, extensions[i].ext) == 0) {
            return extensions[i].mime;
        }
    }
    return "/du failsch ";
}
void set_default_headers(request_info *req_i, response_info *response_i) {
    const char *connection = sc_map_get_str(&req_i->headers, "Connection");

    if (connection && strcasecmp(connection, "keep-alive") != 0) {
        sc_map_put_str(&response_i->headers, "Connection", "close");
    } else {
        sc_map_put_str(&response_i->headers, "Connection", "Keep-Alive");
        sc_map_put_str(&response_i->headers, "Keep-Alive", "timeout=3");
    }
}
