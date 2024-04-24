#include "parser.h"
#include "response.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int handle_get_request(int *sock, request_info req_i) {
    if (strlen(req_i.file_path) == 1 && strcmp(req_i.file_path, "/") == 0) {
        sprintf(req_i.file_path, "%s", "/index.html");
    }
    if (access(req_i.file_path, F_OK) != 0) {
        send_error(*sock, 404);
        return -1;
    }
    send_ok(*sock, req_i.file_path);// TODO: handle other mime types
    return 1;
}
