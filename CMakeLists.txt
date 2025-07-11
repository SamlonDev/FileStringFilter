cmake_minimum_required(VERSION 3.10)
project(SalmonFiltuh VERSION 1.0 LANGUAGES CXX)

# Set C++ standard and required options
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)  # For clang-tidy and other tools

# Set output directory for the executable
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../)

# Configure build types with appropriate flags
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# Set default build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type (Debug, Release, etc.)" FORCE)
endif()

# Add executable
add_executable(${PROJECT_NAME}
    salmonFiltuh.cpp
    file_processor.cpp
    search_utils.cpp
)

# Set include directories
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Set compiler warnings
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Debug>:-Wall -Wextra -Wpedantic>
    $<$<CONFIG:Release>:-Wall -O3>
)

# Platform-specific settings
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else()
    # Enable link-time optimization for release builds on non-Windows
    include(CheckIPOSupported)
    check_ipo_supported(RESULT ipo_supported)
    if(ipo_supported)
        set_property(TARGET ${PROJECT_NAME} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
    
    # Additional optimizations for release builds
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:-march=native -mtune=native -fomit-frame-pointer -funroll-loops>
    )
endif()