#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stddef.h>
#include <sys/types.h>

ssize_t recv_line(int fd, char* buffer, size_t max_len);
int send_all(int fd, const char* buffer, size_t len);
int send_line(int fd, const char *message);

#endif
