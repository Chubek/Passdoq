# Passdoq

**A secure, extensible CLI password manager with scripting and remote access capabilities**

Passdoq is a C++ password manager designed for developers and power users who need secure credential storage with programmability. It combines military-grade encryption with a plugin architecture, embedded scripting, and network transparency.

---

## What Makes Passdoq Different

**Security First**
- Master key derived via Argon2id (memory-hard, GPU-resistant)
- Secrets encrypted with XSalsa20-Poly1305 (authenticated encryption)
- Memory-locked buffers automatically zeroed after use
- On-demand decryption with configurable timeout
- No plaintext secrets in logs or swap

**Programmable**
- C plugin API for extending core functionality
- Embedded Lua runtime with full vault access
- Service framework for background automation
- RPC server for remote vault operations

**Fast Search**
- Xapian-powered full-text indexing
- Search metadata without decrypting vault
- Tag-based organization
- Query language for complex filters

**Flexible Delivery**
- Copy to clipboard with auto-clear
- Write to file or stdout
- IPC to other processes
- Custom delivery via plugins

---

## Quick Start

```bash
# Build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j

# Initialize vault
./passdoq init

# Add a password
./passdoq add github.com

# Retrieve it
./passdoq get github.com

# Search
./passdoq search github
```

---

## Core Features

### Encryption & Storage

Passdoq uses libsodium for all cryptographic operations:

- **Key Derivation**: Argon2id with configurable memory/time parameters
- **Encryption**: XSalsa20-Poly1305 (secretbox) for authenticated encryption
- **Serialization**: MessagePack for compact binary vault format
- **Storage**: LMDBX for metadata and index persistence

The vault file is a single encrypted blob. Metadata (tags, timestamps, entry names) is indexed separately for fast search without full decryption.

### CLI Interface

Built with CLI11, the interface supports:

- Standard CRUD operations (add, get, update, delete, list)
- Search with query syntax
- Vault locking with timeout
- Password generation with policies
- Configuration via runcom files (PRL syntax, preprocessed with ucpp)

### Search & Indexing

Xapian indexes entry metadata, enabling:

- Full-text search across tags and metadata
- Boolean queries (AND, OR, NOT)
- Prefix matching and wildcards
- No vault decryption required for search

### Extensibility

**Modules (C Plugins)**

Plugins are shared libraries loaded via libltdl. They can hook into vault lifecycle events:

```c
// Hook into vault unlock
static int on_vault_unlock(passdoq_ctx_t *ctx) {
    // Rotate data keys, log access, etc.
    return 0;
}
```

Example plugin: `ephemeral_datakey` rotates encryption keys on a schedule.

**Lua Scripting**

Embedded Lua runtime (via Sol2) exposes vault operations:

```lua
-- Auto-copy to clipboard and mask output
passdoq.on("secret_retrieved", function(entry)
    passdoq.clipboard_set(entry.value)
    passdoq.print("Retrieved: " .. mask(entry.value))
    passdoq.defer(20, function()
        passdoq.clipboard_clear()
    end)
end)
```

Example script: `auto_copy_and_mask.lua` automatically copies secrets to clipboard with auto-clear.

### Services & RPC

**Daemon Mode**

`passdoqd` runs as a background service, managing:

- Long-running vault sessions
- IPC via NNG (nanomsg-next-gen)
- Service lifecycle (start, stop, reload)
- Background automation tasks

**RPC Server**

Remote vault access via rpclib:

```bash
# Start RPC server
passdoqd --rpc --port 8080

# Connect from another machine
passdoq --remote https://vault.example.com:8080 get github.com
```

---

## Configuration

Passdoq reads configuration from:

1. `~/.passdoqrc`
2. `$XDG_CONFIG_HOME/passdoq/Passdoq.cnf`
3. `$PASSDOQ_RUNCOM_FILE`

Configuration uses PRL (Passdoq Runcom Language), preprocessed with ucpp:

```prl
# Vault settings
vault_path = ~/.passdoq.vault
timeout = 300

# Modules
#include "modules.conf"
module_path = /usr/lib/passdoq/ephemeral_datakey.so

# Lua scripts
lua_script = ~/.config/passdoq/auto_copy.lua
```

---

## Building

### Dependencies

**Required:**
- libsodium (crypto)
- Lua 5.4+ (scripting)
- Xapian (search)
- CMake 3.5+
- C++20 compiler

**Bundled in `third_party/`:**
- CLI11 (CLI parsing)
- Sol2 (Lua bindings)
- msgpack11 (serialization)
- PEGTL (parsing)
- nng (IPC)
- rpclib (RPC)
- libmdbx (storage)
- libltdl (module loading)
- libclipboard (clipboard)

### Build Steps

```bash
mkdir build && cd build
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DPASSDOQ_BUILD_EXAMPLES=ON \
  -DPASSDOQ_BUILD_TESTS=ON
make -j
sudo make install
```

### Build Options

- `PASSDOQ_BUILD_TESTS` - Build test suite (default: ON)
- `PASSDOQ_BUILD_DOCS` - Build Doxygen documentation (default: ON)
- `PASSDOQ_BUILD_EXAMPLES` - Build example plugins and scripts (default: ON)
- `PASSDOQ_USE_JEMALLOC` - Use jemalloc for memory allocation (default: OFF)

---

## Architecture

```
┌──────────────────────────────────────────────────┐
│  CLI / RPC Client                                │
├──────────────────────────────────────────────────┤
│  Runcom │ Search │ Modules │ Lua │ Services      │
├──────────────────────────────────────────────────┤
│  Vault Core (Encryption + Serialization)         │
├──────────────────────────────────────────────────┤
│  Crypto (libsodium) │ Secure Memory (mlock)      │
└──────────────────────────────────────────────────┘
```

See `AGENTS.md` for detailed architecture documentation.

---

## Security Model

- **Master password**: Never stored, only used to derive keys
- **Key derivation**: Argon2id with 64MB memory, 3 iterations
- **Encryption**: XSalsa20-Poly1305 (authenticated encryption)
- **Memory protection**: mlock() + sodium_mprotect() + automatic zeroing
- **Session timeout**: Configurable auto-lock after inactivity
- **Audit trail**: Optional logging of vault access (metadata only)

---

## License

See `LICENSE` for details.
