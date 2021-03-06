#ifndef CONF_FILE_H
#define CONF_FILE_H

typedef enum __ConfParam
{
  confLocations,
  confApiKey,
  confRefreshRate,
  confFilter,
  confDebugLog,
  confSmtpServer,
  confSmtpPort,
  confSmtpUser,
  confSmtpPwd,
  confSmtpSender,
  confSmtpSenderName,
  confSmtpRecipient,
  confSmtpTLS
} ConfParam;

#endif
