#include "logger.h"
#include "utils.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

Logger *logger_create(const char* filename) {
    Logger *logger = (Logger *)malloc(sizeof(Logger));
    if (logger == NULL) {
        perror("Failed to initialize logger");
        return NULL;
    }
    char filepath[256] = "logs/";
    strcat(filepath, filename);
    FILE *file = fopen(filepath, "w");
    if (file == NULL) {
        perror("Failed to create log file");
        return NULL;
    }
    logger->log_file = file;
    logger->start_time = now_seconds();
    return logger;
}
void logger_free(Logger *logger) {
    fclose(logger->log_file);
    free(logger);
}

void logger_message(Logger *logger, const char* tag, const char* message) {
    if (logger == NULL || message == NULL) return;
    double current_time = now_seconds() - logger->start_time;
    if (logger->log_file != NULL) {
        fprintf(logger->log_file, "[%f] [%s] %s\n", current_time, tag, message);
        fflush(logger->log_file);
    } else {
        printf("[%f] [%s] %s\n", current_time, tag, message);
    }
}
