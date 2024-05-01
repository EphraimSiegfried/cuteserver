#ifndef CGI_HANDLER_H
#define CGI_HANDLER_H

#include "parser.h"

int run_cgi_script(request_info req_i);
int set_env(char *env_variables[], request_info req_i);

#endif//CGI_HANDLER_H
