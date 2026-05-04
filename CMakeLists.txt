cmake_minimum_required(VERSION 3.15)
project(passdoq VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Options
option(BUILD_TESTS "Build tests" ON)
option(BUILD_STAGE_1 "Build Stage 1: Core Vault + Crypto" ON)
option(BUILD_STAGE_2 "Build Stage 2: CLI + Runcom" ON)
option(BUILD_STAGE_3 "Build Stage 3: Indexing + Search" ON)
option(BUILD_STAGE_4 "Build Stage 4: Modules + Lua" ON)
option(BUILD_STAGE_5 "Build Stage 5: Services + RPC" ON)

find_package(Threads REQUIRED)

# Third-party dependencies from third_party/
set(THIRD_PARTY_DIR ${CMAKE_SOURCE_DIR}/third_party)

# libsodium
set(SODIUM_INCLUDE_DIR ${THIRD_PARTY_DIR}/libsodium/src/libsodium/include)
set(SODIUM_LIBRARY_DIR ${THIRD_PARTY_DIR}/libsodium/src/libsodium/.libs)
find_library(SODIUM_LIBRARY sodium PATHS ${SODIUM_LIBRARY_DIR} NO_DEFAULT_PATH)
if(NOT SODIUM_LIBRARY)
    message(STATUS "libsodium not built, will need to build it first")
    set(SODIUM_LIBRARY sodium)
endif()

# msgpack11
set(MSGPACK_INCLUDE_DIR ${THIRD_PARTY_DIR}/msgpack11)

# CLI11
set(CLI11_INCLUDE_DIR ${THIRD_PARTY_DIR}/CLI11/include)

# PEGTL
set(PEGTL_INCLUDE_DIR ${THIRD_PARTY_DIR}/PEGTL/include)

# Xapian
set(XAPIAN_INCLUDE_DIR ${THIRD_PARTY_DIR}/xapian/xapian-core/include)
set(XAPIAN_LIBRARY_DIR ${THIRD_PARTY_DIR}/xapian/xapian-core/.libs)
find_library(XAPIAN_LIBRARY xapian PATHS ${XAPIAN_LIBRARY_DIR} NO_DEFAULT_PATH)
if(XAPIAN_LIBRARY)
    set(XAPIAN_FOUND TRUE)
else()
    message(WARNING "Xapian not built, Stage 3 will not build")
    set(XAPIAN_FOUND FALSE)
endif()

# Lua
set(LUA_INCLUDE_DIR ${THIRD_PARTY_DIR}/lua/src)
set(LUA_LIBRARY_DIR ${THIRD_PARTY_DIR}/lua/src)
find_library(LUA_LIBRARY lua PATHS ${LUA_LIBRARY_DIR} NO_DEFAULT_PATH)
if(LUA_LIBRARY)
    set(LUA_FOUND TRUE)
else()
    message(WARNING "Lua not built, Lua scripting will not be available")
    set(LUA_FOUND FALSE)
endif()

# Sol2
set(SOL2_INCLUDE_DIR ${THIRD_PARTY_DIR}/sol2/include)

# libltdl
set(LTDL_INCLUDE_DIR ${THIRD_PARTY_DIR}/libltdl)
set(LTDL_LIBRARY_DIR ${THIRD_PARTY_DIR}/libltdl/.libs)
find_library(LTDL_LIBRARY ltdl PATHS ${LTDL_LIBRARY_DIR} NO_DEFAULT_PATH)
if(NOT LTDL_LIBRARY)
    message(WARNING "libltdl not built, module loading will not be available")
endif()

# rpclib
set(RPCLIB_INCLUDE_DIR ${THIRD_PARTY_DIR}/rpclib/include)
set(RPCLIB_LIBRARY_DIR ${THIRD_PARTY_DIR}/rpclib/build)
find_library(RPCLIB_LIBRARY rpc PATHS ${RPCLIB_LIBRARY_DIR} NO_DEFAULT_PATH)
if(NOT RPCLIB_LIBRARY)
    message(WARNING "rpclib not built, RPC functionality will not be available")
endif()

# NNG (for future IPC)
set(NNG_INCLUDE_DIR ${THIRD_PARTY_DIR}/nng/include)
set(NNG_LIBRARY_DIR ${THIRD_PARTY_DIR}/nng/build)

# jemalloc (optional)
set(JEMALLOC_INCLUDE_DIR ${THIRD_PARTY_DIR}/jemalloc/include)

# libclipboard (optional)
set(LIBCLIPBOARD_INCLUDE_DIR ${THIRD_PARTY_DIR}/libclipboard/include)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
    ${SODIUM_INCLUDE_DIR}
    ${MSGPACK_INCLUDE_DIR}
    ${CLI11_INCLUDE_DIR}
    ${PEGTL_INCLUDE_DIR}
    ${XAPIAN_INCLUDE_DIR}
    ${LUA_INCLUDE_DIR}
    ${SOL2_INCLUDE_DIR}
    ${LTDL_INCLUDE_DIR}
    ${RPCLIB_INCLUDE_DIR}
    ${NNG_INCLUDE_DIR}
    ${JEMALLOC_INCLUDE_DIR}
    ${LIBCLIPBOARD_INCLUDE_DIR}
)

