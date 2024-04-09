#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_BUF_SIZE 8122

int main(void) {
    // PF_INET= ipv4, SOCK_STREAM=tcp
    int server_sock = socket(PF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    // htons= host to network short
    server_addr.sin_port = htons(8129);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (bind(server_sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        printf("Couldn't connect to the port");
        exit(EXIT_FAILURE);
    }
    if (listen(server_sock, 1) < 0) {
        printf("Error while listening");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in client_addr;
    unsigned int client_addr_len = sizeof(client_addr);
    int client_sock;
    client_sock =
            accept(server_sock, (struct sockaddr *) &client_addr, &client_addr_len);

    printf("connected to %d", client_addr.sin_addr.s_addr);

    if (client_sock < 0) {
        printf("failed to accept");
        exit(EXIT_FAILURE);
    }

    char buff[MAX_BUF_SIZE];

    while (1) {
        bzero(buff, MAX_BUF_SIZE);
        read(client_sock, buff, sizeof(buff));
        printf("%s", buff);
    }

    close(client_sock);
    close(server_sock);
}
