#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include "response.h"

#define MAX_BUF_SIZE 8122
#define MAX_PENDING 21

void *serve (void *client_sock){
    int* client_socket = client_sock;
    char buff[MAX_BUF_SIZE];
    int ret = read(*client_socket, buff, sizeof(buff));
    //TODO: error handling for reading from buffer
    printf("%s", buff);
    //hashmap info = parse_request(*buff)
    fflush(stdout);
    //TODO:Check if the request is a valid http request
    send_ok(*client_socket,"/index.html","text/html");
    close(*client_socket);
    return NULL;
}

int main(void) {

    if (chroot("./data") < 0){
        perror("Error");
    }
    // SERVER SOCKET
    struct sockaddr_in server_addr;

    // PF_INET= ipv4, SOCK_STREAM=tcp
    int server_sock = socket(PF_INET, SOCK_STREAM, 0);
    printf("Server has started\n");

    server_addr.sin_family = AF_INET;
    // htons= host to network short
    server_addr.sin_port = htons(8890);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (bind(server_sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        printf("Couldn't connect to the port");
        fflush(stdout);
        exit(EXIT_FAILURE);
    }
    if (listen(server_sock, MAX_PENDING) < 0) {
        printf("Error while listening");
        fflush(stdout);
        exit(EXIT_FAILURE);
    }

    while(1) {
        struct sockaddr_in client_addr;
        unsigned int client_addr_len = sizeof(client_addr);
        int client_sock;
        client_sock =
                accept(server_sock, (struct sockaddr *) &client_addr, &client_addr_len);

        printf("\nconnected to %d\n", client_addr.sin_addr.s_addr);
        fflush(stdout);

        if (client_sock < 0) {
            printf("failed to accept");
            exit(EXIT_FAILURE);
        }

        pthread_t client_handler;
        pthread_create(&client_handler, NULL, serve, (void *)&client_sock);
    }
}
