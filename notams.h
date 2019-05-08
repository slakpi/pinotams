#ifndef NOTAM_H
#define NOTAM_H

typedef struct __NOTAM
{
  char *text;
  char *location;
  char *key;
  time_t start;
  time_t end;
  struct __NOTAM *next;
} NOTAM;

int queryNotams(const char *_apiKey, const char *_locations, NOTAM **_latest);
int trimNotams();

#endif
