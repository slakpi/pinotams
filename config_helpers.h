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
  int smtpPort;
  char *smtpUser;
  char *smtpPwd;
  char *smtpSender;
  char *smtpSenderName;
  char *smtpRecipient;
  int smtpTLS;
} PinotamsConfig;

PinotamsConfig* getPinotamsConfig();
void freePinotamsConfig(PinotamsConfig *_cfg);

#endif
