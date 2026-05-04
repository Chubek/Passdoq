# ---------------------------
# Examples
# ---------------------------

# Only build if we're part of the main project
if(NOT TARGET passdoq_core)
    message(FATAL_ERROR "examples/CMakeLists.txt must be included from parent project")
endif()

# Plugins
add_library(ephemeral_datakey MODULE
    plugins/ephemeral_datakey.c
)

# Ensure no "lib" prefix (typical for plugins)
set_target_properties(ephemeral_datakey PROPERTIES
    PREFIX ""
    OUTPUT_NAME "ephemeral_datakey"
    SUFFIX ".so"
)

# Include Passdoq headers
target_include_directories(ephemeral_datakey
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)

# Plugins should NOT link against passdoq_core directly
# They load symbols at runtime via dlopen
# Only link if absolutely necessary for compilation
if(APPLE)
    # macOS requires explicit linking for undefined symbols
    target_link_libraries(ephemeral_datakey PRIVATE passdoq_core)
endif()

# Platform-specific tweaks
if(UNIX)
    target_compile_options(ephemeral_datakey PRIVATE -fPIC)
endif()

if(WIN32)
    set_target_properties(ephemeral_datakey PROPERTIES SUFFIX ".dll")
endif()

# ---------------------------
# Lua scripts (resources)
# ---------------------------

file(GLOB LUA_SCRIPTS scripts/*.lua)

if(NOT LUA_SCRIPTS)
    message(WARNING "No Lua scripts found in examples/scripts/")
endif()

# Copy scripts to build dir
add_custom_target(copy_example_scripts ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/scripts
        ${CMAKE_BINARY_DIR}/examples/scripts
    COMMENT "Copying example Lua scripts to build directory"
)

# ---------------------------
# Install
# ---------------------------

install(TARGETS ephemeral_datakey
    LIBRARY DESTINATION lib/passdoq/plugins
    RUNTIME DESTINATION lib/passdoq/plugins  # For Windows DLLs
)

if(LUA_SCRIPTS)
    install(FILES ${LUA_SCRIPTS}
        DESTINATION share/passdoq/examples/scripts
    )
endif()

# Install plugin header for reference
install(FILES ${CMAKE_SOURCE_DIR}/include/passdoq-module.h
    DESTINATION include/passdoq
)
