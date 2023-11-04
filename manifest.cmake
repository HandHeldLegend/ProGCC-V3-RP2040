# Set the path to your header file
set(VERSION_HEADER "${CMAKE_SOURCE_DIR}/include/board_config.h")

# Set the path to the HOJA backend version
set(HOJA_HEADER "${CMAKE_SOURCE_DIR}/lib/HOJA-LIB-RP2040/include/hoja.h")

# Set the path to your manifest.json
set(MANIFEST_JSON "${CMAKE_SOURCE_DIR}/manifest.json")

# Read the FW version from the header file
file(READ ${VERSION_HEADER} VERSION_CONTENT)

# Read the HOJA version from the header file
file(READ ${HOJA_HEADER} HOJA_CONTENT)

# Extract the version number from the file content
string(REGEX MATCH "HOJA_FW_VERSION[ ]*0x([0-9A-Fa-f]+)" VERSION_HEX "${VERSION_CONTENT}")

if (NOT VERSION_HEX)
    message(FATAL_ERROR "Version not found in ${VERSION_HEADER}")
else()
    string(TOUPPER "${CMAKE_MATCH_1}" VERSION_HEX_UPPER)
    math(EXPR VERSION_INT "0x${VERSION_HEX_UPPER}")

    # Read the existing JSON data from manifest.json
    file(READ ${MANIFEST_JSON} MANIFEST_JSON_CONTENT)

    # Find and replace the "version" key in JSON content
    string(REGEX REPLACE "\"fw_version\"[ ]*:[ ]*[0-9]+" "\"fw_version\": ${VERSION_INT}" UPDATED_JSON "${MANIFEST_JSON_CONTENT}")

    # Write the updated JSON content back to manifest.json
    file(WRITE ${MANIFEST_JSON} ${UPDATED_JSON})

    # Print the extracted version
    message(STATUS "Extracted version: ${VERSION_INT}")
endif()

# Extract the HOJA number from the file content
string(REGEX MATCH "HOJA_BACKEND_VERSION[ ]*0x([0-9A-Fa-f]+)" HOJA_HEX "${HOJA_CONTENT}")

if (NOT HOJA_HEX)
    message(FATAL_ERROR "HOJA Backend not found in ${HOJA_HEADER}")
else()
    string(TOUPPER "${CMAKE_MATCH_1}" HOJA_HEX_UPPER)
    math(EXPR HOJA_INT "0x${HOJA_HEX_UPPER}")

    # Read the existing JSON data from manifest.json
    file(READ ${MANIFEST_JSON} MANIFEST_JSON_CONTENT)

    # Find and replace the "version" key in JSON content
    string(REGEX REPLACE "\"hoja_version\"[ ]*:[ ]*[0-9]+" "\"hoja_version\": ${HOJA_INT}" UPDATED_JSON "${MANIFEST_JSON_CONTENT}")

    # Write the updated JSON content back to manifest.json
    file(WRITE ${MANIFEST_JSON} ${UPDATED_JSON})

    # Print the extracted version
    message(STATUS "Extracted HOJA Backend: ${HOJA_INT}")
endif()