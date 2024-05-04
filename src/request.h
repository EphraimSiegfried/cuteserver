#ifndef WEBSERVER_REQUEST_H
#define WEBSERVER_REQUEST_H


#include "parser.h"

#define ERROR 42
#define LOG 44
#define BADREQUEST 400
#define FORBIDDEN 403
#define NOTFOUND 404
#define VERSIONNOTSUPPORTED 505
#define SERVERERROR 500

int handle_static_request(int *client_socket, request_info *req_i);
int handle_dynamic_request(int *sock, request_info *req_i);

#endif//WEBSERVER_REQUEST_H
