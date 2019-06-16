#include <stdio.h>
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

void writeLog(const char *_msg)
{
  char logTime[256];
  time_t now;
  struct tm nowTime;

  if (!log)
    return;

  now = time(0);
  localtime_r(&now, &nowTime);
  strftime(logTime, 256, "%a, %d %b %Y %H:%M:%S %z >> ", &nowTime);

  fprintf(log, "%s %s\n", logTime, _msg);
  fflush(log);
}

void closeLog()
{
  if (!log)
    return;

  fclose(log);
  log = NULL;
}
