// logger.h - Simple logging functionality for the Game Boy emulator
#ifndef LOGGER_H
#define LOGGER_H

#include <stdbool.h>

// Log levels
typedef enum {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR
} LogLevel;

// Initialize the logger
bool logger_init(const char* filename);

// Write a log message
void logger_log(LogLevel level, const char* format, ...);

// Close the logger
void logger_close();

// Shorthand macros for different log levels
#define LOG_DEBUG(format, ...) logger_log(LOG_DEBUG, format, ##__VA_ARGS__)
#define LOG_INFO(format, ...) logger_log(LOG_INFO, format, ##__VA_ARGS__)
#define LOG_WARNING(format, ...) logger_log(LOG_WARNING, format, ##__VA_ARGS__)
#define LOG_ERROR(format, ...) logger_log(LOG_ERROR, format, ##__VA_ARGS__)

#endif // LOGGER_H
