#ifndef CONF_FILE_H
#define CONF_FILE_H

typedef enum __ConfParam
{
  confLocations,
  confApiKey,
  confRefreshRate,
  confSmtpServer,
  confSmtpUser,
  confSmtpPwd,
  confSmtpRecipient,
  confSmtpTLS
} ConfParam;

#endif
