#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <curl/curl.h>
#include <jansson.h>
#include <sqlite3.h>
#include <mhash.h>

#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>

#include "log.h"
#include "notams.h"

typedef struct __DateTime
{
  int year, month, day;
  int hour, minute, second;
} DateTime;

static int64_t toLilian(const DateTime *_tm)
{
  int64_t a = 0, y = 0, m = 0, ld = 0;

  a = (14 - _tm->month) / 12;
  y = _tm->year + 4800 - a;
  m = _tm->month + 12 * a - 3;
  ld = _tm->day + ((153 * m + 2) / 5) + 365 * y + (y / 4) - (y / 100) + (y / 400) - 32045;

  ld -= 2299160LL;
  ld *= 86400LL; /* days -> seconds */
  ld += _tm->hour * 3600LL;
  ld += _tm->minute * 60LL;
  ld += _tm->second;

  if (ld < 86400LL)
    ld = 86400LL;

  return ld;
}

static void fromLilian(int64_t _ld, DateTime *_tm)
{
  int y = 4716, j = 1401, m = 2, n = 12, r = 4, p = 1461;
  int v = 3, u = 5, s = 153, w = 2, B = 274277, C = -38;
  int e = 0, f = 0, g = 0, h = 0;

  _tm->second = _ld % 60LL; _ld /= 60LL;
  _tm->minute = _ld % 60LL; _ld /= 60LL;
  _tm->hour = _ld % 24LL; _ld /= 24LL;

  _ld += 2299160LL;
  f = (int)_ld + j + (((4 * (int)_ld + B) / 146097) * 3) / 4 + C;
  e = r * f + v;
  g = (e % p) / r;
  h = u * g + w;

  _tm->day = (h % s) / u + 1;
  _tm->month = (h / s + m) % n + 1;
  _tm->year = e / p - y + (n + m - _tm->month) / n;
}

static int64_t notamDateTimeToLilian(char _dateBuf[11])
{
  DateTime tm;
  int j;
  char c;

  memset(&tm, 0, sizeof(tm));

  for (j = 0; j < 10; j += 2)
  {
    c = _dateBuf[j + 2];
    _dateBuf[j + 2] = 0;

    switch (j)
    {
    case 0:
      tm.year = atoi(&_dateBuf[j]) + 2000;
      break;
    case 2:
      tm.month = atoi(&_dateBuf[j]);
      break;
    case 4:
      tm.day = atoi(&_dateBuf[j]);
      break;
    case 6:
      tm.hour = atoi(&_dateBuf[j]);
      break;
    case 8:
      tm.minute = atoi(&_dateBuf[j]);
      break;
    }

    _dateBuf[j + 2] = c;
  }

  return toLilian(&tm);
}

static int64_t lilianNow()
{
  time_t now = time(NULL);
  struct tm tmNow;
  DateTime dtNow;

  gmtime_r(&now, &tmNow);
  dtNow.year = tmNow.tm_year + 1900;
  dtNow.month = tmNow.tm_mon + 1;
  dtNow.day = tmNow.tm_mday;
  dtNow.hour = tmNow.tm_hour;
  dtNow.minute = tmNow.tm_min;
  dtNow.second = tmNow.tm_sec;

  return toLilian(&dtNow);
}

static void hashNotam(const char *_notam, char *_hash, size_t _hashLen)
{
  MHASH h;
  size_t len;

  memset(_hash, 0, sizeof(char) * _hashLen);
  if (_hashLen < 32)
    return;

  len = strlen(_notam);
  if (len < 1)
    return;

  h = mhash_init(MHASH_SHA256);
  if (h == MHASH_FAILED)
    return;

  mhash(h, _notam, len);
  mhash_deinit(h, _hash);
}

static sqlite3* openDatabase(const char *_db)
{
  sqlite3 *db;
  int r;

  r = sqlite3_open_v2(_db, &db, SQLITE_OPEN_READWRITE, NULL);
  if (r == SQLITE_OK)
    return db;

  r = sqlite3_open_v2(_db, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
    NULL);

  if (r != SQLITE_OK)
    return NULL;

  r = sqlite3_exec(db,
    "CREATE TABLE notams("
    " hash BLOB NOT NULL,"
    " key TEXT NOT NULL,"
    " expires INTEGER NOT NULL,"
    " PRIMARY KEY (hash));",
    NULL,
    NULL,
    NULL);

  if (r != SQLITE_OK)
  {
    sqlite3_close_v2(db);
    return NULL;
  }

  return db;
}

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

