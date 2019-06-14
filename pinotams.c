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

static int go(int _test)
{
  PinotamsConfig *cfg = getPinotamsConfig();
  NOTAM *notams;
  time_t nextUpdate = 0, now;

  do
  {
    now = time(0);

    if (now < nextUpdate)
    {
      usleep(50000);
      continue;
    }

    trimNotams(cfg->cacheFile);
    queryNotams(cfg->cacheFile, cfg->apiKey, cfg->locations, cfg->filterSuaw,
      &notams);

    if (notams)
    {
      mailNotams(cfg->smtpServer, cfg->smtpPort, cfg->smtpUser, cfg->smtpPwd,
        cfg->smtpSender, cfg->smtpSenderName, cfg->smtpRecipients, cfg->smtpTLS,
        notams);
      freeNotams(notams);
      notams = NULL;
    }

    nextUpdate = ((now / cfg->refreshRate) + (now % cfg->refreshRate != 0)) *
      cfg->refreshRate;
  } while (run);

  if (cfg)
    freePinotamsConfig(cfg);

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
