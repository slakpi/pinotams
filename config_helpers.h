#ifndef CONFIG_HELPERS_H
#define CONFIG_HELPERS_H

#include "str_vector.h"

typedef struct __PinotamsConfig
{
  char *installPrefix;
  char *configFile;
  char *cacheFile;
  char *logFile;
  StrVector locations;
  char *apiKey;
  int refreshRate;
  StrVector filters;
  int debugLog;
  char *smtpServer;
  int smtpPort;
  char *smtpUser;
  char *smtpPwd;
  char *smtpSender;
  char *smtpSenderName;
  StrVector smtpRecipients;
  int smtpTLS;
} PinotamsConfig;

PinotamsConfig* getPinotamsConfig();

void freePinotamsConfig(PinotamsConfig *_cfg);

#endif
