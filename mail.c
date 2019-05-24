#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <curl/curl.h>
#include <uuid/uuid.h>
#include "mail.h"

typedef enum __ContextState
{
  headerState,
  notamState,
  separatorState
} ContextState;

typedef struct __MailContext
{
  ContextState state;
  const NOTAM *cur;
  const char *pos;
  size_t remaining;
} MailContext;

static const char *sep = "\n\n";

static size_t min(size_t _a, size_t _b)
{
  return (_a < _b ? _a : _b);
}

static size_t notamCallback(char *_ptr, size_t _size, size_t _nmemb,
  void *_userdata)
{
  MailContext *ctx = (MailContext*)_userdata;
  size_t len;

  if (!ctx->cur)
    return 0;

  if (!ctx->pos)
  {
    ctx->pos = ctx->cur->text;
    ctx->remaining = strlen(ctx->cur->text);
  }

  len = min(ctx->remaining, _nmemb);
  memcpy(_ptr, ctx->pos, len);
  ctx->pos += len;
  ctx->remaining -= len;

  if (ctx->remaining > 0)
    return len;

  switch (ctx->state)
  {
  case headerState:
    ctx->state = notamState;
    ctx->pos = NULL;
    ctx->remaining = 0;
    break;
  case notamState:
    ctx->state = separatorState;
    ctx->pos = sep;
    ctx->remaining = strlen(sep);
    break;
  case separatorState:
    ctx->state = notamState;
    ctx->cur = ctx->cur->next;
    ctx->pos = NULL;
    ctx->remaining = 0;
    break;
  }

  return len;
}

int mailNotams(const char *_server, int _port, const char *_user,
  const char *_pwd, const char *_sender, const char *_senderName,
  const char *_recipient, int _tls, const NOTAM *_notams)
{
  MailContext ctx;
  CURL *curl;
  CURLcode res;
  uuid_t mailId;
  time_t now;
  struct tm nowTime;
  char tmp[4096];
  char *hdr;
  size_t len, buf;
  struct curl_slist *recipients = NULL;

  now = time(NULL);
  localtime_r(&now, &nowTime);
  len = strftime(tmp, 4096, "%a, %d %b %Y %H:%M:%S %z", &nowTime);

  buf = strlen(_recipient) + strlen(_senderName) + strlen(_sender) + len;
  buf = (buf << 1) + 256;
  hdr = (char*)malloc(sizeof(char) * buf);

  len = snprintf(hdr, buf,
    "To: <%s>\r\n"
    "From: %s <%s>\r\n"
    "Subject: !! NOTAM Update !!\r\n"
    "Date: %s\r\n"
    "X-Priority: 1\r\n"
    "X-MSMail-Priority: High\r\n"
    "Importance: High\r\n",
    _recipient, _senderName, _sender, tmp);

  uuid_generate(mailId);
  uuid_unparse(mailId, tmp);
  snprintf(hdr + len, buf - len, "Message-Id: <%s@%s>\r\n\r\n", tmp, _server);

  ctx.state = headerState;
  ctx.cur = _notams;
  ctx.pos = hdr;
  ctx.remaining = strlen(hdr);

  curl = curl_easy_init();
  if (!curl)
    return -1;

  curl_easy_setopt(curl, CURLOPT_USERNAME, _user);
  curl_easy_setopt(curl, CURLOPT_PASSWORD, _pwd);
  snprintf(tmp, 4096, "smtp://%s:%d", _server, _port);
  curl_easy_setopt(curl, CURLOPT_URL, tmp);

  if (_tls)
    curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);

  curl_easy_setopt(curl, CURLOPT_MAIL_FROM, _sender);

  recipients = curl_slist_append(recipients, _recipient);
  curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

  curl_easy_setopt(curl, CURLOPT_READFUNCTION, notamCallback);
  curl_easy_setopt(curl, CURLOPT_READDATA, &ctx);
  curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

  res = curl_easy_perform(curl);
  curl_slist_free_all(recipients);
  curl_easy_cleanup(curl);
  free(hdr);

  return (res == CURLE_OK ? 0 : -1);
}
