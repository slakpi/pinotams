#ifndef CONFIG_HELPERS_H
#define CONFIG_HELPERS_H

#include "str_vector.h"

typedef struct __PinotamsConfig
{
  char *installPrefix;
  char *configFile;
  char *cacheFile;
  char *locations;
  char *apiKey;
  int refreshRate;
  int filterSuaw;
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
