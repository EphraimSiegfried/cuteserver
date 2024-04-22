#include "log.h"
#include "response.h"
#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_BUF_SIZE 8122
#define MAX_PENDING 21

void *serve(void *client_sock) {
    int *client_socket = client_sock;
    char buff[MAX_BUF_SIZE];
    int ret = read(*client_socket, buff, sizeof(buff));
    int request_line_indices[3];
    int index = 0;
    //TODO: error handling for reading from buffer
    for (size_t i = 0; i < ret; i++) {
        // printf("%c", buff[i]);
        if (buff[i] == ' ') {
            request_line_indices[index] = i;
            index++;
        }
        if (buff[i] == '\r') {
            request_line_indices[index] = i;
            break;
        }
    }
    log_info("%*.*s", request_line_indices[2], request_line_indices[2], buff);
    if (strncmp(buff, "GET", request_line_indices[0]) == 0) {
        char file_path[MAX_BUF_SIZE];
        int file_path_len = request_line_indices[1] - request_line_indices[0];
        strncpy(file_path, &buff[request_line_indices[0] + 1], file_path_len - 1);// +1 because of whitespace
        printf("%s", file_path);
        fflush(stdout);
        if (strlen(file_path) == 1 && strncmp(file_path, "/", 1) == 0) {
            sprintf(file_path, "%s", "/index.html");
        }
        printf("\nFILE PATH: %s, LENGTH: %lu", file_path, strlen(file_path));
        fflush(stdout);
        if (access(file_path, F_OK) != 0) {
            printf("\naccess not granted\n");
            send_error(*client_socket, 404);
            return NULL;
        }
        send_ok(*client_socket, file_path);//TODO: handle other mime types
    }
    // TODO: hashmap info = parse_request(*buff)
    return NULL;
}

int main(void) {

    if (chroot("./data") < 0) {
        log_error("Chroot error: %s", strerror(errno));
    }
    // SERVER SOCKET
    struct sockaddr_in server_addr;

    // PF_INET= ipv4, SOCK_STREAM=tcp
    int server_sock = socket(PF_INET, SOCK_STREAM, 0);
    log_info("Server has started");

    server_addr.sin_family = AF_INET;
    // htons= host to network short
    server_addr.sin_port = htons(8891);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (bind(server_sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        log_fatal("Binding error: %d", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (listen(server_sock, MAX_PENDING) < 0) {
        log_fatal("Listening error %d", strerror(errno));
        exit(EXIT_FAILURE);
    }

    while (1) {
        struct sockaddr_in client_addr;
        unsigned int client_addr_len = sizeof(client_addr);
        int client_sock;
        client_sock =
                accept(server_sock, (struct sockaddr *) &client_addr, &client_addr_len);

        log_info("New connection accepted from %s:%d", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        if (client_sock < 0) {
            log_fatal("Accepting Error: %d", strerror(errno));
            exit(EXIT_FAILURE);
        }

        pthread_t client_handler;
        pthread_create(&client_handler, NULL, serve, (void *) &client_sock);
    }
}
