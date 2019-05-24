#ifndef MAIL_H
#define MAIL_H

#include "notams.h"

int mailNotams(const char *_server, int _port, const char *_user,
  const char *_pwd, const char *_sender, const char *_senderName,
  const char *_recipient, int _tls, const NOTAM *_notams);

#endif
