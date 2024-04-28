#include "../deps/log/log.h"
#include "../deps/threadpool/thpool.h"
#include "config.h"
#include "parser.h"
#include "request.h"
#include "response.h"
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/socket.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_BUF_SIZE 8122
#define MAX_PENDING 21

config conf;
void serve(void *client_sock) {
    int *client_socket = client_sock;
    char buff[MAX_BUF_SIZE];
    const char *connection;
    int recvd_bytes, rl_len, hdr_len;
    bool keep_alive = true;

    do {
        memset(buff, 0, sizeof(buff));
        request_info req_i = {0};
        // log_debug("TID: %d sock: %d", syscall(__NR_gettid), *client_socket);
        log_debug(buff);

        if ((recvd_bytes = recv(*client_socket, buff, sizeof(buff), 0)) <= 0) {
            log_error("%s %d", strerror(errno), *client_socket);
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

        connection = sc_map_get_str(&req_i.headers, "Connection");
        if (connection && strcasecmp(connection, "keep-alive") != 0) {
            keep_alive = false;
        }
        log_debug("Client sent: %s", connection);

        switch (req_i.type) {
            case GET:
                handle_get_request(client_sock, req_i);
                break;
            case PUT:
            case POST:
                break;
        }

    } while (keep_alive);
    log_debug(" bye %d", *client_socket);
    close(*client_socket);
    return;
}

int main(int argv, char *args[]) {
    log_debug("hello from the other siiiiiideee");

    int port = args[1] ? atoi(args[1]) : 8888;
    int log_level = args[2] && atoi(args[2]) <= 5 ? atoi(args[2]) : 0;

    parse_config(&conf, "./config.toml");

    const char *key;
    const char *value;
    log_debug("%d", conf.port);

    sc_map_foreach(&conf.resources[0].remaps, key, value) {
        printf("Key:[%s], Value:[%s] \n", key, value);
    }

    log_set_level(log_level);

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

    threadpool thpool = thpool_init(8);

    while (1) {
        struct sockaddr_in client_addr;
        unsigned int client_addr_len = sizeof(client_addr);
        int *client_sock = malloc(sizeof(int));
        // log_debug("%d", thpool_num_threads_working(thpool));
        *client_sock = accept(server_sock, (struct sockaddr *) &client_addr, &client_addr_len);

        log_debug("Created sock: %d", *client_sock);
        if (*client_sock < 0) {
            log_fatal("Accepting Error: %d", strerror(errno));
            exit(EXIT_FAILURE);
        }
        log_info("New connection accepted from %s:%d", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        // setsockopt(*client_sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

        thpool_add_work(thpool, serve, client_sock);
    }
}
