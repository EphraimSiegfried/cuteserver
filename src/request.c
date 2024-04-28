#include "request.h"
#include "../deps/log/log.h"
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
        log_error("File not found: ", req_i.file_path);
        send_error(*sock, NOTFOUND);
        return -1;
    }
    send_ok(*sock, req_i.file_path);// TODO: handle other mime types
    return 1;
}

int handle_post_request(int *sock, request_info req_i) {

    // find script corresponding to req_i->path
    // chöme das iwie imne andere file speichere ?
    // also demet me cha zb imne json (weiss ned ob gschid) path: /gugus -> script: guguscript


    // set environment variables for the script

    // execute the script

    // wait for it to finish & read from the standard output of the script

    // compose html message and respond to client
    return 1;
}
