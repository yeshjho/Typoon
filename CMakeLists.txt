cmake_minimum_required(VERSION 4.0)
project(Typoon
    VERSION 1.0.0
    DESCRIPTION "A text expander that properly supports Korean typing system."
    LANGUAGES CXX
)

# -------------------------
# Configuration options
# -------------------------
if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Build type" FORCE)
endif()

option(TYPOON_WARNINGS_AS_ERRORS "Treat warnings as errors" ON)
option(TYPOON_ENABLE_SANITIZERS "Enable Address/Undefined/Leak sanitizers (only in Debug)" ON)
option(TYPOON_ENABLE_LTO "Enable Link Time Optimization" ON)
option(TYPOON_ENABLE_PCH "Enable precompiled headers" OFF)

option(TYPOON_BUILD_UNITTEST "Build unit tests" ON)


# -------------------------
# Helper: compiler family detection
# -------------------------
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(TYPOON_COMPILER_CLANG TRUE)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(TYPOON_COMPILER_GNU TRUE)
elseif (MSVC)
    set(TYPOON_COMPILER_MSVC TRUE)
endif()


# -------------------------
# C++ standard version
# -------------------------
set(TYPOON_CXX_STANDARD 26 CACHE STRING "C++ standard to use (e.g. 17, 20, 23, 26)")
set_property(CACHE TYPOON_CXX_STANDARD PROPERTY STRINGS 17 20 23 26)

if (TYPOON_CXX_STANDARD STREQUAL "26")
    if (TYPOON_COMPILER_MSVC)
        add_compile_options(/std:c++latest)
    elseif (TYPOON_COMPILER_GNU OR TYPOON_COMPILER_CLANG)
        add_compile_options(-std=c++2c)
    endif()
else()
    set(CMAKE_CXX_STANDARD ${TYPOON_CXX_STANDARD})
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


# -------------------------
# Misc
# -------------------------
# Build position independent code by default for libraries
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Unicode charset
if (MSVC)
    add_compile_definitions(UNICODE _UNICODE)
endif()

# Uni-algo
add_compile_definitions(UNI_ALGO_STATIC_DATA)


