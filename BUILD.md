# Building Passdoq from Source

This guide explains how to build Passdoq using the bundled third-party dependencies.

## Quick Start

```bash
# 1. Build third-party dependencies
./build_dependencies.sh

# 2. Configure and build Passdoq
mkdir build && cd build
cmake ..
make -j$(nproc)

# 3. Run tests
ctest
```

## Third-Party Dependencies

All dependencies are included in the `third_party/` directory:

- **libsodium** - Cryptography library (required)
- **msgpack11** - Serialization (required, header-only)
- **CLI11** - Command-line parsing (required, header-only)
- **PEGTL** - Parser library (required, header-only)
- **Xapian** - Search engine (optional, for Stage 3)
- **Lua** - Scripting language (optional, for Stage 4)
- **Sol2** - Lua C++ bindings (optional, header-only)
- **libltdl** - Dynamic loading (optional, for Stage 4)
- **rpclib** - RPC library (optional, for Stage 5)
- **NNG** - Messaging library (optional, future use)
- **jemalloc** - Memory allocator (optional)
- **libclipboard** - Clipboard access (optional)

## Building Dependencies

### Automated Build

Use the provided script to build all dependencies:

```bash
./build_dependencies.sh
```

This will build:
- libsodium
- Lua
- libltdl
- Xapian
- rpclib

### Manual Build

If you prefer to build dependencies manually:

#### libsodium (Required)

```bash
cd third_party/libsodium
./autogen.sh
./configure
make -j$(nproc)
cd ../..
```

#### Lua (Optional)

```bash
cd third_party/lua
make linux -j$(nproc)
cd ../..
```

#### Xapian (Optional)

```bash
cd third_party/xapian/xapian-core
./bootstrap
./configure
make -j$(nproc)
cd ../../..
```

#### rpclib (Optional)

```bash
cd third_party/rpclib
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
cd ../../..
```

#### libltdl (Optional)

```bash
cd third_party/libltdl
autoreconf -fi
./configure
make -j$(nproc)
cd ../..
```

## Building Passdoq

### Standard Build

```bash
mkdir build
cd build
cmake ..
make -j$(nproc)
```

### Build Options

Control which stages to build:

```bash
cmake .. \
  -DBUILD_STAGE_1=ON \   # Core vault + crypto
  -DBUILD_STAGE_2=ON \   # CLI + runcom
  -DBUILD_STAGE_3=ON \   # Indexing + search
  -DBUILD_STAGE_4=ON \   # Modules + Lua
  -DBUILD_STAGE_5=ON \   # Services + RPC
  -DBUILD_TESTS=ON       # Build tests
```

### Debug Build

```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)
```

### Release Build

```bash
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

## Testing

### Run All Tests

```bash
cd build
ctest
```

### Run Specific Test Suite

```bash
./build/test_stage1  # Core tests
./build/test_stage2  # CLI tests
./build/test_stage3  # Search tests
./build/test_stage4  # Module tests
./build/test_stage5  # Service tests
```

### Verbose Test Output

```bash
ctest --verbose
```

## Installation

```bash
cd build
sudo make install
```

Default installation paths:
- Binaries: `/usr/local/bin/`
- Libraries: `/usr/local/lib/`
- Headers: `/usr/local/include/passdoq/`
- Modules: `/usr/local/lib/passdoq/modules/`
- Scripts: `/usr/local/share/passdoq/scripts/`

### Custom Install Prefix

```bash
cmake .. -DCMAKE_INSTALL_PREFIX=/opt/passdoq
make install
```

## Troubleshooting

### Dependency Not Found

If CMake can't find a dependency:

1. Check if it's built:
   ```bash
   ls third_party/libsodium/src/libsodium/.libs/libsodium.a
   ls third_party/lua/src/liblua.a
   ```

2. Rebuild the dependency:
   ```bash
   cd third_party/[dependency]
   make clean
   make -j$(nproc)
   ```

3. Reconfigure CMake:
   ```bash
   cd build
   rm -rf *
   cmake ..
   ```

### Build Fails

Check the configuration summary:

```bash
cmake .. 2>&1 | grep -A 20 "Configuration Summary"
```

This shows which dependencies were found.

### Missing System Dependencies

Some third-party libraries may need system packages:

**Ubuntu/Debian:**
```bash
sudo apt-get install \
    build-essential \
    autoconf automake libtool \
    pkg-config \
    zlib1g-dev
```

**Fedora/RHEL:**
```bash
sudo dnf install \
    gcc gcc-c++ make \
    autoconf automake libtool \
    pkgconfig \
    zlib-devel
```

### Tests Fail

1. Ensure all dependencies are built
2. Check test output for specific errors:
   ```bash
   ctest --output-on-failure
   ```

3. Run individual tests:
   ```bash
   ./build/test_stage1 --success
   ```

## Minimal Build

To build only the core functionality (Stage 1):

```bash
# Build only libsodium
cd third_party/libsodium
./autogen.sh && ./configure && make -j$(nproc)
cd ../..

# Build Passdoq Stage 1 only
mkdir build && cd build
cmake .. \
  -DBUILD_STAGE_1=ON \
  -DBUILD_STAGE_2=OFF \
  -DBUILD_STAGE_3=OFF \
  -DBUILD_STAGE_4=OFF \
  -DBUILD_STAGE_5=OFF
make -j$(nproc)
```

## Full Build

To build everything with all features:

```bash
# Build all dependencies
./build_dependencies.sh

# Build Passdoq with all stages
mkdir build && cd build
cmake .. \
  -DBUILD_STAGE_1=ON \
  -DBUILD_STAGE_2=ON \
  -DBUILD_STAGE_3=ON \
  -DBUILD_STAGE_4=ON \
  -DBUILD_STAGE_5=ON \
  -DBUILD_TESTS=ON \
  -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

## Cross-Compilation

For cross-compilation, you'll need to build dependencies for the target platform first, then:

```bash
cmake .. \
  -DCMAKE_TOOLCHAIN_FILE=/path/to/toolchain.cmake \
  -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

## Development Build

For active development with debug symbols and no optimization:

```bash
mkdir build-dev && cd build-dev
cmake .. \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
make -j$(nproc)
```

The `compile_commands.json` file can be used with IDEs and language servers.

## Clean Build

To start fresh:

```bash
# Clean Passdoq build
rm -rf build
mkdir build && cd build
cmake ..
make -j$(nproc)

# Clean dependencies (if needed)
cd third_party/libsodium && make clean && cd ../..
cd third_party/lua && make clean && cd ../..
# etc.
```

## Build Artifacts

After a successful build, you'll have:

```
build/
├── passdoq                    # Main CLI executable
├── passdoqd                   # Daemon executable
├── ephemeral_datakey.so       # Example plugin
├── libpassdoq_stage1.a        # Stage 1 library
├── libpassdoq_stage2.a        # Stage 2 library
├── libpassdoq_stage3.a        # Stage 3 library
├── libpassdoq_stage4.a        # Stage 4 library
├── libpassdoq_stage5.a        # Stage 5 library
├── test_stage1                # Test executables
├── test_stage2
├── test_stage3
├── test_stage4
└── test_stage5
```

## Next Steps

After building:

1. Run tests: `ctest`
2. Try the CLI: `./build/passdoq --help`
3. Read the user guide: `README.md`
4. Check the quick start: `QUICKSTART.md`
