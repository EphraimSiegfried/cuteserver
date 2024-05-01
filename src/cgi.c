#include "parser.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int set_env(request_info req_i) {
    char *env_variables[17];
    env_variables[0] = "SERVER_SOFTWARE=cuteserver/0.1";
    env_variables[1] = "SERVER_NAME=localhost";// TODO: aus config
    env_variables[2] = "GATEWAY_INTERFACE=CGI/1.1";
    sprintf(env_variables[3], "SERVER_PROTOCOL=%s", req_i.version);
    env_variables[4] = "SERVER_PORT=8888";// TODO: aus config
    sprintf(env_variables[5], "REQUEST_METHOD=%s", req_i.type);
    env_variables[5] = "REQUEST_METHOD=%s";
    env_variables[6] = "PATH_INFO=CGI/1.1";
    env_variables[7] = "PATH_TRANSLATED=CGI/1.1";
    env_variables[8] = "SCRIPT_NAME=CGI/1.1";
    env_variables[9] = "QUERY_STRING=CGI/1.1";
    env_variables[10] = "REMOTE_HOST=CGI/1.1";
    env_variables[11] = "REMOTE_ADDR=CGI/1.1";
    env_variables[12] = "AUTH_TYPE=CGI/1.1";
    env_variables[13] = "REMOTE_USER=CGI/1.1";
    env_variables[14] = "REMOTE_IDENT=CGI/1.1";
    env_variables[15] = "CONTENT_TYPE=CGI/1.1";
    env_variables[16] = "CONTENT_LENGTH=CGI/1.1";


    return 1;
}
int run_cgi_script() {
    return 1;
}

int main() {
    char *arguments[3];
    arguments[0] = "./cgi";
    arguments[1] = "23";
    arguments[2] = NULL;
    char *env_variables[2];
    env_variables[0] = "QUERY_STRING=GET";
    env_variables[1] = "REMOTE_HOST=POPO";
    env_variables[1] = "REMOTE_ADDR=192.212.12.1";

    int fd[2];
    if (pipe(fd) < 0) {
        printf("Pipe error");
        return -1;
    }

    pid_t pid = fork();

    if (pid == 0) {
        // Child
        close(fd[0]);              // Close unused read end
        dup2(fd[1], STDOUT_FILENO);// Redirect stdout to pipe
        close(fd[1]);              // Close the original write end of the pipe

        // Execute command that generates output
        if (execve("./cgi", arguments, env_variables) < 0) {
            printf("Error: %s\n", strerror(errno));
        }
        perror("execlp");// Only runs if execlp fails
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // This is the parent process
        char buffer[1024];// Buffer to store output
        int status;
        close(fd[1]);// Close unused write end

        // Read the output from the child process
        ssize_t count = read(fd[0], buffer, sizeof(buffer) - 1);
        if (count == -1) {
            perror("read");
            exit(EXIT_FAILURE);
        }
        buffer[count] = '\0';// Null-terminate the string

        // Wait for the child to finish
        waitpid(pid, &status, 0);

        // Print the output from the child process
        printf("Output from child: %s\n", buffer);
        close(fd[0]);// Close the read end
    } else {
        printf("Error");
        return -1;
    }


    printf("here i am");
    fflush(stdout);
}
