#ifndef PIPE_H
#define PIPE_H

#include "defs.h"

void pipe_init_system(void);
int pipe_create(int fds[2]); /* fds[0]=read end, fds[1]=write end */
int pipe_write(int fd, const char *buf, int count);
int pipe_read(int fd, char *buf, int count);
int pipe_close(int fd);
int pipe_is_pipe(int fd);

#endif
