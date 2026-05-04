cmake_minimum_required(VERSION 3.5)

project(Passdoq
    VERSION 1.0
    DESCRIPTION "CLI-based password manager"
    LANGUAGES C CXX
)

# ---------------------------
# Options
# ---------------------------
option(PASSDOQ_BUILD_TESTS "Build tests" ON)
option(PASSDOQ_BUILD_DOCS  "Build documentation" ON)
option(PASSDOQ_BUILD_EXAMPLES "Build examples" ON)
option(PASSDOQ_USE_JEMALLOC "Use jemalloc" OFF)

# ---------------------------
# Global settings
# ---------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# ---------------------------
# Third-party dependencies
# ---------------------------

# CLI11 (header-only)
add_library(CLI11 INTERFACE)
target_include_directories(CLI11 INTERFACE
    ${CMAKE_SOURCE_DIR}/third_party/CLI11/include
)

# libsodium
find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(SODIUM REQUIRED libsodium)
else()
    # Fallback: assume system install
    find_library(SODIUM_LIBRARIES NAMES sodium)
    find_path(SODIUM_INCLUDE_DIRS NAMES sodium.h)
    if(NOT SODIUM_LIBRARIES OR NOT SODIUM_INCLUDE_DIRS)
        message(FATAL_ERROR "libsodium not found")
    endif()
endif()

# Lua
find_package(Lua REQUIRED)

# Sol2 (header-only)
add_library(sol2 INTERFACE)
target_include_directories(sol2 INTERFACE
    ${CMAKE_SOURCE_DIR}/third_party/sol2/include
)
target_link_libraries(sol2 INTERFACE ${LUA_LIBRARIES})
target_include_directories(sol2 INTERFACE ${LUA_INCLUDE_DIR})

# msgpack11
add_subdirectory(third_party/msgpack11)

# PEGTL (header-only)
add_library(PEGTL INTERFACE)
target_include_directories(PEGTL INTERFACE
    ${CMAKE_SOURCE_DIR}/third_party/PEGTL/include
)

# nng
add_subdirectory(third_party/nng)

# rpclib
add_subdirectory(third_party/rpclib)

# libmdbx
add_subdirectory(third_party/libmdbx)

# Xapian
find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(XAPIAN REQUIRED xapian-core)
else()
    find_library(XAPIAN_LIBRARIES NAMES xapian)
    find_path(XAPIAN_INCLUDE_DIRS NAMES xapian.h)
    if(NOT XAPIAN_LIBRARIES OR NOT XAPIAN_INCLUDE_DIRS)
        message(FATAL_ERROR "Xapian not found")
    endif()
endif()

# libltdl
add_subdirectory(third_party/libltdl)

# ucpp (preprocessor - typically built separately)
# Assuming it provides ucpp executable or library
find_program(UCPP_EXECUTABLE ucpp
    PATHS ${CMAKE_SOURCE_DIR}/third_party/ucpp
    NO_DEFAULT_PATH
)
if(NOT UCPP_EXECUTABLE)
    message(WARNING "ucpp not found - runcom preprocessing may not work")
endif()

# libclipboard (has CMake)
add_subdirectory(third_party/libclipboard)

# jemalloc (optional, autotools-based)
# Note: jemalloc uses autotools, not CMake
# Users should build it separately and link via find_library
if(PASSDOQ_USE_JEMALLOC)
    find_library(JEMALLOC_LIBRARY NAMES jemalloc
        PATHS ${CMAKE_SOURCE_DIR}/third_party/jemalloc/lib
        NO_DEFAULT_PATH
    )
    if(NOT JEMALLOC_LIBRARY)
        message(WARNING "jemalloc requested but not found - build it separately first")
        set(PASSDOQ_USE_JEMALLOC OFF)
    endif()
endif()

# Threads
find_package(Threads REQUIRED)

