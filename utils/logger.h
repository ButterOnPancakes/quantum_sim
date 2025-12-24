#ifndef LOGGING_H
#define LOGGING_H

#include <stdio.h>
#include <stdbool.h>

typedef struct {
    double start_time;
    FILE *log_file;
    bool is_file_open;
} Logger;

Logger *logger_create();
void logger_free(Logger *logger);

FILE *create_log_file(const char* filename);
void logger_set_channel(Logger *logger, FILE *channel, bool is_file);

void logger_message(Logger* channel, const char* tag, const char* message);

#endif