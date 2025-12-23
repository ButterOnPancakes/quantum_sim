#include "logger.h"
#include "utils.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

Logger *create_logger() {
    Logger *logger = (Logger *)malloc(sizeof(Logger));
    if (logger == NULL) {
        perror("Failed to initialize logger");
        return NULL;
    }
    logger->log_file = NULL;
    logger->is_file_open = false;
    logger->start_time = now_seconds();
    return logger;
}
void close_logger(Logger *logger) {
    if (logger != NULL && logger->is_file_open && logger->log_file != NULL) {
        fclose(logger->log_file);
    }
    logger->is_file_open = false;
    logger->log_file = NULL;
}

FILE *create_log_file(const char* filename) {
    char filepath[256] = "logs/";
    strcat(filepath, filename);
    FILE *file = fopen(filepath, "w");
    if (file == NULL) {
        perror("Failed to create log file");
        return NULL;
    }
    return file;
}
void set_channel(Logger *logger, FILE *channel, bool is_file) {
    if (logger == NULL) return;
    if (logger->is_file_open && logger->log_file != NULL) {
        fclose(logger->log_file);
    }
    logger->log_file = channel;
    logger->is_file_open = is_file;
}

void logMessage(Logger *logger, const char* tag, const char* message) {
    if (logger == NULL || message == NULL) return;
    double current_time = now_seconds() - logger->start_time;
    if (logger->log_file != NULL) {
        fprintf(logger->log_file, "[%f] [%s] %s\n", current_time, tag, message);
        fflush(logger->log_file);
    } else {
        printf("[%f] [%s] %s\n", current_time, tag, message);
    }
}
