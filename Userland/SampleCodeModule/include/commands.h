#ifndef COMMANDS_H
#define COMMANDS_H

#include <stdint.h>

uint64_t cmd_help(uint64_t argc, char *argv[]);
uint64_t cmd_mem(uint64_t argc, char *argv[]);
uint64_t cmd_ps(uint64_t argc, char *argv[]);
uint64_t cmd_loop(uint64_t argc, char *argv[]);
uint64_t cmd_kill(uint64_t argc, char *argv[]);
uint64_t cmd_nice(uint64_t argc, char *argv[]);
uint64_t cmd_block(uint64_t argc, char *argv[]);
uint64_t cmd_cat(uint64_t argc, char *argv[]);
uint64_t cmd_wc(uint64_t argc, char *argv[]);
uint64_t cmd_filter(uint64_t argc, char *argv[]);
uint64_t cmd_mvar(uint64_t argc, char *argv[]);
uint64_t cmd_clear(uint64_t argc, char *argv[]);

#endif
