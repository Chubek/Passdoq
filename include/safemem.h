#ifndef SAFEMEM_H
#define SAFEMEM_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct PDQ_SafeString
{
  uint8_t *value;
  size_t len, cap;
} PDQ_SafeString;

typedef struct PDQ_SafeArray
{
  void **values;
  size_t len, cap;
  const char *tag;
} PDQ_SafeArray;

PDQ_SafeString *pdq_safestring_new (void);
void pdq_safestring_delete (PDQ_SafeString *sstr);
PDQ_SafeString *pdq_safestring_from (uint8_t *value, size_t length);
bool pdq_safestring_mustgrow (PDQ_SafeString *sstr);
PDQ_SafeString *pdq_safestring_grow (PDQ_SafeString *sstr);
PDQ_SafeString *pdq_safestring_copy (PDQ_SafeString *sstr);
PDQ_SafeString *pdq_safestring_concat (PDQ_SafeString *sstr1,
                                       PDQ_SafeString *sstr2);

PDQ_SafeArray *pdq_safearray_new (size_t size);
void pdq_safearray_delete (PDQ_SafeArray *sarr);
bool pdq_safearray_mustgrow (PDQ_SafeArray *sarr);
PDQ_SafeArray *pdq_safearray_grow (PDQ_SafeArray *sarr);
void pdq_safearray_push (PDQ_SafeArray *sarr, void *item);
void pdq_safearray_shift (PDQ_SafeArray *sarr, void *item);
void *pdq_safearray_pop (PDQ_SafeArray *sarr);
void *pdq_safearray_unshift (PDQ_SafeArray *sarr);
void *pdq_safearray_getat (PDQ_SafeArray *sarr, size_t idx);
void pdq_safearray_setat (PDQ_SafeArray *sarr, size_t idx, void *item);

#endif