# Link directories
link_directories(
    ${SODIUM_LIBRARY_DIR}
    ${XAPIAN_LIBRARY_DIR}
    ${LUA_LIBRARY_DIR}
    ${LTDL_LIBRARY_DIR}
    ${RPCLIB_LIBRARY_DIR}
    ${NNG_LIBRARY_DIR}
)

# Stage 1: Core Vault + Crypto
if(BUILD_STAGE_1)
    add_library(passdoq_stage1 STATIC
        src/crypto.cpp
        src/memory.cpp
        src/vault.cpp
    )
    
    target_link_libraries(passdoq_stage1
        ${SODIUM_LIBRARY}
    )
    
    # Tests for Stage 1
    if(BUILD_TESTS)
        find_package(Catch2 3 QUIET)
        
        if(Catch2_FOUND)
            add_executable(test_stage1
                tests/test_crypto.cpp
                tests/test_memory.cpp
                tests/test_vault.cpp
            )
            
            target_link_libraries(test_stage1
                passdoq_stage1
                Catch2::Catch2WithMain
            )
            
            enable_testing()
            add_test(NAME Stage1Tests COMMAND test_stage1)
        else()
            message(WARNING "Catch2 not found, tests will not be built")
        endif()
    endif()
endif()

# Stage 2: CLI + Runcom
if(BUILD_STAGE_2 AND BUILD_STAGE_1)
    add_library(passdoq_stage2 STATIC
        src/runcom.cpp
        src/cli.cpp
    )
    
    target_link_libraries(passdoq_stage2
        passdoq_stage1
    )
    
    # Main executable
    add_executable(passdoq
        src/main.cpp
    )
    
    target_link_libraries(passdoq
        passdoq_stage2
        passdoq_stage1
    )
    
    # Tests for Stage 2
    if(BUILD_TESTS)
        find_package(Catch2 3 QUIET)
        
        if(Catch2_FOUND)
            add_executable(test_stage2
                tests/test_runcom.cpp
            )
            
            target_link_libraries(test_stage2
                passdoq_stage2
                passdoq_stage1
                Catch2::Catch2WithMain
            )
            
            add_test(NAME Stage2Tests COMMAND test_stage2)
        endif()
    endif()
endif()

# Stage 3: Indexing + Search
if(BUILD_STAGE_3 AND BUILD_STAGE_1 AND XAPIAN_FOUND)
    add_library(passdoq_stage3 STATIC
        src/indexing.cpp
        src/search.cpp
    )
    
    target_link_libraries(passdoq_stage3
        passdoq_stage1
        ${XAPIAN_LIBRARY}
    )
    
    # Link stage3 to main executable if stage2 is built
    if(BUILD_STAGE_2)
        target_link_libraries(passdoq
            passdoq_stage3
        )
    endif()
    
    # Tests for Stage 3
    if(BUILD_TESTS)
        find_package(Catch2 3 QUIET)
        
        if(Catch2_FOUND)
            add_executable(test_stage3
                tests/test_indexing.cpp
                tests/test_search.cpp
            )
            
            target_link_libraries(test_stage3
                passdoq_stage3
                passdoq_stage1
                Catch2::Catch2WithMain
            )
            
            add_test(NAME Stage3Tests COMMAND test_stage3)
        endif()
    endif()
endif()

# Stage 4: Modules + Lua
if(BUILD_STAGE_4 AND BUILD_STAGE_1)
    set(STAGE4_SOURCES src/modules.cpp)
    set(STAGE4_LIBS passdoq_stage1)
    
    # Add Lua support if available
    if(LUA_FOUND AND SOL2_INCLUDE_DIR)
        list(APPEND STAGE4_SOURCES src/lua.cpp)
        list(APPEND STAGE4_LIBS ${LUA_LIBRARY})
    else()
        message(WARNING "Lua or Sol2 not found, Lua scripting will not be available")
    endif()
    
    # Add libltdl support if available
    if(LTDL_LIBRARY)
        list(APPEND STAGE4_LIBS ${LTDL_LIBRARY})
    else()
        message(WARNING "libltdl not found, module loading will not be available")
    endif()
    
    add_library(passdoq_stage4 STATIC ${STAGE4_SOURCES})
    target_link_libraries(passdoq_stage4 ${STAGE4_LIBS})
    
    # Link stage4 to main executable if stage2 is built
    if(BUILD_STAGE_2)
        target_link_libraries(passdoq passdoq_stage4)
    endif()
    
    # Example plugin
    if(LTDL_LIBRARY)
        add_library(ephemeral_datakey MODULE
            examples/plugins/ephemeral_datakey.c
        )
        
        target_include_directories(ephemeral_datakey PRIVATE
            ${CMAKE_SOURCE_DIR}/include
            ${SODIUM_INCLUDE_DIR}
        )
        
        target_link_libraries(ephemeral_datakey
            ${SODIUM_LIBRARY}
        )
        
        set_target_properties(ephemeral_datakey PROPERTIES
            PREFIX ""
            SUFFIX ".so"
        )
    endif()
    
    # Tests for Stage 4
    if(BUILD_TESTS)
        find_package(Catch2 3 QUIET)
        
        if(Catch2_FOUND)
            add_executable(test_stage4
                tests/test_modules.cpp
            )
            
            target_link_libraries(test_stage4
                passdoq_stage4
                passdoq_stage1
                Catch2::Catch2WithMain
            )
            
            # Add Lua tests if available
            if(LUA_FOUND AND SOL2_INCLUDE_DIR)
                target_sources(test_stage4 PRIVATE tests/test_lua.cpp)
            endif()
            
            add_test(NAME Stage4Tests COMMAND test_stage4)
        endif()
    endif()
