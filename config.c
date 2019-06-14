#include <string.h>
#include <config.h>

typedef void* yyscan_t;

#include "config_helpers.h"
#include "conf_file.h"
#include "conf_file.parser.h"

#define YYSTYPE CONF_STYPE

#include "conf_file.lexer.h"

PinotamsConfig* getPinotamsConfig()
{
  FILE *cfgFile;
  yyscan_t scanner;
  PinotamsConfig *cfg = (PinotamsConfig*)malloc(sizeof(PinotamsConfig));

  cfg->installPrefix = strdup(INSTALL_PREFIX);
  cfg->configFile = strdup(CONFIG_FILE);
  cfg->cacheFile = strdup(CACHE_FILE);
  cfg->logFile = strdup(LOG_FILE);
  cfg->locations = NULL;
  cfg->apiKey = NULL;
  cfg->refreshRate = 360 * 60;
  cfg->filterSuaw = 1;
  cfg->smtpServer = NULL;
  cfg->smtpUser = NULL;
  cfg->smtpPwd = NULL;
  cfg->smtpSender = NULL;
  cfg->smtpSenderName = NULL;
  initStrVector(&cfg->smtpRecipients);
  cfg->smtpTLS = 0;

  cfgFile = fopen(CONFIG_FILE, "r");
  if (!cfgFile)
    return cfg;

  conf_lex_init(&scanner);
  conf_set_in(cfgFile, scanner);
  conf_parse(scanner, cfg);
  conf_lex_destroy(scanner);

  fclose(cfgFile);

  return cfg;
}

void freePinotamsConfig(PinotamsConfig *_cfg)
{
  if (_cfg->installPrefix)
    free(_cfg->installPrefix);
  if (_cfg->configFile)
    free(_cfg->configFile);
  if (_cfg->cacheFile)
    free(_cfg->cacheFile);
  if (_cfg->logFile)
    free(_cfg->logFile);
  if (_cfg->locations)
    free(_cfg->locations);
  if (_cfg->apiKey)
    free(_cfg->apiKey);
  if (_cfg->smtpServer)
    free(_cfg->smtpServer);
  if (_cfg->smtpUser)
    free(_cfg->smtpUser);
  if (_cfg->smtpPwd)
    free(_cfg->smtpPwd);
  if (_cfg->smtpSender)
    free(_cfg->smtpSender);
  if (_cfg->smtpSenderName)
    free(_cfg->smtpSenderName);

  freeStrVector(&_cfg->smtpRecipients);

  free(_cfg);
}

static char* appendFileToPath(const char *_prefix, const char *_file, char *_path, size_t _len)
{
  size_t pl = strlen(_prefix), fl = strlen(_file);

  if (_prefix[pl - 1] != '/')
    ++pl;
  if (pl + fl + 1 >= _len)
    return NULL;

  _path[0] = 0;

  strcat(_path, _prefix);

  if (_path[pl - 1] != '/')
  {
    _path[pl - 1] = '/';
    _path[pl] = 0;
  }

  strcat(_path, _file);

  return _path;
}
