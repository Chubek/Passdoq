#!/bin/bash
set -e

THIRD_PARTY_DIR="third_party"
NPROC=$(nproc 2>/dev/null || echo 4)

echo "Building third-party dependencies..."
echo "Using $NPROC parallel jobs"
echo ""

# Build libsodium
echo "==> Building libsodium..."
cd "$THIRD_PARTY_DIR/libsodium"
if [ ! -f "configure" ]; then
    ./autogen.sh
fi
if [ ! -f "Makefile" ]; then
    ./configure --prefix=$(pwd)/install
fi
make -j$NPROC
cd ../..
echo "✓ libsodium built"
echo ""

# Build Lua
echo "==> Building Lua..."
cd "$THIRD_PARTY_DIR/lua"
if [ ! -f "src/liblua.a" ]; then
    make -j$NPROC linux
fi
cd ../..
echo "✓ Lua built"
echo ""

# Build libltdl
echo "==> Building libltdl..."
cd "$THIRD_PARTY_DIR/libltdl"
if [ ! -f "configure" ]; then
    autoreconf -fi || echo "Warning: autoreconf failed, trying anyway..."
fi
if [ ! -f "Makefile" ]; then
    ./configure --prefix=$(pwd)/install || echo "Warning: configure may have issues"
fi
make -j$NPROC || echo "Warning: libltdl build may have issues"
cd ../..
echo "✓ libltdl built (or skipped)"
echo ""

# Build Xapian
echo "==> Building Xapian..."
cd "$THIRD_PARTY_DIR/xapian/xapian-core"
if [ ! -f "configure" ]; then
    ./bootstrap || echo "Warning: bootstrap failed"
fi
if [ ! -f "Makefile" ]; then
    ./configure --prefix=$(pwd)/install
fi
make -j$NPROC || echo "Warning: Xapian build may have issues"
cd ../../..
echo "✓ Xapian built (or skipped)"
echo ""

# Build rpclib
echo "==> Building rpclib..."
cd "$THIRD_PARTY_DIR/rpclib"
if [ ! -d "build" ]; then
    mkdir build
fi
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$NPROC
cd ../../..
echo "✓ rpclib built"
echo ""

echo "==================================="
echo "Third-party dependencies built!"
echo "==================================="
echo ""
echo "Note: Some dependencies may have been skipped if they had build issues."
echo "The CMake configuration will detect what's available."
echo ""
echo "Now you can build Passdoq:"
echo "  mkdir build && cd build"
echo "  cmake .."
echo "  make -j$NPROC"
