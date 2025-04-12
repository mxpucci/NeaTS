cmake_minimum_required(VERSION 3.22)
project(NeaTS LANGUAGES C CXX)

# --- Standard Settings ---
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_CXX_EXTENSIONS OFF)  # Uncomment for stricter standard compliance

# Set a default build type if none was specified
if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Release' as none was specified.")
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the build type" FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
    "MinSizeRel" "RelWithDebInfo")
endif()

# --- Options ---
option(ENABLE_HUGE_PAGES "Enable Huge Pages support (requires OS-specific code implementation)" OFF)
option(ENABLE_FAST_MATH "Enable potentially faster, non-standard math optimizations (-fno-math-errno for GCC/Clang)" ON)
option(ENABLE_AVX512 "Attempt to enable AVX512F instructions if supported by compiler" ON)
option(ENABLE_SSE_FPMATH "Use SSE for floating point math on GCC/Clang if supported (-mfpmath=sse)" ON)

# --- Compiler Flags & Definitions ---
include(CheckCXXCompilerFlag)
include(CheckCCompilerFlag)

set(PORTABLE_C_FLAGS "")
set(PORTABLE_CXX_FLAGS "")
set(PORTABLE_DEFINITIONS "")
set(PORTABLE_LINK_LIBS "")

# Common warning flags
if (MSVC)
  list(APPEND PORTABLE_CXX_FLAGS "/W3")
  list(APPEND PORTABLE_C_FLAGS "/W3")
else()
  list(APPEND PORTABLE_CXX_FLAGS -Wall -Wextra)
  list(APPEND PORTABLE_C_FLAGS "-Wall")
endif()

# Optional Fast Math (-fno-math-errno)
if (ENABLE_FAST_MATH AND NOT MSVC)
  list(APPEND PORTABLE_CXX_FLAGS "-fno-math-errno")
endif()

# Optional SSE FP Math (GCC/Clang x86/x64)
if (ENABLE_SSE_FPMATH AND NOT MSVC)
  check_cxx_compiler_flag("-mfpmath=sse" COMPILER_SUPPORTS_MFPMATH_SSE)
  if (COMPILER_SUPPORTS_MFPMATH_SSE)
    list(APPEND PORTABLE_CXX_FLAGS "-mfpmath=sse")
    message(STATUS "Enabled -mfpmath=sse")
  else()
    message(STATUS "Compiler does not support -mfpmath=sse, skipping.")
  endif()
endif()

# Optional AVX512F
if (ENABLE_AVX512)
  if (MSVC)
    list(APPEND PORTABLE_CXX_FLAGS "/arch:AVX512")
    message(STATUS "Attempting to enable /arch:AVX512 for MSVC (requires compatible VS version)")
  else()
    check_cxx_compiler_flag("-mavx512f" COMPILER_SUPPORTS_AVX512F)
    if (COMPILER_SUPPORTS_AVX512F)
      list(APPEND PORTABLE_CXX_FLAGS "-mavx512f")
      message(STATUS "Enabled -mavx512f")
    else()
      message(STATUS "Compiler does not support -mavx512f, skipping.")
    endif()
  endif()
endif()

# Huge Pages Definition
if (ENABLE_HUGE_PAGES)
  list(APPEND PORTABLE_DEFINITIONS "USE_HUGE_PAGES")
  message(STATUS "Enabling Huge Pages support definition (USE_HUGE_PAGES).")
endif()

# Unix/Linux specific settings
if (UNIX)
  list(APPEND PORTABLE_LINK_LIBS "m") # Link math library
  if (CMAKE_SIZEOF_VOID_P EQUAL 4)
    list(APPEND PORTABLE_DEFINITIONS "_FILE_OFFSET_BITS=64")
    message(STATUS "Adding -D_FILE_OFFSET_BITS=64 for 32-bit Unix-like system")
  endif()
  if (NOT APPLE AND NOT MSVC)
    list(APPEND PORTABLE_LINK_LIBS "dl")
  endif()
  if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    list(APPEND PORTABLE_LINK_LIBS "rt")
  endif()
endif()

# --- Library Definitions ---

# Create an INTERFACE library target for NeaTS.
# This target will export the headers from the /include directory.
add_library(NeaTS INTERFACE)
target_include_directories(NeaTS INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

# --- Include other dependency interface libraries if needed ---
# Here we prepare the two dependencies (sux and sdsl) as INTERFACE libraries.
add_library(sux INTERFACE)
target_include_directories(sux INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/sux
)

add_library(sdsl INTERFACE)
target_include_directories(sdsl INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/sdsl-lite/include
)

# --- Executable Definition ---

# Build the main executable (for testing/demo)
add_executable(main main.cpp)
target_include_directories(main PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include  # Project header directory
)
target_compile_options(main PRIVATE ${PORTABLE_CXX_FLAGS})
target_compile_definitions(main PRIVATE ${PORTABLE_DEFINITIONS})
# Link the main executable with:
# - The NeaTS INTERFACE target (which provides your include paths)
# - The dependency interface targets (sux, sdsl)
# - And any OS-specific libraries collected in PORTABLE_LINK_LIBS.
target_link_libraries(main PRIVATE
  NeaTS
  sux
  sdsl
  ${PORTABLE_LINK_LIBS}
)

# --- Installation Setup ---
include(CMakePackageConfigHelpers)

# Install exported headers (the public API)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
  DESTINATION include
)

# Install the NeaTS interface target for consumers.
install(TARGETS NeaTS
  EXPORT NeaTSTargets
  INCLUDES DESTINATION include
)

# Export the target so that it can be found as NeaTS::NeaTS.
install(EXPORT NeaTSTargets
  FILE NeaTSTargets.cmake
  NAMESPACE NeaTS::
  DESTINATION cmake
)

# Configure the package configuration file
configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/NeaTSConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/NeaTSConfig.cmake
  INSTALL_DESTINATION cmake
  NO_SET_AND_CHECK_MACRO
  NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

# Install the generated config file
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/NeaTSConfig.cmake
  DESTINATION cmake
)