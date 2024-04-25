#include "../deps/log/log.h"
#include "parser.h"
#include "request.h"
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
    const char *connection;
    int recvd_bytes, rl_len, hdr_len;
    bool keep_alive = true;

    do {
        memset(buff, 0, sizeof(buff));
        request_info req_i = {0};

        if ((recvd_bytes = recv(*client_socket, buff, sizeof(buff), 0)) <= 0) {
            log_error("%s", strerror(errno));
            break;
        }

        if ((rl_len = parse_request_line(buff, recvd_bytes, &req_i)) < 0) {
            send_error(*client_socket, BADREQUEST);
            break;
        }

        if ((hdr_len = parse_headers(buff + rl_len, &req_i)) < 0) {
            send_error(*client_socket, BADREQUEST);
            break;
        }

        if (strcmp(req_i.version, "HTTP/1.1") != 0) {
            log_error("Not supported:", req_i.version);
            send_error(*client_socket, VERSIONNOTSUPPORTED);
            break;
        }

        switch (req_i.type) {
            case GET:
                handle_get_request(client_sock, req_i);
                break;
            case PUT:
            case POST:
                // Handle other types as necessary
                break;
        }

        connection = sc_map_get_str(&req_i.headers, "Connection");
        if (connection && strcasecmp(connection, "keep-alive") != 0) {
            keep_alive = false;
        }
        log_debug("Client sent: %s", connection);

    } while (keep_alive);

    close(*client_socket);
    return NULL;
}

int main(int argv, char *args[]) {

    int port = argv <= 1 ? 8888 : atoi(args[1]);

    if (chroot("./data") < 0) {
        log_error("Chroot error: %s", strerror(errno));
    }
    // SERVER SOCKET
    struct sockaddr_in server_addr;

    // PF_INET= ipv4, SOCK_STREAM=tcp
    int server_sock = socket(PF_INET, SOCK_STREAM, 0);

    server_addr.sin_family = AF_INET;
    // htons= host to network short
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");


    if (bind(server_sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        log_fatal("Binding error: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    log_info("Server listening on %s:%d", inet_ntoa(server_addr.sin_addr), port);

    if (listen(server_sock, MAX_PENDING) < 0) {
        log_fatal("Listening error %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    while (1) {
        struct sockaddr_in client_addr;
        unsigned int client_addr_len = sizeof(client_addr);
        int client_sock;
        client_sock = accept(server_sock, (struct sockaddr *) &client_addr, &client_addr_len);
        int optval = 1;
        log_info("New connection accepted from %s:%d", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        if (client_sock < 0) {
            log_fatal("Accepting Error: %d", strerror(errno));
            exit(EXIT_FAILURE);
        }

        pthread_t client_handler;
        pthread_create(&client_handler, NULL, serve, (void *) &client_sock);
    }
}
