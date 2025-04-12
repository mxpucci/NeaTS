cmake_minimum_required(VERSION 3.22)
project(NeaTS LANGUAGES C CXX) # Explicitly state languages

# --- Standard Settings ---
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_CXX_EXTENSIONS OFF) # Uncomment for stricter standard compliance

# Set a default build type if none was specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Release' as none was specified.")
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the build type" FORCE)
  # Set the possible values of build type for cmake-gui, ccmake
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
include(CheckCCompilerFlag) # For C flags if needed

set(PORTABLE_C_FLAGS "")
set(PORTABLE_CXX_FLAGS "")
set(PORTABLE_DEFINITIONS "")
set(PORTABLE_LINK_LIBS "")

# Common Warning Flags
if(MSVC)
  list(APPEND PORTABLE_CXX_FLAGS "/W3")
  list(APPEND PORTABLE_C_FLAGS "/W3")
else()
  list(APPEND PORTABLE_CXX_FLAGS -Wall -Wextra)
  list(APPEND PORTABLE_C_FLAGS "-Wall")
endif()

# Optional Fast Math (-fno-math-errno)
if(ENABLE_FAST_MATH AND NOT MSVC)
  list(APPEND PORTABLE_CXX_FLAGS "-fno-math-errno")
endif()

# Optional SSE FP Math (GCC/Clang x86/x64)
if(ENABLE_SSE_FPMATH AND NOT MSVC)
  check_cxx_compiler_flag("-mfpmath=sse" COMPILER_SUPPORTS_MFPMATH_SSE)
  if(COMPILER_SUPPORTS_MFPMATH_SSE)
    list(APPEND PORTABLE_CXX_FLAGS "-mfpmath=sse")
    message(STATUS "Enabled -mfpmath=sse")
  else()
    message(STATUS "Compiler does not support -mfpmath=sse, skipping.")
  endif()
endif()

# Optional AVX512F (Check compiler support)
if(ENABLE_AVX512)
  if(MSVC)
    # MSVC uses /arch:AVX512 - Check flag might not work perfectly for /arch
    # Assume recent MSVC might support it; requires VS 2019 16.11+ or VS 2022+
    # A more robust check might involve try_compile
    list(APPEND PORTABLE_CXX_FLAGS "/arch:AVX512")
    message(STATUS "Attempting to enable /arch:AVX512 for MSVC (requires compatible VS version)")
  else()
    check_cxx_compiler_flag("-mavx512f" COMPILER_SUPPORTS_AVX512F)
    if(COMPILER_SUPPORTS_AVX512F)
      list(APPEND PORTABLE_CXX_FLAGS "-mavx512f")
      message(STATUS "Enabled -mavx512f")
    else()
      message(STATUS "Compiler does not support -mavx512f, skipping.")
    endif()
  endif()
endif()

# Huge Pages Definition
if(ENABLE_HUGE_PAGES)
  list(APPEND PORTABLE_DEFINITIONS "USE_HUGE_PAGES")
  message(STATUS "Enabling Huge Pages support definition (USE_HUGE_PAGES). Requires OS-specific code.")
endif()

# Unix/Linux specific libraries and definitions
if(UNIX)
  # Link math library (often automatic for C++, but safe to add)
  list(APPEND PORTABLE_LINK_LIBS "m")

  # Large File Support Definition (usually only needed on 32-bit Unix)
  # Check if we are on a 32-bit system before adding
  if(CMAKE_SIZEOF_VOID_P EQUAL 4)
      list(APPEND PORTABLE_DEFINITIONS "_FILE_OFFSET_BITS=64")
      message(STATUS "Adding -D_FILE_OFFSET_BITS=64 for 32-bit Unix-like system")
  endif()

  # Dynamic linking library (libdl) - not on Apple or Windows
  if(NOT APPLE AND NOT MSVC)
    list(APPEND PORTABLE_LINK_LIBS "dl")
  endif()

  # Real-time library (librt) - Linux specific usually
  if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    list(APPEND PORTABLE_LINK_LIBS "rt")
    # Optional: Find and link libhugetlbfs if requested and found on Linux
    # if(ENABLE_HUGE_PAGES)
    #   find_library(HUGETLBFS_LIBRARY hugetlbfs)
    #   if(HUGETLBFS_LIBRARY)
    #     message(STATUS "Found libhugetlbfs: ${HUGETLBFS_LIBRARY}")
    #     list(APPEND PORTABLE_LINK_LIBS ${HUGETLBFS_LIBRARY})
    #   else()
    #     message(WARNING "Huge Pages enabled, but libhugetlbfs not found.")
    #   endif()
    # endif()
  endif()
endif()


# --- Library Definitions ---

# Interface library for sux (header only assumed)
add_library(sux INTERFACE)
target_include_directories(sux INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/sux
)

# Interface library for sdsl-lite (header only assumed)
add_library(sdsl INTERFACE)
target_include_directories(sdsl INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/sdsl-lite/include
)


# --- Executable Definition ---
add_executable(main main.cpp)

# --- Target Specific Settings ---

# Include directories for main executable
target_include_directories(main PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include # Project's own include directory
)

# Apply portable compile options/flags
target_compile_options(main PRIVATE
    ${PORTABLE_CXX_FLAGS} # Apply collected C++ flags
)
# If you had C specific flags for main (e.g. from PORTABLE_C_FLAGS):
# target_compile_options(main PRIVATE $<$<COMPILE_LANGUAGE:C>:${PORTABLE_C_FLAGS}>)

# Apply portable definitions
target_compile_definitions(main PRIVATE
    ${PORTABLE_DEFINITIONS}
)

# Link libraries
target_link_libraries(main PRIVATE
    sux                     # Link against interface library sux
    sdsl                    # Link against interface library sdsl
    ${PORTABLE_LINK_LIBS}   # Link against OS-specific libs (m, dl, rt)
)

#-----------

# Create an INTERFACE library target for NeaTS
add_library(NeaTS INTERFACE)

# Export the include directory
target_include_directories(NeaTS INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
