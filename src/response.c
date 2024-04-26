#include "../deps/log/log.h"
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/errno.h>
#include <unistd.h>


#define BUFFER_LEN 80000


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

int send_ok(int socket_fd, char *file_path) {
    int file_fd;
    long file_len, num_bytes;
    char buffer[BUFFER_LEN];
    char *mime = get_mime_type(file_path);
    if ((file_fd = open(file_path, O_RDONLY)) == -1) {//open file
        log_fatal("Error opening %s: %s", file_path, strerror(errno));
        return -1;
    }
    file_len = (long) lseek(file_fd, (off_t) 0, SEEK_END);                                                                // lseek to the file end to find the length of the file
    lseek(file_fd, (off_t) 0, SEEK_SET);                                                                                  // seek back to the file start ready for reading
    sprintf(buffer, "HTTP/1.1 200 OK\nConnection: keep-alive\nContent-Length: %ld\nContent-Type: %s\n\n", file_len, mime);// Header + a blank line
    write(socket_fd, buffer, strlen(buffer));

    /* send file in 8KB block - last block may be smaller */
    while ((num_bytes = read(file_fd, buffer, BUFFER_LEN)) > 0) {
        write(socket_fd, buffer, num_bytes);
    }
    log_info("Sending %s over socket: %d", file_path, socket_fd);
    sleep(1);
    close(file_fd);
    return 1;
}

int send_error(int socket_fd, short unsigned int type) {
    char content[102];
    char response[102 + 2 + 79];
    short unsigned int content_len;//TODO
    sprintf(content, "\n\n<html><head>\n<title>Error %hu</title>\n</head><body>\n<img src=https://http.cat/%hu /></body></html>\n", type, type);
    content_len = strlen(content);
    sprintf(response, "HTTP/1.1 %hu\nContent-Length: %hu\nConnection: close\nContent-Type: text/html\n\n%s", type, content_len, content);
    long response_len = strlen(response);
    write(socket_fd, response, response_len);
    log_error("Sending error: %d", type);

    sleep(1);
    return 1;
}
