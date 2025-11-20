#ifndef SAFEMEM_H
#define SAFEMEM_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct PDQ_SafeMemory
{
  void *memory;
  uint8_t item_size;
  size_t item_count;
  size_t total_size;
  bool locked : 1;
  bool readonly : 1;
  bool noaccess : 1;
} PDQ_SafeMemory;

typedef struct PDQ_SafeString
{
  uint8_t *value;
  size_t len, cap;
  PDQ_SafeMemory *safemem;
} PDQ_SafeString;

typedef struct PDQ_SafeArray
{
  void **values;
  size_t len, cap;
  const char *tag;
  PDQ_SafeMemory *safemem;
} PDQ_SafeArray;

PDQ_SafeMemory *pdq_safemem_new (uint8_t item_size, size_t item_count);
PDQ_SafeMemory *pdq_safemem_delete (PDQ_SafeMemory *smem);
PDQ_SafeMemory *pdq_safemem_resize (PDQ_SafeMemory *smem, size_t new_count);
void pdq_safemem_lock (PDQ_SafeMemory *smem);
void pdq_safemem_unlock (PDQ_SafeMemory *smem);
void pdq_safemem_readonly (PDQ_SafeMemory *smem);
void pdq_safemem_readwrite (PDQ_SafeMemory *smem);
void pdq_safemem_noaccess (PDQ_SafeMemory *smem);
void pdq_safemem_access (PDQ_SafeMemory *smem);

PDQ_SafeString *pdq_safestring_new (void);
void pdq_safestring_delete (PDQ_SafeString *sstr);
PDQ_SafeString *pdq_safestring_from (uint8_t *value, size_t length);
bool pdq_safestring_shouldgrow (PDQ_SafeString *sstr);
PDQ_SafeString *pdq_safestring_grow (PDQ_SafeString *sstr);
PDQ_SafeString *pdq_safestring_copy (PDQ_SafeString *sstr);
PDQ_SafeString *pdq_safestring_concat (PDQ_SafeString *sstr1,
                                       PDQ_SafeString *sstr2);

PDQ_SafeArray *pdq_safearray_new (size_t size);
void pdq_safearray_delete (PDQ_SafeArray *sarr);
bool pdq_safearray_shouldgrow (PDQ_SafeArray *sarr);
PDQ_SafeArray *pdq_safearray_grow (PDQ_SafeArray *sarr);
void pdq_safearray_push (PDQ_SafeArray *sarr, void *item);
void pdq_safearray_shift (PDQ_SafeArray *sarr, void *item);
void *pdq_safearray_pop (PDQ_SafeArray *sarr);
void *pdq_safearray_unshift (PDQ_SafeArray *sarr);
void *pdq_safearray_getat (PDQ_SafeArray *sarr, size_t idx);
void pdq_safearray_setat (PDQ_SafeArray *sarr, size_t idx, void *item);

#endif
