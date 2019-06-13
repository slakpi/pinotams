#ifndef MAIL_H
#define MAIL_H

#include "notams.h"
#include "str_vector.h"

int mailNotams(const char *_server, int _port, const char *_user,
  const char *_pwd, const char *_sender, const char *_senderName,
  const StrVector _recipients, int _tls, const NOTAM *_notams);

#endif
