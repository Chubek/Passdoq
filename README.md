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

## Quick Start

### Build from Source

All dependencies are bundled in `third_party/`:

```bash
# 1. Build dependencies
./build_dependencies.sh

# 2. Build Passdoq
mkdir build && cd build
cmake ..
make -j$(nproc)

# 3. Run tests
ctest
```

For detailed build instructions, see [BUILD.md](BUILD.md).

### First Use

```bash
# Initialize vault
./build/passdoq init

# Add an entry
./build/passdoq add

# List entries
./build/passdoq list

# Search
./build/passdoq search
```

For detailed usage, see [QUICKSTART.md](QUICKSTART.md).

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

All dependencies are included in `third_party/`:

### Required
- **libsodium** - Cryptography
- **msgpack11** - Serialization (header-only)
- **CLI11** - CLI parsing (header-only)
- **PEGTL** - Parser (header-only)

### Optional
- **Xapian** - Search/indexing (Stage 3)
- **Lua** - Scripting (Stage 4)
- **Sol2** - Lua bindings (Stage 4, header-only)
- **libltdl** - Module loading (Stage 4)
- **rpclib** - RPC (Stage 5)
- **Catch2** - Testing (system package)

## Building

### Standard Build

```bash
./build_dependencies.sh
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Build Options

```bash
cmake .. \
  -DBUILD_STAGE_1=ON \   # Core vault + crypto
  -DBUILD_STAGE_2=ON \   # CLI + runcom
  -DBUILD_STAGE_3=ON \   # Indexing + search
  -DBUILD_STAGE_4=ON \   # Modules + Lua
  -DBUILD_STAGE_5=ON \   # Services + RPC
  -DBUILD_TESTS=ON       # Build tests
```

### Minimal Build (Core Only)

```bash
cd third_party/libsodium
./autogen.sh && ./configure && make -j$(nproc)
cd ../..

mkdir build && cd build
cmake .. -DBUILD_STAGE_1=ON -DBUILD_STAGE_2=OFF -DBUILD_STAGE_3=OFF -DBUILD_STAGE_4=OFF -DBUILD_STAGE_5=OFF
make -j$(nproc)
```

See [BUILD.md](BUILD.md) for complete build documentation.

## Testing

```bash
cd build
ctest
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

### Basic Operations

```bash
# Initialize vault
passdoq init

# Add entry
passdoq add

# Get entry
passdoq get

# List all entries
passdoq list

# Search
passdoq search

# Lock/unlock
passdoq lock
passdoq unlock
```

### Configuration

Create `~/.passdoqrc`:

```
vault_path = ~/.passdoq.vault
timeout = 300
auto_lock = true
```

### Modules

Load the example plugin:

```bash
# Build plugin
cd build
make ephemeral_datakey

# Configure
echo 'module_path = ./ephemeral_datakey.so' >> ~/.passdoqrc
echo 'ephemeral.rotation_seconds = 300' >> ~/.passdoqrc
```

### Lua Scripting

```bash
passdoq --script examples/scripts/auto_copy_and_mask.lua
```

### RPC Server

```bash
# Start daemon
passdoqd --port 8080
```

## Documentation

- **[BUILD.md](BUILD.md)** - Detailed build instructions
- **[QUICKSTART.md](QUICKSTART.md)** - Quick start guide
- **[IMPLEMENTATION_SUMMARY.md](IMPLEMENTATION_SUMMARY.md)** - Implementation details
- **[PROJECT_STATUS.md](PROJECT_STATUS.md)** - Project status and completion
- **[AGENTS.md](AGENTS.md)** - Architecture specification

## Project Structure

```
passdoq/
├── include/              # Public API headers
├── src/                  # Implementation
├── tests/                # Catch2 tests
├── examples/             # Example plugins and scripts
├── third_party/          # Bundled dependencies
├── docs/                 # Documentation
├── CMakeLists.txt        # Build configuration
├── build_dependencies.sh # Dependency build script
└── README.md            # This file
```

## Implementation Status

✅ **All 5 Stages Complete**

- ✅ Stage 1: Core Vault + Crypto
- ✅ Stage 2: CLI + Runcom
- ✅ Stage 3: Indexing + Search
- ✅ Stage 4: Modules + Lua
- ✅ Stage 5: Services + RPC

See [PROJECT_STATUS.md](PROJECT_STATUS.md) for details.

## Security

- Master password never stored
- Data encrypted with XSalsa20-Poly1305
- Keys derived with Argon2id
- Memory locked and zeroed
- Time-limited access
- No plaintext in logs

## Contributing

Contributions welcome! Please ensure:
- Code follows existing style
- Tests pass (`ctest`)
- New features include tests
- Security-sensitive code is auditable

## License

See LICENSE file for details.

## Support

For issues, questions, or contributions, please visit the project repository.
