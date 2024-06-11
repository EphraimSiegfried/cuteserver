#include "../deps/log/log.h"
#include "../deps/threadpool/thpool.h"
#include "config.h"
#include "parser.h"
#include "request.h"
#include "response.h"
#include "utils.h"
#include <arpa/inet.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/socket.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_BUF_SIZE 8122
#define MAX_PENDING 21

typedef struct {
    int client_socket;
    struct sockaddr_in client_address;
} client_socket_info;


void serve(void *client_info) {
    client_socket_info *client_sock_i = (client_socket_info *) client_info;
    int *client_socket = &client_sock_i->client_socket;
    char buff[MAX_BUF_SIZE];
    const char *connection;
    int recvd_bytes, rl_len, hdr_len;
    bool keep_alive = true;
    struct timeval tv;

    // We tell the client that the socket is open for 5 sec
    // but we make it actually a little longer, 7 sec
    tv.tv_sec = 7;
    tv.tv_usec = 0;
    setsockopt(*client_socket, SOL_SOCKET, SO_RCVTIMEO, (const char *) &tv, sizeof tv);

    do {
        memset(buff, 0, sizeof(buff));

        request_info *req_i = allocate_request_info();
        if (req_i == NULL) {
            log_error("Memory allocation failed for request_info");
            break;
        }
        req_i->client_addr = client_sock_i->client_address;

        if ((recvd_bytes = recv(*client_socket, buff, sizeof(buff), 0)) < 0) {
            log_error("%s %d", strerror(errno), *client_socket);
            free_request_info(req_i);
            break;
        }
        if (!recvd_bytes) {
            free_request_info(req_i);// Free allocated memory
            break;                   // Client has closed the connection
        }

        if ((rl_len = parse_request_line(buff, recvd_bytes, req_i)) < 0) {
            send_error(*client_socket, BADREQUEST);
            free_request_info(req_i);
            break;
        }

        sc_map_init_str(&req_i->headers, 0, 0);// Initialize headers map
        if ((hdr_len = parse_headers(buff + rl_len, &req_i->headers)) < 0) {
            send_error(*client_socket, BADREQUEST);
            free_request_info(req_i);
            break;
        }

        req_i->request_body = buff + hdr_len + rl_len;

        resolve_real_path(req_i);

        if (access(req_i->real_path, F_OK) != 0) {
            log_warn("File not found: %s", req_i->file_path);
            send_error(*client_socket, NOTFOUND);
            free_request_info(req_i);
            break;
        }

        if (strcmp(req_i->version, "HTTP/1.1") != 0) {
            send_error(*client_socket, VERSIONNOTSUPPORTED);
            free_request_info(req_i);
            break;
        }

        connection = sc_map_get_str(&req_i->headers, "Connection");
        if (connection && strcasecmp(connection, "keep-alive") != 0) {
            keep_alive = false;
        }

        int return_val;
        if (ends_with("cgi", req_i->real_path)) {
            return_val = handle_dynamic_request(client_socket, req_i);
        } else {
            return_val = handle_static_request(client_socket, req_i);
        }
        if (return_val < 0) {
            free_request_info(req_i);
            break;
        }

        // Free request_info before next iteration if keep_alive is true
        free_request_info(req_i);

    } while (keep_alive);
    close(*client_socket);
    return;
}

int server_sock;
threadpool thpool;
void cleanup() {
    log_debug("Exiting...cleaning up");
    close(server_sock);
    thpool_destroy(thpool);
    cleanup_config();
    log_debug("Finished cleanup");
    exit(0);
}

int main(int argc, char *argv[]) {
    // HANDLE COMMAND LINE ARGUMENTS
    if (argc > 9) {
        fprintf(stderr, "Too many Arguments.\nUsage: %s [-a address] [-p port] [-l log_level] [-c config_path]\nDefault values: -a 127.0.0.1 -p 8888 -l 0 -c ./config.toml\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int opt;
    char *config_path = "./config.toml";
    struct sockaddr_in server_addr;
    int server_addr_valid = 0;
    int port = 0;
    int log_level = 0;
    while ((opt = getopt(argc, argv, "a:p:l:c:")) != -1) {
        switch (opt) {
            case 'a':                                                                     //address, default value = 127.0.0.1
                if ((server_addr_valid = inet_aton(optarg, &server_addr.sin_addr) == 0)) {//returns 0 on error
                    printf("Invalid address value.\n");
                    return -1;
                }
                server_addr_valid = 1;
                break;
            case 'p':// port, default value = 8888
                port = atoi(optarg);
                if (port == 0) {
                    printf("Invalid port value.\n");
                    return -1;
                }
                break;
            case 'l':// log-level, default value = 0
                log_level = atoi(optarg);
                if (log_level > 5 || log_level < 0) {
                    printf("Invalid log level value. Must be between 0 and 5\n");
                    return -1;
                }
                break;
            case 'c':// config path, default value = ./config.toml
                config_path = optarg;
                break;
            default:
                fprintf(stderr, "Usage: %s [-a address] [-p port] [-l log_level] [-c config_path]\nDefault values: -a 127.0.0.1 -p 8888 -l 0 -c ./config.toml\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }
    if (access(config_path, F_OK) == -1) {
        printf("Configuration file does not exist: %s\n", config_path);
        return -1;
    }
    parse_config(config_path);
    server_addr.sin_addr.s_addr = (server_addr_valid == 0) ? (conf->address ? inet_addr(conf->address) : inet_addr("127.0.0.1")) : server_addr.sin_addr.s_addr;
    port = (port == 0) ? (conf->port ? conf->port : 8888) : port;//NOTE: order is argument > config > default value
    log_set_level(log_level);
    FILE *fp = fopen(conf->log_file, "a");//TODO: close at the end
    if (!fp) {
        log_error("cannot open config file %s: %s", conf->log_file, strerror(errno));
    }
    log_add_fp(fp, 0);

    log_debug("Address: %s Port=%d, Log Level: %d, Config Path: %s\n", inet_ntoa(server_addr.sin_addr), port, log_level, config_path);

    // SERVER SOCKET
    // PF_INET= ipv4, SOCK_STREAM=tcp
    server_sock = socket(PF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    // htons= host to network short
    server_addr.sin_port = htons(port);

    if (bind(server_sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        log_fatal("Binding error: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    log_info("Server listening on %s:%d", inet_ntoa(server_addr.sin_addr), port);

    if (listen(server_sock, MAX_PENDING) < 0) {
        log_fatal("Listening error %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    thpool = thpool_init(conf->workers);

    while (1) {
        client_socket_info *client_sock_i = malloc(sizeof(client_socket_info));
        unsigned int client_addr_len = sizeof(client_sock_i->client_address);
        client_sock_i->client_socket = accept(server_sock, (struct sockaddr *) &client_sock_i->client_address, &client_addr_len);

        if (client_sock_i->client_socket < 0) {
            log_fatal("Accepting Error: %d", strerror(errno));
            exit(EXIT_FAILURE);
        }
        log_info("New connection accepted from %s:%d", inet_ntoa(client_sock_i->client_address.sin_addr), ntohs(client_sock_i->client_address.sin_port));

        thpool_add_work(thpool, serve, client_sock_i);
    }
}
