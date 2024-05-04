#include "cgi_handler.h"
#include <arpa/inet.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define ENV_BUF_SIZE 100

int set_env(char *env_variables[], request_info *req_i) {
    // const char *key;
    // const char *value;
    env_variables[0] = "SERVER_SOFTWARE=cuteserver/0.1";
    env_variables[1] = "SERVER_NAME=localhost";// TODO: aus config
    env_variables[2] = "GATEWAY_INTERFACE=CGI/1.1";
    env_variables[3] = malloc(ENV_BUF_SIZE);
    sprintf(env_variables[3], "SERVER_PROTOCOL=%s", req_i->version);
    env_variables[4] = malloc(ENV_BUF_SIZE);
    sprintf(env_variables[4], "SERVER_PORT=%d", 8888);
    env_variables[5] = malloc(ENV_BUF_SIZE);
    sprintf(env_variables[5], "REQUEST_METHOD=%s", req_i->req_type);
    env_variables[6] = malloc(ENV_BUF_SIZE);
    sprintf(env_variables[6], "PATH_INFO=%s", req_i->file_path);
    env_variables[7] = malloc(ENV_BUF_SIZE);
    sprintf(env_variables[7], "PATH_TRANSLATED=%s", req_i->real_path);
    env_variables[8] = malloc(ENV_BUF_SIZE);
    sprintf(env_variables[8], "SCRIPT_NAME=%s", "scriptname");//TODO: get script name from path
    env_variables[9] = malloc(ENV_BUF_SIZE);
    sprintf(env_variables[9], "QUERY_STRING=%s", req_i->query);
    env_variables[10] = "REMOTE_HOST=NULL";
    env_variables[11] = malloc(ENV_BUF_SIZE);
    sprintf(env_variables[11], "REMOTE_ADDR=%s", inet_ntoa(req_i->client_addr.sin_addr));//TODO:
    env_variables[12] = "AUTH_TYPE=NULL";
    env_variables[13] = "REMOTE_USER=NULL";
    env_variables[14] = "REMOTE_IDENT=NULL";
    env_variables[15] = "CONTENT_TYPE=text/html";//default value, has to be overridden by cgi script
    env_variables[16] = "CONTENT_LENGTH=NULL";   //TODO
    env_variables[17] = NULL;
    // size_t i = 17;
    // uint32_t size = sc_map_size_str(&req_i->headers);
    // sc_map_foreach(&req_i->headers, key, value) {
    //     sprintf(env_variables[i++], "HTTP_%s=%s", key, value);
    // }
    return 1;
}

int run_cgi_script(request_info *req_i, char *cgi_output[]) {
    ssize_t count;
    char *arguments[3];
    arguments[0] = "cgi";
    arguments[1] = "23";
    arguments[2] = NULL;

    char *env_variables[18];
    set_env(env_variables, req_i);

    int fd[2];//2 file descriptors, one for read, one for write
    if (pipe(fd) < 0) {
        printf("Pipe error");
        return -1;
    }

    // puts("hello");
    pid_t pid = fork();//create child process

    if (pid == 0) {
        // Child
        close(fd[0]);              // Close unused read end
        dup2(fd[1], STDOUT_FILENO);// Redirect stdout to pipe
        close(fd[1]);              // Close the original write end of the pipe
        // Execute command that generates output
        if (execve(req_i->real_path, arguments, env_variables) < 0) {
            printf("Error: %s\n", strerror(errno));
        }
        perror("execve");// Only runs if execlp fails
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // This is the parent process
        int status;
        close(fd[1]);// Close unused write end

        // Read the output from the child process
        count = read(fd[0], *cgi_output, 1999);//TODO:FIX MEMORY MANAGEMENT!!
        if (count == -1) {
            perror("read");
            exit(EXIT_FAILURE);
        }
        (*cgi_output)[count] = '\0';// Null-terminate the string

        // Wait for the child to finish
        waitpid(pid, &status, 0);

        close(fd[0]);// Close the read end
    } else {
        printf("Error");
        return -1;
    }


    return count;
}
