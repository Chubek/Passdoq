// example/plugins/ephemeral_datakey.c

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "passdoq/passdoq-module.h"
#include "passdoq/passdoq-api.h"

#define MODULE_NAME "EphemeralDataKey"
#define DEFAULT_ROTATION_SECONDS 300

static uint64_t last_rotation = 0;
static uint64_t rotation_interval = DEFAULT_ROTATION_SECONDS;

/* Utility: get unix time */
static uint64_t now_sec(void) {
    return (uint64_t)time(NULL);
}

/* Hook: called when vault is unlocked */
static int on_vault_unlock(passdoq_ctx_t *ctx) {
    uint64_t now = now_sec();

    if ((now - last_rotation) < rotation_interval) {
        return 0; // no rotation needed
    }

    // Generate new data key
    uint8_t new_key[32];
    if (passdoq_crypto_random(ctx, new_key, sizeof(new_key)) != 0) {
        return -1;
    }

    // Rewrap all entries (abstract API call)
    if (passdoq_vault_rekey(ctx, new_key, sizeof(new_key)) != 0) {
        return -1;
    }

    // Zero key after use
    passdoq_secure_memzero(new_key, sizeof(new_key));

    last_rotation = now;

    passdoq_log(ctx, PASSDOQ_LOG_INFO,
        "[EphemeralDataKey] Data key rotated");

    return 0;
}

/* Optional: configuration hook */
static int on_config(passdoq_ctx_t *ctx, const char *key, const char *value) {
    if (strcmp(key, "ephemeral.rotation_seconds") == 0) {
        rotation_interval = (uint64_t)strtoull(value, NULL, 10);
        return 0;
    }
    return 1; // not handled
}

/* Module descriptor */
static passdoq_module_t module = {
    .name = MODULE_NAME,
    .version = "1.0.0",
    .description = "Automatic data key rotation",
    .author = "Passdoq Team",

    .init = NULL,
    .deinit = NULL,

    .on_vault_unlock = on_vault_unlock,
    .on_vault_lock = NULL,
    .on_config = on_config,
};

/* Entry point */
PASSDOQ_MODULE_EXPORT
passdoq_module_t *passdoq_module_init(void) {
    return &module;
}
