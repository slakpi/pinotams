#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <jansson.h>
#include <sqlite3.h>
#include "notams.h"

typedef struct __Response
{
  char *str;
  size_t len, bufLen;
} Response;

static void initResponse(Response *_res)
{
  _res->str = (char*)malloc(sizeof(char) * 256);
  _res->str[0] = 0;
  _res->len = 0;
  _res->bufLen = 256;
}

static void appendToResponse(Response *_res, const char *_str, size_t _len)
{
  size_t newBuf = _res->bufLen;

  if (!_res->str)
    return;

  while (1)
  {
    if (_res->len + _len < newBuf)
    {
      if (_res->bufLen < newBuf)
      {
        _res->str = realloc(_res->str, newBuf);
        _res->bufLen = newBuf;
      }

      memcpy(_res->str + _res->len, _str, _len);
      _res->len += _len;
      _res->str[_res->len] = 0;

      return;
    }

    newBuf <<= 1;
    if (newBuf < _res->bufLen)
      return;
  }
}

static void freeResponse(Response *_res)
{
  free(_res->str);
  _res->str = NULL;
  _res->len = 0;
  _res->bufLen = 0;
}

static size_t notamCallback(char *_ptr, size_t _size, size_t _nmemb,
  void *_userdata)
{
  Response *res = (Response*)_userdata;
  appendToResponse(res, _ptr, _nmemb);
  return _nmemb;
}

int queryNotams(const char *_apiKey, const char *_locations, NOTAM **_latest)
{
  CURL *curlLib;
  CURLcode res;
#if 0
  json_t *root;
  json_error_t err;
#endif
  char url[4096];
  Response json;
  int ok = -1;

  curlLib = curl_easy_init();
  if (!curlLib)
    return -1;

  snprintf(url, 4096,
    "https://v4p4sz5ijk.execute-api.us-east-1.amazonaws.com/"
    "anbdata/states/notams/notams-realtime-list?"
    "api_key=%s&"
    "format=json&"
    "criticality=&"
    "locations=%s",
    _apiKey, _locations);

  printf(url);

  initResponse(&json);

  curl_easy_setopt(curlLib, CURLOPT_URL, url);
  curl_easy_setopt(curlLib, CURLOPT_WRITEFUNCTION, notamCallback);
  curl_easy_setopt(curlLib, CURLOPT_WRITEDATA, &json);
  res = curl_easy_perform(curlLib);
  curl_easy_cleanup(curlLib);

  if (res != CURLE_OK)
    return -1;

#if 0
  root = json_loads(json.str, 0, &err);
  freeResponse(&json);

  if (!root)
    return -1;
#endif

  printf(json.str);

  return 0;
}

void freeNotams(NOTAM *_notams)
{

}

int trimNotams()
{
  return 0;
}
