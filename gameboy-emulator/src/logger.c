// logger.c - Simple logging functionality for the Game Boy emulator
#include "logger.h"
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>  // For strlen function

// File handle for the log file
static FILE* log_file = NULL;

// Initialize the logger
bool logger_init(const char* filename) {
    if (log_file) {
        // Already initialized
        return true;
    }
    
    log_file = fopen(filename, "w");
    if (!log_file) {
        return false;
    }
    
    // Write initial timestamp
    time_t now = time(NULL);
    fprintf(log_file, "=== Log started at %s", ctime(&now));
    fflush(log_file);
    
    return true;
}

// Write a log message
void logger_log(LogLevel level, const char* format, ...) {
    if (!log_file) {
        return;
    }
    
    // Get current time
    time_t rawtime;
    struct tm* timeinfo;
    char timestamp[9]; // HH:MM:SS\0
    
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(timestamp, sizeof(timestamp), "%H:%M:%S", timeinfo);
    
    // Log level as string
    const char* level_str;
    switch (level) {
        case LOG_DEBUG:
            level_str = "DEBUG";
            break;
        case LOG_INFO:
            level_str = "INFO";
            break;
        case LOG_WARNING:
            level_str = "WARNING";
            break;
        case LOG_ERROR:
            level_str = "ERROR";
            break;
        default:
            level_str = "UNKNOWN";
            break;
    }
    
    // Print timestamp and level
    fprintf(log_file, "[%s] [%s] ", timestamp, level_str);
    
    // Print formatted message
    va_list args;
    va_start(args, format);
    vfprintf(log_file, format, args);
    va_end(args);
    
    // Add newline if not already there
    size_t format_len = strlen(format);
    if (format_len > 0 && format[format_len - 1] != '\n') {
        fprintf(log_file, "\n");
    }
    
    // Ensure it's written to disk
    fflush(log_file);
}

// Close the logger
void logger_close() {
    if (log_file) {
        time_t now = time(NULL);
        fprintf(log_file, "=== Log ended at %s", ctime(&now));
        fclose(log_file);
        log_file = NULL;
    }
}
