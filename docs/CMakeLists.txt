# ---------------------------
# Documentation
# ---------------------------

find_package(Doxygen)

if(NOT DOXYGEN_FOUND)
    message(WARNING "Doxygen not found - documentation will not be built")
    return()
endif()

# Configure Doxyfile
set(DOXYGEN_INPUT_DIR "${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/src")
set(DOXYGEN_OUTPUT_DIR "${CMAKE_BINARY_DIR}/docs")
set(DOXYGEN_PROJECT_NAME "${PROJECT_NAME}")
set(DOXYGEN_PROJECT_VERSION "${PROJECT_VERSION}")
set(DOXYGEN_PROJECT_BRIEF "${PROJECT_DESCRIPTION}")

# Process Doxyfile.in
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
    ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    @ONLY
)

# Create output directory
file(MAKE_DIRECTORY ${DOXYGEN_OUTPUT_DIR})

# Add custom target for documentation
add_custom_target(docs
    COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen"
    VERBATIM
)

# Optional: add manual pages
file(GLOB MANUAL_PAGES manual/*.md)

if(MANUAL_PAGES)
    add_custom_target(copy_manual_pages ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/manual
            ${DOXYGEN_OUTPUT_DIR}/manual
        COMMENT "Copying manual pages"
    )
endif()

# Install documentation
install(DIRECTORY ${DOXYGEN_OUTPUT_DIR}/
    DESTINATION share/doc/passdoq
)
