#ifndef PIPE_H
#define PIPE_H

#include "defs.h"

void pipe_init_module(void);
int pipe_create(fd_t fds[2]);
int pipe_open(const char *name, fd_t fds[2]);
int pipe_write(fd_t fd, const char *buf, int count);
int pipe_read(fd_t fd, char *buf, int count);
int pipe_close(fd_t fd);

#endif
