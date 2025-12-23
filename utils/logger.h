#ifndef LOGGING_H
#define LOGGING_H

#include <stdio.h>
#include <stdbool.h>

typedef struct {
    double start_time;
    FILE *log_file;
    bool is_file_open;
} Logger;

Logger *create_logger();
void close_logger(Logger *logger);

FILE *create_log_file(const char* filename);
void set_channel(Logger *logger, FILE *channel, bool is_file);

void logMessage(Logger* channel, const char* tag, const char* message);

#endif