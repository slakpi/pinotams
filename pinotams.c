#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <getopt.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include "config.h"
#include "config_helpers.h"
#include "notams.h"
#include "mail.h"
#include "log.h"

static const char *shortArgs = "stv";
static const struct option longArgs[] = {
  { "stand-alone", no_argument,       0, 's' },
  { "test",        no_argument,       0, 't' },
  { "version",     no_argument,       0, 'v' },
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

static int go(int _test)
{
  PinotamsConfig *cfg = getPinotamsConfig();
  NOTAM *notams, *notamsTmp, *p;
  time_t nextUpdate = 0, now;
  const char *query;
  size_t len, i;
  int ret, fail;

  openLog(cfg->debugLog);

  do
  {
    fail = 0;
    now = time(0);

    if (now < nextUpdate)
    {
      usleep(50000);
      continue;
    }

    ret = trimNotams(cfg->cacheFile);
    if (ret != 0)
      writeLog(1, "Failed to trim NOTAM cache.");

    notams = NULL;
    len = getStrVectorCount(cfg->locations);
    for (i = 0; i < len; ++i)
    {
      query = getStrInVector(cfg->locations, i);

      writeLog(2, "Querying for locations: %s", query);
      ret = queryNotams(cfg->cacheFile, cfg->apiKey, query, cfg->filters,
        &notamsTmp);

      writeLog(2, "queryNotams() returned %d.", ret);

      if (ret != 0)
      {
        writeLog(1, "Failed to query locations: %s", query);
        fail = 1;
        break;
      }

      writeLog(2, "Query succeeded, merging any new NOTAMs.");

      if (notamsTmp)
      {
        if (notams)
          p->next = notamsTmp;
        else
        {
          notams = notamsTmp;
          p = notams;
        }

        while (p->next != NULL)
          p = p->next;
      }
    }

    notamsTmp = NULL;
    p = NULL;

    if (fail)
    {
      if (notams)
        freeNotams(notams);

      notams = NULL;
      now = time(0);
      nextUpdate = now + 3600; // 60-minute time out

      writeLog(2, "Delaying re-query by 1 hour.");

      continue;
    }

    if (!notams)
      writeLog(1, "No new NOTAMs.");
    else
    {
      writeLog(2, "Attempting to mail new NOTAMs.");
      ret = mailNotams(cfg->smtpServer, cfg->smtpPort, cfg->smtpUser, cfg->smtpPwd,
        cfg->smtpSender, cfg->smtpSenderName, cfg->smtpRecipients, cfg->smtpTLS,
        notams);

      if (ret != 0)
        writeLog(1, "Failed to mail NOTAMs.");

      freeNotams(notams);
      notams = NULL;
    }

    now = time(0);
    nextUpdate = ((now / cfg->refreshRate) + (now % cfg->refreshRate != 0)) *
      cfg->refreshRate;

    if (_test)
      break;
  } while (run);

  if (cfg)
    freePinotamsConfig(cfg);

  closeLog();

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
    case 'v':
      printf("pinotams (%s)\n", GIT_COMMIT_HASH);
      return 0;
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
