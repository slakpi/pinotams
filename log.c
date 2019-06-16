#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include "config.h"
#include "log.h"

static FILE *log = NULL;

int openLog()
{
  if (log)
    closeLog();

  log = fopen(LOG_FILE, "a+");
  if (!log)
    return -1;

  return 0;
}

void writeLog(const char *_fmt, ...)
{
  char logTime[256];
  time_t now;
  struct tm nowTime;
  va_list args;

  if (!log)
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
}
