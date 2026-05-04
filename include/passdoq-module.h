#ifndef PASSDOQ_MODULE_H
#define PASSDOQ_MODULE_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Module API version */
#define PASSDOQ_MODULE_API_VERSION 1

/* Forward declarations */
typedef struct passdoq_ctx passdoq_ctx_t;
typedef struct passdoq_module passdoq_module_t;

/* Log levels */
typedef enum {
    PASSDOQ_LOG_DEBUG,
    PASSDOQ_LOG_INFO,
    PASSDOQ_LOG_WARNING,
    PASSDOQ_LOG_ERROR
} passdoq_log_level_t;

/* Module hooks */
typedef int (*passdoq_init_fn)(passdoq_ctx_t *ctx);
typedef void (*passdoq_deinit_fn)(passdoq_ctx_t *ctx);
typedef int (*passdoq_vault_unlock_fn)(passdoq_ctx_t *ctx);
typedef int (*passdoq_vault_lock_fn)(passdoq_ctx_t *ctx);
typedef int (*passdoq_config_fn)(passdoq_ctx_t *ctx, const char *key, const char *value);

/* Module descriptor */
struct passdoq_module {
    const char *name;
    const char *version;
    const char *description;
    const char *author;
    
    /* Hooks */
    passdoq_init_fn init;
    passdoq_deinit_fn deinit;
    passdoq_vault_unlock_fn on_vault_unlock;
    passdoq_vault_lock_fn on_vault_lock;
    passdoq_config_fn on_config;
};

/* Module entry point */
#define PASSDOQ_MODULE_EXPORT __attribute__((visibility("default")))

/* Module initialization function signature */
typedef passdoq_module_t* (*passdoq_module_init_fn)(void);

/* API functions available to modules */
void passdoq_log(passdoq_ctx_t *ctx, passdoq_log_level_t level, const char *message);
int passdoq_crypto_random(passdoq_ctx_t *ctx, uint8_t *buf, size_t size);
int passdoq_vault_rekey(passdoq_ctx_t *ctx, const uint8_t *new_key, size_t key_size);
void passdoq_secure_memzero(void *ptr, size_t size);

#ifdef __cplusplus
}
#endif

#endif /* PASSDOQ_MODULE_H */
