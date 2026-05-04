#ifndef PASSDOQ_API_H
#define PASSDOQ_API_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* API version */
#define PASSDOQ_API_VERSION 1

/* Forward declarations */
typedef struct passdoq_vault passdoq_vault_t;
typedef struct passdoq_entry passdoq_entry_t;

/* Vault state */
typedef enum {
    PASSDOQ_VAULT_UNINITIALIZED,
    PASSDOQ_VAULT_LOCKED,
    PASSDOQ_VAULT_UNLOCKED
} passdoq_vault_state_t;

/* Error codes */
typedef enum {
    PASSDOQ_OK = 0,
    PASSDOQ_ERROR = -1,
    PASSDOQ_ERROR_INVALID_PARAM = -2,
    PASSDOQ_ERROR_VAULT_LOCKED = -3,
    PASSDOQ_ERROR_NOT_FOUND = -4,
    PASSDOQ_ERROR_CRYPTO = -5,
    PASSDOQ_ERROR_IO = -6
} passdoq_error_t;

/* Initialize library */
int passdoq_init(void);

/* Vault operations */
passdoq_vault_t* passdoq_vault_create(const char *path, const char *passphrase);
passdoq_vault_t* passdoq_vault_open(const char *path);
void passdoq_vault_close(passdoq_vault_t *vault);

int passdoq_vault_unlock(passdoq_vault_t *vault, const char *passphrase);
void passdoq_vault_lock(passdoq_vault_t *vault);
passdoq_vault_state_t passdoq_vault_get_state(passdoq_vault_t *vault);

/* Entry operations */
passdoq_entry_t* passdoq_entry_create(void);
void passdoq_entry_free(passdoq_entry_t *entry);

int passdoq_entry_set_name(passdoq_entry_t *entry, const char *name);
int passdoq_entry_set_username(passdoq_entry_t *entry, const char *username);
int passdoq_entry_set_secret(passdoq_entry_t *entry, const uint8_t *secret, size_t size);

const char* passdoq_entry_get_id(passdoq_entry_t *entry);
const char* passdoq_entry_get_name(passdoq_entry_t *entry);
const char* passdoq_entry_get_username(passdoq_entry_t *entry);
const uint8_t* passdoq_entry_get_secret(passdoq_entry_t *entry, size_t *size);

/* Vault entry management */
int passdoq_vault_add_entry(passdoq_vault_t *vault, passdoq_entry_t *entry, char **out_id);
int passdoq_vault_get_entry(passdoq_vault_t *vault, const char *id, passdoq_entry_t **out_entry);
int passdoq_vault_update_entry(passdoq_vault_t *vault, const char *id, passdoq_entry_t *entry);
int passdoq_vault_delete_entry(passdoq_vault_t *vault, const char *id);

/* Save vault */
int passdoq_vault_save(passdoq_vault_t *vault);

#ifdef __cplusplus
}
#endif

#endif /* PASSDOQ_API_H */