endif()

# Stage 5: Services + RPC
if(BUILD_STAGE_5 AND BUILD_STAGE_1)
    set(STAGE5_SOURCES
        src/services.cpp
        src/daemon.cpp
        src/delivery.cpp
    )
    set(STAGE5_LIBS passdoq_stage1 Threads::Threads)
    
    # Add RPC support if available
    if(RPCLIB_LIBRARY)
        list(APPEND STAGE5_SOURCES src/remote.cpp)
        list(APPEND STAGE5_LIBS ${RPCLIB_LIBRARY})
    else()
        message(WARNING "rpclib not found, RPC functionality will not be available")
    endif()
    
    add_library(passdoq_stage5 STATIC ${STAGE5_SOURCES})
    target_link_libraries(passdoq_stage5 ${STAGE5_LIBS})
    
    # Link stage5 to main executable if stage2 is built
    if(BUILD_STAGE_2)
        target_link_libraries(passdoq passdoq_stage5)
    endif()
    
    # Daemon executable
    add_executable(passdoqd
        src/main.cpp
    )
    
    target_compile_definitions(passdoqd PRIVATE PASSDOQ_DAEMON)
    
    target_link_libraries(passdoqd
        passdoq_stage5
        passdoq_stage1
    )
    
    if(BUILD_STAGE_2)
        target_link_libraries(passdoqd passdoq_stage2)
    endif()
    
    if(BUILD_STAGE_3 AND XAPIAN_FOUND)
        target_link_libraries(passdoqd passdoq_stage3)
    endif()
    
    if(BUILD_STAGE_4)
        target_link_libraries(passdoqd passdoq_stage4)
    endif()
    
    # Tests for Stage 5
    if(BUILD_TESTS)
        find_package(Catch2 3 QUIET)
        
        if(Catch2_FOUND)
            add_executable(test_stage5
                tests/test_services.cpp
                tests/test_delivery.cpp
                tests/test_daemon.cpp
            )
            
            target_link_libraries(test_stage5
                passdoq_stage5
                passdoq_stage1
                Catch2::Catch2WithMain
            )
            
            add_test(NAME Stage5Tests COMMAND test_stage5)
        endif()
    endif()
endif()

# Installation
if(BUILD_STAGE_1)
    install(TARGETS passdoq_stage1
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
    )
endif()

if(BUILD_STAGE_2)
    install(TARGETS passdoq_stage2 passdoq
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
    )
endif()

if(BUILD_STAGE_3 AND XAPIAN_FOUND)
    install(TARGETS passdoq_stage3
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
    )
endif()

if(BUILD_STAGE_4)
    install(TARGETS passdoq_stage4
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
    )
    
    if(TARGET ephemeral_datakey)
        install(TARGETS ephemeral_datakey
            LIBRARY DESTINATION lib/passdoq/modules
        )
    endif()
endif()

if(BUILD_STAGE_5)
    install(TARGETS passdoq_stage5 passdoqd
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
    )
endif()

install(DIRECTORY include/
    DESTINATION include/passdoq
    FILES_MATCHING PATTERN "*.h"
)

# Install example scripts
install(DIRECTORY examples/scripts/
    DESTINATION share/passdoq/scripts
    FILES_MATCHING PATTERN "*.lua"
)

# Print configuration summary
message(STATUS "")
message(STATUS "Passdoq Configuration Summary:")
message(STATUS "  Build Stage 1 (Core): ${BUILD_STAGE_1}")
message(STATUS "  Build Stage 2 (CLI): ${BUILD_STAGE_2}")
message(STATUS "  Build Stage 3 (Search): ${BUILD_STAGE_3}")
message(STATUS "  Build Stage 4 (Modules): ${BUILD_STAGE_4}")
message(STATUS "  Build Stage 5 (Services): ${BUILD_STAGE_5}")
message(STATUS "  Build Tests: ${BUILD_TESTS}")
message(STATUS "")
message(STATUS "Third-party Dependencies:")
message(STATUS "  libsodium: ${SODIUM_LIBRARY}")
message(STATUS "  Xapian: ${XAPIAN_FOUND}")
message(STATUS "  Lua: ${LUA_FOUND}")
message(STATUS "  libltdl: ${LTDL_LIBRARY}")
message(STATUS "  rpclib: ${RPCLIB_LIBRARY}")
message(STATUS "")
