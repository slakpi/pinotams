#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include "config.h"
#include "log.h"

static FILE *log = NULL;
static int maxLevel = 0;

int openLog(int _maxLevel)
{
  if (log)
    closeLog();

  if (_maxLevel <= 0)
    return 0;

  log = fopen(LOG_FILE, "a+");
  if (!log)
    return -1;

  maxLevel = _maxLevel;

  return 0;
}

void writeLog(int _level, const char *_fmt, ...)
{
  char logTime[256];
  time_t now;
  struct tm nowTime;
  va_list args;

  if (!log || _level > maxLevel)
    return;

  va_start(args, _fmt);

  now = time(0);
  localtime_r(&now, &nowTime);
  strftime(logTime, 256, ">> %a, %d %b %Y %H:%M:%S %z ", &nowTime);

  fprintf(log, "%s\n", logTime);
  vfprintf(log, _fmt, args);
  fprintf(log, "\n");
  fflush(log);

  va_end(args);
}

void closeLog()
{
  if (!log)
    return;

  fclose(log);
  log = NULL;
  maxLevel = 0;
}
