#ifndef ROUTER_H
#define ROUTER_H

#include <stddef.h>

#include "http.h"

void route_request(const HttpRequest* request, char* response, size_t response_size);

#endif
