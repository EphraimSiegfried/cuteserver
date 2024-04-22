#ifndef WEBSERVER_REQUEST_H 
#define WEBSERVER_REQUEST_H

// typedef struct request_info {
//     char * type; 
//     char * file_path; 
//     char * rest;
// }request_info;

void handle_request(int* indices, char * buff, int ret); 
// void handle_get_request(char *char); 

#endif//WEBSERVER_REQUEST_H


