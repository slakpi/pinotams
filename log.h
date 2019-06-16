#ifndef LOG_H
#define LOG_H

int openLog(int _maxLevel);

void writeLog(int _level, const char *_fmt, ...);

void closeLog();

#endif
