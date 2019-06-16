#ifndef LOG_H
#define LOG_H

int openLog();

void writeLog(const char *_fmt, ...);

void closeLog();

#endif
