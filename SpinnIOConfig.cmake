# ===================================================================================
#  The SpinnIO library CMake configuration file
#
#
#  Usage from an external project: 
#    In your CMakeLists.txt, add these lines:
#
#    FIND_PACKAGE(SpinnIO REQUIRED )
#    TARGET_LINK_LIBRARIES(MY_TARGET_NAME ${SPINNIO_LIB})   
#
#    This file will define the following variables:
#      - SPINNIO_LIB          : The list of libraries to link against.
#      - SPINNIO_LIB_DIR       : The directory where lib files are. Calling LINK_DIRECTORIES
#                                with this path is NOT needed.
#      - SPINNIO_VERSION       : The  version of this build. Example: "2.1.1"
#      - SPINNIO_VERSION_MAJOR : Major version part. Example: "2"
#      - SPINNIO_VERSION_MINOR : Minor version part. Example: "1"
#      - SPINNIO_VERSION_PATCH : Patch version part. Example: "0"
#
# ===================================================================================


# Extract the directory where *this* file has been installed (determined at cmake run-time)
#  This variable may or may not be used below, depending on the parsing of OpenCVConfig.cmake
get_filename_component(THIS_SPINNIO_CONFIG_PATH "${CMAKE_CURRENT_LIST_FILE}" PATH)

# ======================================================
# Include directories to add to the user project:
# ======================================================
INCLUDE_DIRECTORIES("${HOME}/SpinnIO/")
SET(SPINNIO_INCLUDE_DIR "/usr/local/include")

# ======================================================
# Link directories to add to the user project:
# ======================================================
LINK_DIRECTORIES("/usr/local/lib")
# Provide the libs directory anyway, it may be needed in some cases.
SET(SPINNIO_LIB_DIRS "/usr/local/lib")

# ====================================================================
# Link libraries
# ====================================================================
SET(SPINNIO_LIB SpinnIO)

# ======================================================
#  Version variables: 
# ======================================================
SET(SPINNIO_VERSION 0.1.0)
SET(SPINNIO_VERSION_MAJOR  0)
SET(SPINNIO_VERSION_MINOR  1)
SET(SPINNIO_VERSION_PATCH  0)
