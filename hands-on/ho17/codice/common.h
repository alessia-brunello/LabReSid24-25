#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stddef.h>
#include <errno.h>

void die(const char* message);

void write_exact(int fd, const void* buffer, size_t size);

void read_exact(int fd, void* buffer, size_t size);

#endif
