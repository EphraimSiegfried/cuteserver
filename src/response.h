#ifndef WEBSERVER_RESPONSE_H
#define WEBSERVER_RESPONSE_H

int send_ok(int socket_fd, char *file_path);

int send_ok_buf(int socket_fd, char *content_buffer, char *mime, long size);
int send_error(int socket_fd, int type);

#endif//WEBSERVER_RESPONSE_H
