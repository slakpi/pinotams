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
  size_t line;
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
  char *p;
  size_t len, i;

  if (!ctx->cur)
    return 0;

  if (!ctx->pos)
  {
    ctx->pos = ctx->cur->text;
    ctx->remaining = strlen(ctx->cur->text);
    ctx->line = 0;
  }

  i = 0;
  p = _ptr;
  len = min(ctx->remaining, _nmemb - 1);

  if (ctx->state != notamState)
  {
    memcpy(_ptr, ctx->pos, len);
    ctx->pos += len;
    ctx->remaining -= len;
  }
  else
  {
    while (ctx->remaining > 0 && i < len)
    {
      if (ctx->line < 998 && *ctx->pos != '\n')
      {
        *p++ = *ctx->pos;
        ++i;
      }
      else
      {
        *p++ = '\r';
        *p++ = '\n';
        i += 2;
        ctx->line = 0;

        if (*ctx->pos != '\n')
          continue;
      }

      ++ctx->pos;
      --ctx->remaining;
    }
  }

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
  const StrVector _recipients, int _tls, const NOTAM *_notams)
{
  MailContext ctx;
  CURL *curl;
  CURLcode res;
  uuid_t mailId;
  time_t now;
  struct tm nowTime;
  char tmp[4096];
  char *hdr, *p;
  size_t len, buf, i, r;
  int first = 1;
  struct curl_slist *recipients = NULL, *recipTmp = NULL;

  buf = 0;
  r = getStrVectorCount(_recipients);
  if (r < 1)
    return 0;

  for (i = 0; i < r; ++i)
  {
    buf += strlen(getStrInVector(_recipients, i) + 4);
    recipTmp = curl_slist_append(recipients, getStrInVector(_recipients, i));

    if (!recipTmp)
    {
      if (recipients)
        curl_slist_free_all(recipients);

      return -1;
    }

    recipients = recipTmp;
  }

  now = time(NULL);
  localtime_r(&now, &nowTime);
  len = strftime(tmp, 4096, "%a, %d %b %Y %H:%M:%S %z", &nowTime);

  buf += strlen(_senderName) + strlen(_sender) + len;
  buf = (buf << 1) + 256;
  hdr = (char*)malloc(sizeof(char) * buf);

  r = getStrVectorCount(_recipients);
  strncpy(hdr, "To: ", 5);
  p = hdr + 4;
  for (i = 0; i < r; ++i)
  {
    if (!first)
    {
      strncpy(p, ", \r\n ", 6); // Folding white-space
      p += 5;
    }

    len = snprintf(p, buf - (p - hdr), "<%s>", getStrInVector(_recipients, i));
    p += len;
    first = 0;
  }

  len = snprintf(p, buf,
    "\r\n"
    "From: %s <%s>\r\n"
    "Subject: !! NOTAM Update !!\r\n"
    "Date: %s\r\n"
    "X-Priority: 1\r\n"
    "X-MSMail-Priority: High\r\n"
    "Importance: High\r\n",
    _senderName, _sender, tmp);

  p += len;

  uuid_generate(mailId);
  uuid_unparse(mailId, tmp);
  snprintf(p, buf - (p - hdr), "Message-Id: <%s@%s>\r\n\r\n", tmp, _server);

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
