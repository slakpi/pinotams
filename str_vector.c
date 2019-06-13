#include <stdlib.h>
#include <string.h>
#include "str_vector.h"

typedef struct __StrVector
{
  char **vec;
  size_t count;
  size_t capac;
} _StrVector;

void initStrVector(StrVector *_vec)
{
  _StrVector **vec = (_StrVector**)_vec;
  *vec = (_StrVector*)malloc(sizeof(_StrVector));
  (*vec)->vec = (char**)malloc(sizeof(char*) * 16);
  (*vec)->count = 0;
  (*vec)->capac = 16;
}

void addStrToVector(StrVector _vec, const char *_str)
{
  _StrVector *vec = (_StrVector*)_vec;
  char **p;

  if (vec->count == vec->capac)
  {
    vec->capac *= 2;
    p = (char**)realloc(vec->vec, sizeof(char*) * vec->capac);
    if (p == NULL)
      return;

    vec->vec = p;
  }

  vec->vec[vec->count] = strdup(_str);
  ++vec->count;
}

void freeStrVector(StrVector *_vec)
{
  _StrVector **vec = (_StrVector**)_vec;
  char **p;

  if (!*vec || !(*vec)->vec)
    return;

  p = (*vec)->vec;
  for ( ; (*vec)->count > 0; --(*vec)->count)
    free(*p++);

  free((*vec)->vec);
  free(*vec);
  *vec = NULL;
}

size_t getStrVectorCount(StrVector _vec)
{
  _StrVector *vec = (_StrVector*)_vec;
  return vec->count;
}

const char* getStrInVector(StrVector _vec, size_t _idx)
{
  _StrVector *vec = (_StrVector*)_vec;

  if (_idx >= vec->count)
    return NULL;

  return vec->vec[_idx];
}
