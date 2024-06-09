#include "cgi_handler.h"
#include "../deps/log/log.h"
#include <arpa/inet.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>


#define ENV_BUF_SIZE 300

int set_env(char *env_variables[], request_info *req_i) {
    char temp[ENV_BUF_SIZE];

    env_variables[0] = "SERVER_SOFTWARE=cuteserver/0.1";
    env_variables[1] = "SERVER_NAME=cuteserver";
    env_variables[2] = "GATEWAY_INTERFACE=CGI/1.1";
    sprintf(temp, "SERVER_PROTOCOL=%s", req_i->version);
    env_variables[3] = strdup(temp);
    sprintf(temp, "SERVER_PORT=%d", 8888);
    env_variables[4] = strdup(temp);
    sprintf(temp, "REQUEST_METHOD=%s", req_i->req_type);
    env_variables[5] = strdup(temp);
    sprintf(temp, "PATH_INFO=%s", req_i->file_path);
    env_variables[6] = strdup(temp);
    sprintf(temp, "PATH_TRANSLATED=%s", req_i->real_path);
    env_variables[7] = strdup(temp);
    char *script_name = strrchr(req_i->real_path, '/') + 1;
    sprintf(temp, "SCRIPT_NAME=%s", script_name);
    env_variables[8] = strdup(temp);
    sprintf(temp, "QUERY_STRING=%s", req_i->query);
    env_variables[9] = strdup(temp);
    char hostname[256]; 
    gethostname(hostname, sizeof(hostname));
    sprintf(temp, "REMOTE_HOST=%s", hostname);
    env_variables[10] = strdup(temp);
    sprintf(temp, "REMOTE_ADDR=%s", inet_ntoa(req_i->client_addr.sin_addr));
    env_variables[11] = strdup(temp);
    env_variables[12] = "AUTH_TYPE=NULL";
    env_variables[13] = "REMOTE_USER=NULL";
    env_variables[14] = "REMOTE_IDENT=NULL";
    sprintf(temp, "CONTENT_TYPE=%s", sc_map_get_str(&req_i->headers, "Content-Type"));
    env_variables[15] = strdup(temp);
    sprintf(temp, "CONTENT_LENGTH=%lu", strlen(req_i->request_body));
    env_variables[16] = strdup(temp);
    env_variables[17] = NULL;
    return 1;
}

int run_cgi_script(request_info *req_i, char **cgi_output) {
    ssize_t count;
    char *script_name = strrchr(req_i->real_path, '/') + 1;
    char *arguments[2] = {script_name, NULL};
    size_t buffer_size = 100;
    size_t total_bytes_read = 0;

    char *env_variables[18];
    set_env(env_variables, req_i);

    //stdin: parent -> child, stdout: child -> parent
    int stdin_pipe[2], stdout_pipe[2];//pipes have two ends (read & write)
    if (pipe(stdin_pipe) < 0 || pipe(stdout_pipe) < 0) {
        printf("Pipe error");
        return -1;
    }

    pid_t pid = fork();//create child process

    if (pid == 0) {
        // Child
        //close parent ends
        close(stdin_pipe[1]);
        close(stdout_pipe[0]);

        dup2(stdin_pipe[0], STDIN_FILENO);
        dup2(stdout_pipe[1], STDOUT_FILENO);
        close(stdin_pipe[0]);
        close(stdout_pipe[1]);

        // Execute command that generates output
        if (execve(req_i->real_path, arguments, env_variables) < 0) {
            printf("Error: %s\n", strerror(errno));
        }
    } else if (pid > 0) {
        // This is the parent process
        int status;

        write(stdin_pipe[1], req_i->request_body, strlen(req_i->request_body));
        close(stdin_pipe[1]);//not used anymore

        // Read the output from the child process
        *cgi_output = malloc(buffer_size);
        if (!cgi_output) {
            log_error("Malloc error: %s", strerror(errno));
            close(stdout_pipe[0]);
            return -1;
        }
        while ((count = read(stdout_pipe[0], *cgi_output + total_bytes_read, buffer_size - total_bytes_read)) > 0) {
            total_bytes_read += count;
            log_info("bytes read: %d", total_bytes_read);
            log_info("count: %d", count);
            // log_info("buffer: %s", *cgi_output);

            // If the buffer is full, increase its size
            if (total_bytes_read >= buffer_size) {
                buffer_size *= 2;// Double the buffer size
                char *new_cgi_output = realloc(*cgi_output, buffer_size);
                if (new_cgi_output == NULL) {
                    perror("realloc");
                    free(*cgi_output);
                    close(stdout_pipe[0]);
                    return -1;
                }
                *cgi_output = new_cgi_output;
            } else {
                break; //TODO: find better solution
            }
        }

        if (count == -1) {
            perror("read");
            free(*cgi_output);
            close(stdout_pipe[0]);
            return -1;
        }
        (*cgi_output)[total_bytes_read] = '\0';// Null-terminate the string
        close(stdout_pipe[0]);

        // Wait for the child to finish
        waitpid(pid, &status, 0);

    } else {
        printf("Error");
        return -1;
    }

    return total_bytes_read;
}
