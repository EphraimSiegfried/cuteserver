#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#define BUFFER_LEN 65536

#define ERROR      42
#define LOG        44
#define FORBIDDEN 403
#define NOTFOUND  404

int send_ok(int socket_fd, char * file_path, char * mime){
    int file_fd;
    long file_len, num_bytes;
    char buffer[BUFFER_LEN];
    if((file_fd = open(file_path,O_RDONLY)) == -1) { //open file
        printf("Could not open file\n");
    }
    file_len = (long)lseek(file_fd, (off_t)0, SEEK_END); // lseek to the file end to find the length of the file
    lseek(file_fd, (off_t)0, SEEK_SET); // seek back to the file start ready for reading
    sprintf(buffer,"HTTP/1.1 200 OK\nContent-Length: %ld\nConnection: close\nContent-Type: %s\n\n", file_len, mime); // Header + a blank line
    write(socket_fd,buffer,strlen(buffer));

    /* send file in 8KB block - last block may be smaller */
    while (	(num_bytes = read(file_fd, buffer, BUFFER_LEN)) > 0 ) {
        write(socket_fd,buffer,num_bytes);
    }
    sleep(1);	// allow socket to drain before signalling the socket is closed
    close(file_fd);
    close(socket_fd);
}

int send_error(int socket_fd, int type) {
    char response[BUFFER_LEN];
    char message[BUFFER_LEN];
    char err_name[BUFFER_LEN];
    char content[BUFFER_LEN];
    int content_len; //TODO
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
    sprintf(content, "\n\n<html><head>\n<title>%d %s</title>\n</head><body>\n<h1>%s</h1>\n%s\n</body></html>\n",type, err_name, err_name, message);
    content_len = strlen(content);
    sprintf(response, "HTTP/1.1 %d %s\nContent-Length: %d\nConnection: close\nContent-Type: text/html\n\n%s",type,err_name, content_len, content);
    printf("PRINTING RESPONSE !!!!!!%s", response);
    long response_len = strlen(response);
    write(socket_fd, response,response_len);
    sleep(1);	// allow socket to drain before signalling the socket is closed
    close(socket_fd);
}