int queryNotams(const char *_db, const char *_apiKey, const char *_locations,
  StrVector _filters, NOTAM **_latest)
{
  CURL *curlLib;
  CURLcode res;
  json_t *root, *n, *notam, *key, *loc;
  json_error_t err;
  char url[4096], dateBuf[11], hash[32];
  const char *notamStr, *keyStr, *locStr;
  Response json;
  sqlite3 *db = NULL;
  sqlite3_stmt *chk, *ins;
  pcre2_code **regex;
  pcre2_match_data **match;
  PCRE2_SIZE errOffset, *ovect;
  int errCode;
  size_t i, j, notamCount, filterCount;
  int64_t lds, lde;
  NOTAM *c, *p;
  int ok = -1;

  *_latest = NULL;

  filterCount = getStrVectorCount(_filters);
  regex = (pcre2_code**)malloc(sizeof(pcre2_code*) * (filterCount + 1));
  match = (pcre2_match_data**)malloc(sizeof(pcre2_match_data*) * (filterCount + 1));

  regex[0] = pcre2_compile((PCRE2_SPTR)"([0-9]{10})-([0-9]{10}|PERM)", -1,
    PCRE2_UTF, &errCode, &errOffset, NULL);

  match[0] = NULL;
  if (regex[0])
    match[0] = pcre2_match_data_create_from_pattern(regex[0], NULL);

  for (i = 0; i < filterCount; ++i)
  {
    regex[i + 1] = pcre2_compile((PCRE2_SPTR)getStrInVector(_filters, i), -1,
      PCRE2_UTF, &errCode, &errOffset, NULL);

    if (regex[i + 1])
      match[i + 1] = pcre2_match_data_create_from_pattern(regex[i + 1], NULL);
    else
    {
      writeLog("Failed to compile user filter.");
      writeLog(getStrInVector(_filters, i));
      match[i + 1] = NULL;
    }
  }

  if (!regex[0])
    goto cleanup;

  curlLib = curl_easy_init();
  if (!curlLib)
    goto cleanup;

  snprintf(url, 4096,
    "https://v4p4sz5ijk.execute-api.us-east-1.amazonaws.com/"
    "anbdata/states/notams/notams-realtime-list?"
    "api_key=%s&"
    "format=json&"
    "locations=%s",
    _apiKey, _locations);

  initResponse(&json);

  curl_easy_setopt(curlLib, CURLOPT_URL, url);
  curl_easy_setopt(curlLib, CURLOPT_WRITEFUNCTION, notamCallback);
  curl_easy_setopt(curlLib, CURLOPT_WRITEDATA, &json);
  res = curl_easy_perform(curlLib);
  curl_easy_cleanup(curlLib);
  curlLib = NULL;

  if (res != CURLE_OK)
    return -1;

  root = json_loads(json.str, 0, &err);
  freeResponse(&json);

  if (!root)
    return -1;

  if (!json_is_array(root))
    goto cleanup;

  db = openDatabase(_db);
  if (!db)
    goto cleanup;

  sqlite3_prepare(db,
    "INSERT INTO notams(hash, key, expires) VALUES(?, ?, ?);",
    -1,
    &ins,
    NULL);

  sqlite3_prepare(db, "SELECT 1 FROM notams WHERE hash = ?;", -1, &chk, NULL);

  notamCount = json_array_size(root);
  for (i = 0; i < notamCount; ++i)
  {
    n = json_array_get(root, i);
    if (!json_is_object(n))
      continue;

    loc = json_object_get(n, "location");
    if (!json_is_string(loc))
      continue;

    notam = json_object_get(n, "all");
    if (!json_is_string(notam))
      continue;

    key = json_object_get(n, "key");
    if (!json_is_string(key))
      continue;

    locStr = json_string_value(loc);
    notamStr = json_string_value(notam);
    keyStr = json_string_value(key);

    errCode = pcre2_match(regex[0], (PCRE2_SPTR)notamStr, -1, 0, 0, match[0],
      NULL);

    if (errCode != 3)
    {
      /**
       * Failed to find a valid date group. Just default to 180 days from
       * today. If this NOTAM is still around in 180 days, it will be sent
       * out again.
       */
      lds = lilianNow();
      lde = lds + 180 * 24 * 60 * 60;
      writeLog("NOTAM has invalid date/time groups; using defaults.");
    }
    else
    {
      ovect = pcre2_get_ovector_pointer(match[0]);

      strncpy(dateBuf, &notamStr[ovect[2]], 10);
      lds = notamDateTimeToLilian(dateBuf);

      if (strncasecmp(&notamStr[ovect[4]], "PERM", 4) == 0)
      {
        /**
         * This is a permanent notam. Use the original creation date, but set
         * the expiration day to 180 days from today. This will send the NOTAM
         * out again if it still exists.
         */
        lde = lilianNow() + 180 * 24 * 60 * 60;
      }
      else
      {
        strncpy(dateBuf, &notamStr[ovect[4]], 10);
        lde = notamDateTimeToLilian(dateBuf);
      }
    }

    errCode = -1;

    for (j = 0; j < filterCount; ++j)
    {
      errCode = pcre2_match(regex[j + 1], (PCRE2_SPTR)notamStr, -1, 0, 0,
        match[j + 1], NULL);

      if (errCode >= 0)
        break;
    }

    if (errCode >= 0)
    {
      writeLog("NOTAM matches user filter.");
      writeLog(notamStr);
      writeLog("\n\n");
      continue; // Filter matched.
    }

    hashNotam(notamStr, hash, 32);
    sqlite3_bind_blob(chk, 1, hash, 32, SQLITE_STATIC);

    if (sqlite3_step(chk) == SQLITE_DONE)
    {
      writeLog("Recording new NOTAM.");
      writeLog(notamStr);
      writeLog("\n\n");

      sqlite3_bind_blob(ins, 1, hash, 32, SQLITE_STATIC);
      sqlite3_bind_text(ins, 2, keyStr, -1, SQLITE_STATIC);
      sqlite3_bind_int64(ins, 3, lde);
      sqlite3_step(ins);
      sqlite3_reset(ins);

      p = (NOTAM*)malloc(sizeof(NOTAM));
      p->text = strdup(notamStr);
      p->location = strdup(locStr);
      p->key = strdup(keyStr);
      p->created = lds;
      p->expires = lde;
      p->next = NULL;

      if (!*_latest)
        *_latest = c = p;
      else
      {
        c->next = p;
        c = p;
      }
    }

    sqlite3_reset(chk);
  }

  ok = 0;

cleanup:
  if (regex)
  {
    for (i = 0; i < filterCount; ++i)
    {
      if (regex[i])
        pcre2_code_free(regex[i]);
      if (match[i])
        pcre2_match_data_free(match[i]);
    }

    free(regex);
    free(match);
  }
  if (curlLib)
    curl_easy_cleanup(curlLib);
  if (root)
    json_decref(root);
  if (ins)
    sqlite3_finalize(ins);
  if (chk)
    sqlite3_finalize(chk);
  if (db)
    sqlite3_close_v2(db);

  return ok;
}

void freeNotams(NOTAM *_notams)
{
  NOTAM *p;

  while (_notams)
  {
    p = _notams;
    _notams = _notams->next;

    if (p->text)
      free(p->text);
    if (p->location)
      free(p->location);
    if (p->key)
      free(p->key);

    free(p);
  }
}

int trimNotams(const char *_db)
{
  int64_t now = lilianNow();
  sqlite3 *db;
  sqlite3_stmt *trim;
  int ok = -1;

  db = openDatabase(_db);
  if (!db)
    goto cleanup;

  sqlite3_prepare(db, "DELETE FROM notams WHERE expires < ?;", -1, &trim, NULL);
  sqlite3_bind_int64(trim, 1, now);
  if (sqlite3_step(trim) != SQLITE_DONE)
    goto cleanup;

  ok = 0;

cleanup:
  if (trim)
    sqlite3_finalize(trim);
  if (db)
    sqlite3_close_v2(db);

  return ok;
}
