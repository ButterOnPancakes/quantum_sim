#ifndef LOGGING_H
#define LOGGING_H

#include <stdio.h>
#include <stdbool.h>

typedef struct {
    double start_time;
    FILE *log_file;
} Logger;

Logger *logger_create(const char* filename);
void logger_free(Logger *logger);

void logger_message(Logger* channel, const char* tag, const char* message);

#endif