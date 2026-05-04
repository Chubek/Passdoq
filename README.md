# Passdoq - CLI Password Manager

Passdoq is a secure, extensible CLI-based password and token manager written in C++. It features strong encryption, modular architecture, Lua scripting support, and RPC capabilities.

## Features

### Core Security (Stage 1)
- **Argon2id Key Derivation**: Memory-hard password hashing
- **XSalsa20-Poly1305 Encryption**: Authenticated encryption via libsodium
- **Secure Memory Management**: Memory locking and automatic zeroing
- **MessagePack Serialization**: Efficient binary vault format

### CLI Interface (Stage 2)
- **Intuitive Commands**: add, get, list, search, update, delete, lock/unlock
- **Configuration System**: PRL (Passdoq Runcom Language) with includes
- **Multiple Config Locations**: `~/.passdoqrc`, `$XDG_CONFIG_HOME/passdoq/Passdoq.cnf`

### Search & Indexing (Stage 3)
- **Xapian Integration**: Fast full-text search without decrypting vault
- **Tag-Based Search**: Organize entries with tags
- **Metadata Indexing**: Search by custom metadata fields

### Extensibility (Stage 4)
- **Dynamic Modules**: Load plugins via libltdl
- **Lua Scripting**: Extend functionality with Lua addons
- **Module API**: C API for writing custom modules
- **Example Plugin**: EphemeralDataKey for automatic key rotation

### Services & RPC (Stage 5)
- **Service Framework**: Pluggable service architecture
- **Daemon Mode**: Background service (`passdoqd`)
- **RPC Server**: Remote vault access via rpclib
- **Secret Delivery**: Multiple delivery methods (clipboard, file, stdout, memory)

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                         CLI / RPC Client                     │
├─────────────────────────────────────────────────────────────┤
│  Runcom  │  Search  │  Modules  │  Lua  │  Services  │ RPC  │
├─────────────────────────────────────────────────────────────┤
│              Vault Core (Encryption + Storage)               │
├─────────────────────────────────────────────────────────────┤
│         Crypto (libsodium)  │  Secure Memory (mlock)        │
└─────────────────────────────────────────────────────────────┘
```

## Dependencies

### Required
- libsodium (crypto)
- msgpack (serialization)
- C++17 compiler

### Optional
- CLI11 (CLI interface)
- Xapian (search/indexing)
- Lua 5.3+ (scripting)
- Sol2 (Lua bindings)
- libltdl (module loading)
- rpclib (RPC)
- Catch2 v3 (testing)

## Building

### CMake

```bash
mkdir build && cd build
cmake ..
make
```

### Build Options

```bash
cmake -DBUILD_STAGE_1=ON \   # Core vault + crypto
      -DBUILD_STAGE_2=ON \   # CLI + runcom
      -DBUILD_STAGE_3=ON \   # Indexing + search
      -DBUILD_STAGE_4=ON \   # Modules + Lua
      -DBUILD_STAGE_5=ON \   # Services + RPC
      -DBUILD_TESTS=ON ..
```

### Running Tests

```bash
ctest --test-dir build
```

Or run individual test suites:

```bash
./build/test_stage1  # Core tests
./build/test_stage2  # CLI tests
./build/test_stage3  # Search tests
./build/test_stage4  # Module tests
./build/test_stage5  # Service tests
```

## Usage

### Initialize Vault

```bash
passdoq init
```

### Add Entry

```bash
passdoq add
# Follow prompts for name, username, secret, tags
```

### Get Entry

```bash
passdoq get
# Enter entry ID
```

### Search

```bash
passdoq search
# Enter search query
```

### List All Entries

```bash
passdoq list
```

### Lock/Unlock

```bash
passdoq lock
passdoq unlock
```

## Configuration

Create `~/.passdoqrc`:

```
vault_path = /home/user/.passdoq.vault
timeout = 300
auto_lock = true
```

Or use `$XDG_CONFIG_HOME/passdoq/Passdoq.cnf`.

## Modules

### Loading Modules

Modules are loaded automatically from `/usr/lib/passdoq/modules/` or can be specified in config:

```
module_path = /path/to/module.so
```

### Example: EphemeralDataKey

Automatically rotates data keys on vault unlock:

```c
// Configured via runcom
ephemeral.rotation_seconds = 300
```

## Lua Scripting

### Example: Auto Copy and Mask

```lua
local passdoq = lpassdoq.init_addon({
    name = "AutoCopyAndMask",
    author = "Your Name"
})

passdoq.on("secret_retrieved", function(entry)
    passdoq.clipboard_set(entry.value)
    passdoq.print("Secret copied to clipboard")
end)
```

Load scripts:

```bash
passdoq --script /path/to/script.lua
```

## RPC Server

### Start Daemon

```bash
passdoqd --port 8080
```

### Remote Access

```cpp
#include <passdoq/remote.hpp>

passdoq::remote::RpcClient client("localhost", 8080);
client.connect();
client.unlock("password");

std::string id = client.add_entry("GitHub", "user", "secret");
```

## Security Considerations

- **Master Password**: Never stored, only used for key derivation
- **Data Key**: Encrypted with master key, stored in vault header
- **Memory Protection**: Sensitive data locked in memory, zeroed on cleanup
- **Time-Limited Access**: Automatic vault locking after timeout
- **No Plaintext Logs**: Secrets never written to logs

## Project Structure

```
passdoq/
├── include/
│   ├── passdoq-api.h       # Public C API
│   └── passdoq-module.h    # Module interface
├── src/
│   ├── crypto.*            # Encryption layer
│   ├── memory.*            # Secure memory
│   ├── vault.*             # Vault core
│   ├── cli.*               # CLI interface
│   ├── runcom.*            # Config system
│   ├── indexing.*          # Xapian indexing
│   ├── search.*            # Search engine
│   ├── modules.*           # Module loader
│   ├── lua.*               # Lua embedding
│   ├── services.*          # Service framework
│   ├── daemon.*            # Daemon
│   ├── delivery.*          # Secret delivery
│   └── remote.*            # RPC
├── tests/                  # Catch2 tests
├── examples/
│   ├── plugins/            # Example modules
│   └── scripts/            # Example Lua scripts
└── docs/                   # Documentation
```

## Implementation Stages

All 5 stages have been implemented:

✅ **Stage 1**: Core Vault + Crypto (Foundation)  
✅ **Stage 2**: CLI + Runcom (Usable local tool)  
✅ **Stage 3**: Indexing + Search (Fast metadata queries)  
✅ **Stage 4**: Extensibility (Modules + Lua)  
✅ **Stage 5**: Services + RPC (Distributed capabilities)

## Testing

Each stage includes comprehensive Catch2 tests:

- **Stage 1**: Crypto, memory, vault operations
- **Stage 2**: Runcom parsing, CLI commands
- **Stage 3**: Indexing, search queries
- **Stage 4**: Module loading, Lua scripting
- **Stage 5**: Services, daemon, delivery methods

## License

See LICENSE file for details.

## Contributing

Contributions welcome! Please ensure:
- Code follows existing style
- Tests pass (`ctest`)
- New features include tests
- Security-sensitive code is auditable

## Roadmap

- [ ] GUI frontend
- [ ] Browser extension
- [ ] Mobile apps
- [ ] Cloud sync (encrypted)
- [ ] Hardware token support (YubiKey)
- [ ] Biometric authentication

## Support

For issues, questions, or contributions, please visit the project repository.