# ---------------------------
# Core library
# ---------------------------
add_library(passdoq_core
    src/crypto.cpp
    src/daemon.cpp
    src/delivery.cpp
    src/indexing.cpp
    src/lua.cpp
    src/memory.cpp
    src/modules.cpp
    src/pssl.cpp
    src/remote.cpp
    src/runcom.cpp
    src/search.cpp
    src/services.cpp
    src/vault.cpp
)

target_include_directories(passdoq_core
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${SODIUM_INCLUDE_DIRS}
        ${XAPIAN_INCLUDE_DIRS}
)

target_link_libraries(passdoq_core
    PUBLIC
        CLI11
        Threads::Threads
        clipboard
        ${SODIUM_LIBRARIES}
        ${LUA_LIBRARIES}
        sol2
        msgpack11
        PEGTL
        nng
        rpclib
        mdbx
        ${XAPIAN_LIBRARIES}
        ltdl
)

if(PASSDOQ_USE_JEMALLOC)
    target_link_libraries(passdoq_core PUBLIC ${JEMALLOC_LIBRARY})
endif()

target_compile_options(passdoq_core PRIVATE -Wall -Wextra)

# ---------------------------
# C API library
# ---------------------------
# C API library (optional - only if source exists)
if(EXISTS ${CMAKE_SOURCE_DIR}/src/api.c)
    add_library(passdoq_api SHARED
        src/api.c
    )

    target_include_directories(passdoq_api
        PUBLIC
            ${CMAKE_SOURCE_DIR}/include
    )

    target_link_libraries(passdoq_api
        PRIVATE passdoq_core
    )

    set_target_properties(passdoq_api PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION 1
        PUBLIC_HEADER include/passdoq-api.h
    )
    
    set(PASSDOQ_HAS_API_LIB TRUE)
else()
    message(STATUS "src/api.c not found - skipping C API library")
    set(PASSDOQ_HAS_API_LIB FALSE)
endif()

# ---------------------------
# CLI executable
# ---------------------------
add_executable(passdoq
    src/main.cpp
    src/cli.cpp
)

target_link_libraries(passdoq
    PRIVATE passdoq_core
)

# ---------------------------
# Daemon executable
# ---------------------------
# Daemon executable (optional - only if source exists)
if(EXISTS ${CMAKE_SOURCE_DIR}/src/daemon_main.cpp)
    add_executable(passdoqd
        src/daemon_main.cpp
    )

    target_link_libraries(passdoqd
        PRIVATE passdoq_core
    )
    
    set(PASSDOQ_HAS_DAEMON TRUE)
else()
    message(STATUS "src/daemon_main.cpp not found - skipping daemon executable")
    set(PASSDOQ_HAS_DAEMON FALSE)
endif()

# ---------------------------
# Examples
# ---------------------------
if(PASSDOQ_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# ---------------------------
# Tests
# ---------------------------
if(PASSDOQ_BUILD_TESTS)
    enable_testing()

    file(GLOB TEST_SOURCES tests/*.cpp)

    foreach(test_src ${TEST_SOURCES})
        get_filename_component(test_name ${test_src} NAME_WE)

        add_executable(${test_name} ${test_src})

        target_link_libraries(${test_name}
            PRIVATE passdoq_core
        )

        add_test(NAME ${test_name} COMMAND ${test_name})
    endforeach()
endif()

# ---------------------------
# Docs
# ---------------------------
if(PASSDOQ_BUILD_DOCS)
    add_subdirectory(docs)
endif()

# ---------------------------
# Install
# ---------------------------
set(INSTALL_TARGETS passdoq passdoq_core)

if(PASSDOQ_HAS_DAEMON)
    list(APPEND INSTALL_TARGETS passdoqd)
endif()

if(PASSDOQ_HAS_API_LIB)
    list(APPEND INSTALL_TARGETS passdoq_api)
endif()

install(TARGETS ${INSTALL_TARGETS}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    PUBLIC_HEADER DESTINATION include
)

install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)
