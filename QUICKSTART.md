# Passdoq Quick Start Guide

## Prerequisites

Install dependencies (Ubuntu/Debian):

```bash
sudo apt-get install \
    build-essential cmake \
    libsodium-dev \
    libmsgpack-dev \
    libxapian-dev \
    liblua5.3-dev \
    libltdl-dev \
    catch2
```

## Build

```bash
# Clone repository (if applicable)
cd passdoq

# Create build directory
mkdir build && cd build

# Configure
cmake ..

# Build
make -j$(nproc)

# Run tests
ctest
```

## First Use

### 1. Initialize Vault

```bash
./passdoq init
# Enter master password when prompted
# Confirm password
```

### 2. Add Your First Entry

```bash
./passdoq add
# Name: GitHub
# Username: myusername
# Secret: (enter password)
# Tags: work,dev
```

### 3. List Entries

```bash
./passdoq list
```

### 4. Search

```bash
./passdoq search
# Search query: GitHub
```

### 5. Get Entry

```bash
./passdoq get
# Entry ID: (paste ID from list)
```

## Configuration

Create `~/.passdoqrc`:

```bash
cat > ~/.passdoqrc << 'CONF'
vault_path = ~/.passdoq.vault
timeout = 300
auto_lock = true
CONF
```

## Using Modules

### Load Example Plugin

```bash
# Build the example plugin
cd build
make ephemeral_datakey

# Configure to use it
echo 'module_path = ./ephemeral_datakey.so' >> ~/.passdoqrc
echo 'ephemeral.rotation_seconds = 300' >> ~/.passdoqrc
```

## Using Lua Scripts

### Run Example Script

```bash
./passdoq --script ../examples/scripts/auto_copy_and_mask.lua
```

### Create Your Own Script

```lua
-- my_addon.lua
local passdoq = lpassdoq.init_addon({
    name = "MyAddon",
    author = "Your Name"
})

passdoq.on("secret_retrieved", function(entry)
    passdoq.print("Retrieved: " .. entry.name)
end)
```

## Running as Daemon

```bash
# Start daemon
./passdoqd --port 8080

# In another terminal, use RPC client
# (requires writing a client program)
```

## Testing Individual Stages

```bash
# Test Stage 1 (Core)
./test_stage1

# Test Stage 2 (CLI)
./test_stage2

# Test Stage 3 (Search)
./test_stage3

# Test Stage 4 (Modules)
./test_stage4

# Test Stage 5 (Services)
./test_stage5
```

## Common Operations

### Lock Vault

```bash
./passdoq lock
```

### Unlock Vault

```bash
./passdoq unlock
# Enter password
```

### Update Entry

```bash
./passdoq update
# Entry ID: (paste ID)
# Follow prompts
```

### Delete Entry

```bash
./passdoq delete
# Entry ID: (paste ID)
# Confirm: yes
```

### View Configuration

```bash
./passdoq config
```

## Troubleshooting

### Build Fails

Check dependencies:
```bash
pkg-config --modversion libsodium
pkg-config --modversion msgpack
```

### Tests Fail

Run with verbose output:
```bash
ctest --verbose
```

### Vault Won't Unlock

- Check password is correct
- Verify vault file exists: `ls -la ~/.passdoq.vault`
- Check file permissions

### Module Won't Load

- Verify module path in config
- Check module file exists and is executable
- Look for error messages in output

## Security Tips

1. **Strong Master Password**: Use a long, unique password
2. **Backup Vault**: Regularly backup `~/.passdoq.vault`
3. **Secure Config**: Set proper permissions on config files
   ```bash
   chmod 600 ~/.passdoqrc
   chmod 600 ~/.passdoq.vault
   ```
4. **Lock When Done**: Always lock vault after use
5. **Timeout**: Configure auto-lock timeout

## Next Steps

- Read full documentation in `README.md`
- Explore example plugins in `examples/plugins/`
- Try Lua scripting with `examples/scripts/`
- Review implementation details in `IMPLEMENTATION_SUMMARY.md`

## Getting Help

- Check `README.md` for detailed documentation
- Review test files for usage examples
- Examine source code for API details
- Consult `AGENTS.md` for architecture overview
