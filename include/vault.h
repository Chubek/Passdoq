#ifndef VAULT_H
#define VAULT_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "safemem.h"

typedef struct PDQ_VaultHeader
{
  uint8_t magic[PDQ_SIZE_MAGIC];
  uint32_t version;
  uint8_t salt[PDQ_SIZE_SALT];
  uint8_t global_nonce[PDQ_SIZE_NONCE];
  time_t created_at;
  time_t modified_at;
  size_t num_entries;
} PDQ_VaultHeader;

typedef struct PDQ_VaultMetadata
{
  PDQ_SafeArray *per_entry_nonce;
  PDQ_SafeArray *per_entry_key;
  PDQ_SafeArray *per_entry_offset;
} PDQ_VaultMetadata;

typedef struct PDQ_VaultEntry
{
  int id;
  PDQ_SafeString *platform;
  PDQ_SafeString *username;
  PDQ_SafeString *password;
} PDQ_VaultEntry;

typedef struct PDQ_Vault
{
  PDQ_VaultHeader header;
  PDQ_VaultMetadata metadata;
  PDQ_VaultEntry entries;
} PDQ_Vault;

#endif
