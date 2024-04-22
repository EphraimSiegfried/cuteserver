#include <string.h>
#include "parser.h"
#include "log.h"

void *parse_request(char *buff, int buf_length, request_info * req_i) {
    log_debug("parsing");
    int request_line_indices[3];
    int index = 0;
    //TODO: error handling for reading from buffer
    for (int i = 0; i < buf_length; i++) {
        if (buff[i] == ' ') { //TODO: refactor
            request_line_indices[index] = i;
            index++;
        }
        if (buff[i] == '\r') {
            request_line_indices[index] = i;
            break;
        }
    }
    log_debug("yooo: %d, %d, %d", request_line_indices[0], request_line_indices[1], request_line_indices[2]);
    log_info("%*.*s", request_line_indices[2], request_line_indices[2], buff);
    strncpy(req_i->type, &buff[0], buff[request_line_indices[0]]); 
    int file_path_len = request_line_indices[1] - request_line_indices[0] - 1;
    log_debug("filepath lenght: %d", file_path_len);
    strncpy(req_i->file_path, &buff[request_line_indices[0] + 1], file_path_len); 
    // strncpy(req_i->rest, &buff[request_line_indices[1] + 1], buff[buf_length] - buff[request_line_indices[1] - 1]); 
    log_debug("REQ: %s", req_i->type); 
    log_debug("FILEPATH: %s", req_i->file_path); 
    return 0;
}
