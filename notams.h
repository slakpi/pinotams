#ifndef NOTAMS_H
#define NOTAMS_H

#include <sys/types.h>
#include "str_vector.h"

typedef struct __NOTAM
{
  char *text;
  char *location;
  char *key;
  int64_t created;
  int64_t expires;
  struct __NOTAM *next;
} NOTAM;

int queryNotams(const char *_db, const char *_apiKey, const char *_locations,
  StrVector _filters, NOTAM **_latest);

void freeNotams(NOTAM *_notams);

int trimNotams(const char *_db);

#endif
