#ifndef SHELL_H
#define SHELL_H

#define BUFFER_SIZE 1024
#define MAX_TOKENS 15

size_t tokenizer(char *buffer, char *argv[], size_t argv_size);

#endif
