#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <getopt.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include "config_helpers.h"
#include "notams.h"
#include "mail.h"

static const char *shortArgs = "st";
static const struct option longArgs[] = {
  { "stand-alone", no_argument,       0, 's' },
  { "test",        no_argument,       0, 't' },
  { 0,             0,                 0,  0  }
};

static int run = 1;

static void signalHandler(int _signo)
{
  switch (_signo)
  {
  case SIGINT:
  case SIGTERM:
  case SIGHUP:
    run = 0;
    break;
  }
}

static void writeLog(FILE *_log, const char *_msg)
{
  char logTime[256];
  time_t now;
  struct tm nowTime;

  if (!_log)
    return;

  now = time(0);
  localtime_r(&now, &nowTime);
  strftime(logTime, 256, "%a, %d %b %Y %H:%M:%S %z >> ", &nowTime);

  fprintf(_log, "%s %s\n", logTime, _msg);
  fflush(_log);
}

static int go(int _test)
{
  PinotamsConfig *cfg = getPinotamsConfig();
  NOTAM *notams;
  FILE *log = NULL;
  time_t nextUpdate = 0, now;
  int ret;

  if (cfg->debugLog)
    log = fopen(cfg->logFile, "a+");

  do
  {
    now = time(0);

    if (now < nextUpdate)
    {
      usleep(50000);
      continue;
    }

    ret = trimNotams(cfg->cacheFile);
    if (ret != 0)
      writeLog(log, "Failed to trim NOTAM cache.");

    ret = queryNotams(cfg->cacheFile, cfg->apiKey, cfg->locations,
      cfg->filterSuaw, &notams);
    if (ret != 0)
      writeLog(log, "Failed to query NOTAMs.");

    if (!notams)
      writeLog(log, "No new NOTAMs.");
    else
    {
      ret = mailNotams(cfg->smtpServer, cfg->smtpPort, cfg->smtpUser, cfg->smtpPwd,
        cfg->smtpSender, cfg->smtpSenderName, cfg->smtpRecipients, cfg->smtpTLS,
        notams);
      if (ret != 0)
        writeLog(log, "Failed to mail NOTAMs.");

      freeNotams(notams);
      notams = NULL;
    }

    nextUpdate = ((now / cfg->refreshRate) + (now % cfg->refreshRate != 0)) *
      cfg->refreshRate;

    if (_test)
      break;
  } while (run);

  if (cfg)
    freePinotamsConfig(cfg);
  if (log)
    fclose(log);

  return 0;
}

int main(int _argc, char* _argv[])
{
  pid_t pid, sid;
  int c, test = 0, standAlone = 0;

  while ((c = getopt_long(_argc, _argv, shortArgs, longArgs, 0)) != -1)
  {
    switch (c)
    {
    case 's':
      standAlone = 1;
      break;
    case 't':
      standAlone = 1;
      test = 1;
      break;
    }
  }

  if (standAlone)
  {
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    signal(SIGHUP, signalHandler);
    return go(test);
  }

  pid = fork();

  if (pid < 0)
    return -1;
  if (pid > 0)
    return 0;

  umask(0);

  sid = setsid();

  if (sid < 0)
    return -1;

  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);

  signal(SIGINT, signalHandler);
  signal(SIGTERM, signalHandler);
  signal(SIGHUP, signalHandler);

  return go(0);
}
