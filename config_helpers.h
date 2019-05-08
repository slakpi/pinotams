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
} PinotamsConfig;

PinotamsConfig* getPinotamsConfig();
void freePinotamsConfig(PinotamsConfig *_cfg);

#endif