# -------------------------
# Common flags by compiler (per-target preferred)
# -------------------------
function (typoon_target_common_setup target IS_EXTERNAL)
    if (TYPOON_COMPILER_GNU OR TYPOON_COMPILER_CLANG)
        # Warnings
        if (IS_EXTERNAL)
            # Ignore all warnings for external libraries
            target_compile_options(${target} PRIVATE
                -Wno-everything
            )

        else()
            target_compile_options(${target} PRIVATE
                -Wall -Wextra -Wpedantic
                -Wcast-align -Wconversion -Wsign-conversion
                -Wnon-virtual-dtor -Wdouble-promotion
            )

            # Recommended hardened flags
            target_compile_options(${target} PRIVATE
                -fstack-protector-strong  # Stack protection
                -fno-omit-frame-pointer   # Keep frame pointer for better stack traces
                -D_FORTIFY_SOURCE=2       # Compile-time and runtime checks for unsafe C library functions
                -fvisibility=hidden       # Hide symbols by default
            )

            # Disabled warnings in Debug mode
            target_compile_options(${target} PRIVATE $<$<CONFIG:Debug>:
                -Wno-unused-variable       # Unused local variable
                -Wno-unused-parameter      # Unreferenced formal parameter
            >)

            if (TYPOON_WARNINGS_AS_ERRORS)
                target_compile_options(${target} PRIVATE -Werror)
            endif()
        endif()

        if (TYPOON_ENABLE_LTO)
            target_compile_options(${target} PRIVATE $<$<NOT:$<CONFIG:Debug>>:-flto>)
            target_link_options(${target} PRIVATE $<$<NOT:$<CONFIG:Debug>>:-flto>)
        endif()

    elseif (TYPOON_COMPILER_MSVC)
        # Warnings
        if (IS_EXTERNAL)
            # Ignore all warnings for external libraries
            target_compile_options(${target} PRIVATE
                /W0
            )

        else()
            target_compile_options(${target} PRIVATE
                /W4 /permissive-
            )

            # Disabled warnings in Debug mode
            target_compile_options(${target} PRIVATE $<$<CONFIG:Debug>:
                /wd4189                 # Unused local variable
                /wd4100                 # Unreferenced formal parameter
            >)

            if (TYPOON_WARNINGS_AS_ERRORS)
                target_compile_options(${target} PRIVATE /WX)
            endif()
        endif()

        if (TYPOON_ENABLE_LTO)
            target_compile_options(${target} PRIVATE $<$<NOT:$<CONFIG:Debug>>:/GL>)
            target_link_options(${target} PRIVATE $<$<NOT:$<CONFIG:Debug>>:/LTCG>)
        endif()

        target_compile_options(${target} PRIVATE 
            /GS                     # Buffer security checks
            /Zc:preprocessor        # Standard-compliant preprocessor
            /sdl                    # Enable additional security checks
            /MP                     # Multi-processor compilation
            /external:anglebrackets # Treat <> includes as external
            /external:W0            # No warnings from external includes
            /source-charset:utf-8   # Source files are UTF-8
            /Oy-                    # Disable frame pointer omission
            $<$<CONFIG:Debug>:
                /JMC                # Just My Code debugging
            >
            $<$<NOT:$<CONFIG:Debug>>:
                /Zi                 # Program database info in non-Debug
                /Ob2                # Full optimization of inline functions
                /Oi                 # Generate intrinsic functions
                /Ot                 # Favor fast code
            >
        )

        target_link_options(${target} PRIVATE
            /NXCOMPAT               # Mark as compatible with Data Execution Prevention (DEP)
            /WX                     # Treat warnings as errors
            /INCREMENTAL:NO         # Disable incremental linking
            $<$<CONFIG:Debug>:
            >
            $<$<NOT:$<CONFIG:Debug>>:
                /LTCG               # Link-time code generation
                /DYNAMICBASE        # Enable Address Space Layout Randomization
                /OPT:REF            # Eliminate unreferenced functions/data
            >
        )

    endif()

    # Debug preprocessor definition
    target_compile_definitions(${target} PRIVATE
        $<$<CONFIG:Debug>:_DEBUG>
        $<$<NOT:$<CONFIG:Debug>>:NDEBUG>
    )

    # Position independent code for libraries
    if (TARGET ${target})
        set_target_properties(${target} PROPERTIES POSITION_INDEPENDENT_CODE ${CMAKE_POSITION_INDEPENDENT_CODE})
    endif()

    # Sanitizers (only in Debug)
    if (TYPOON_ENABLE_SANITIZERS)
        if (TYPOON_COMPILER_GNU OR TYPOON_COMPILER_CLANG)
            target_compile_options(${target} PRIVATE $<$<CONFIG:Debug>:-fsanitize=address,undefined,leak>)
            target_link_options(${target} PRIVATE $<$<CONFIG:Debug>:-fsanitize=address,undefined,leak>)
        elseif (TYPOON_COMPILER_MSVC)
            target_compile_options(${target} PRIVATE $<$<CONFIG:Debug>:/fsanitize=address>)
        endif()
    endif()

    # Precompiled headers
    if (TYPOON_ENABLE_PCH)
        set(local_pch "${CMAKE_CURRENT_SOURCE_DIR}/PCH.h")
        if (EXISTS "${local_pch}")
            target_precompile_headers(${target} PRIVATE "${local_pch}")
        endif()
    endif()

    # External includes
    target_include_directories(${target} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/external/include
    )

    # Mimic folder structure
    get_target_property(SRCS ${target} SOURCES)
    get_target_property(NO_GROUP_SOURCES ${target} NO_GROUP_SOURCES)

    if (SRCS)
        if (NO_GROUP_SOURCES)
            list(REMOVE_ITEM SRCS ${NO_GROUP_SOURCES})
        endif()

        if (SRCS)
            source_group(
                TREE "${CMAKE_CURRENT_SOURCE_DIR}"
                FILES ${SRCS}
            )
        endif()
    endif()
endfunction()


# -------------------------
# Utility: add a proper executable/library
# -------------------------
function (typoon_add_executable name)
    cmake_parse_arguments(ARG "" "" "SRCS;NO_GROUP_SOURCES" ${ARGN})

    # Source files
    file(GLOB_RECURSE AUTO_SRCS CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.inl"
    )
    set(SRCS ${AUTO_SRCS} ${ARG_SRCS})

    add_executable(${name} ${SRCS})

    if (ARG_NO_GROUP_SOURCES)
        set_target_properties(${name} PROPERTIES NO_GROUP_SOURCES "${ARG_NO_GROUP_SOURCES}")
    endif()

    typoon_target_common_setup(${name} FALSE)
endfunction()

function (typoon_add_library name)
    # TYPE: [STATIC]/SHARED/OBJECT
    cmake_parse_arguments(ARG "IS_EXTERNAL" "TYPE" "SRCS;NO_GROUP_SOURCES" ${ARGN})
    if (NOT ARG_TYPE)
        # STATIC by default
        set(ARG_TYPE STATIC)
    endif()

    # Source files
    file(GLOB_RECURSE AUTO_SRCS CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.inl"
    )
    set(SRCS ${AUTO_SRCS} ${ARG_SRCS})

    add_library(${name} ${ARG_TYPE} ${SRCS})
    
    if (ARG_NO_GROUP_SOURCES)
        set_target_properties(${name} PROPERTIES NO_GROUP_SOURCES "${ARG_NO_GROUP_SOURCES}")
    endif()

    # export symbol visibility helper for shared libs on Windows
    if (ARG_TYPE STREQUAL "SHARED")
        target_compile_definitions(${name} PRIVATE "TYPOON_EXPORTS")
    endif()

    typoon_target_common_setup(${name} ${ARG_IS_EXTERNAL})
endfunction()


add_subdirectory(src/Typoon)
if (TYPOON_BUILD_UNITTEST)
    add_subdirectory(src/UnitTests)
endif()
