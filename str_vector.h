#ifndef STR_VECTOR_H
#define STR_VECTOR_H

typedef void* StrVector;

void initStrVector(StrVector *_vec);
void addStrToVector(StrVector _vec, const char *_str);
void freeStrVector(StrVector *_vec);

size_t getStrVectorCount(StrVector _vec);
const char* getStrInVector(StrVector _vec, size_t _idx);

#endif
