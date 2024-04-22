
void handle_request(int* indices, char * buff, int ret) {
    struct request_info request_information = parse_request();

    if (strncmp(buff, "GET", request_line_indices[0]) == 0) {
        char file_path[MAX_BUF_SIZE];
        int file_path_len = request_line_indices[1] - request_line_indices[0];
        strncpy(file_path, &buff[request_line_indices[0] + 1], file_path_len - 1);// +1 because of whitespace
        if (strlen(file_path) == 1 && strncmp(file_path, "/", 1) == 0) {
            sprintf(file_path, "%s", "/index.html");
        }
        if (access(file_path, F_OK) != 0) {
            send_error(*client_socket, 404);
            return NULL;
        }
        send_ok(*client_socket, file_path);//TODO: handle other mime types
    } 
    else if () {

    }
    // TODO: hashmap info = parse_request(*buff)
    return NULL;
}

void get_request()
