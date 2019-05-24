#ifndef CONFIG_HELPERS_H
#define CONFIG_HELPERS_H

typedef struct __PinotamsConfig
{
  char *installPrefix;
  char *configFile;
  char *cacheFile;
  char *locations;
  char *apiKey;
  int refreshRate;
  char *smtpServer;
  char *smtpUser;
  char *smtpPwd;
  char *smtpRecipient;
  int smtpTLS;
} PinotamsConfig;

PinotamsConfig* getPinotamsConfig();
void freePinotamsConfig(PinotamsConfig *_cfg);

#endif
