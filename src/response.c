#include "log.h"
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/errno.h>
#include <unistd.h>


#define BUFFER_LEN 65536

#define ERROR 42
#define LOG 44
#define FORBIDDEN 403
#define NOTFOUND 404

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
    file_len = (long) lseek(file_fd, (off_t) 0, SEEK_END);                                                           // lseek to the file end to find the length of the file
    lseek(file_fd, (off_t) 0, SEEK_SET);                                                                             // seek back to the file start ready for reading
    sprintf(buffer, "HTTP/1.1 200 OK\nContent-Length: %ld\nConnection: close\nContent-Type: %s\n\n", file_len, mime);// Header + a blank line
    write(socket_fd, buffer, strlen(buffer));

    /* send file in 8KB block - last block may be smaller */
    while ((num_bytes = read(file_fd, buffer, BUFFER_LEN)) > 0) {
        write(socket_fd, buffer, num_bytes);
    }
    log_info("Sending %s", file_path);
    sleep(1);// allow socket to drain before signalling the socket is closed
    close(file_fd);
    close(socket_fd);
    return 1;
}

int send_error(int socket_fd, int type) {
    char response[BUFFER_LEN];
    char message[BUFFER_LEN];
    char err_name[BUFFER_LEN];
    char content[BUFFER_LEN];
    int content_len;//TODO
    switch (type) {
        case FORBIDDEN:
            sprintf(err_name, "Forbidden");
            sprintf(message, "The requested URL, file type or operation is not allowed on this simple static file webserver.");
            break;
        case NOTFOUND:
            sprintf(err_name, "Not Found");
            sprintf(message, "The requested URL was not found on this server.");
            break;
    }
    sprintf(content, "\n\n<html><head>\n<title>%d %s</title>\n</head><body>\n<h1>%s</h1>\n%s\n</body></html>\n", type, err_name, err_name, message);
    content_len = strlen(content);
    sprintf(response, "HTTP/1.1 %d %s\nContenti-Length: %d\nConnection: close\nContent-Type: text/html\n\n%s", type, err_name, content_len, content);
    long response_len = strlen(response);
    write(socket_fd, response, response_len);
    log_error("Sending error: %s %d", err_name, type);

    sleep(1);// allow socket to drain before signalling the socket is closed
    close(socket_fd);
    return 1;
}
