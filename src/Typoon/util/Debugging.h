#pragma once


#ifdef __cpp_lib_debugging
    #include <debugging>
    #define BREAKPOINT_IF_DEBUGGING() std::breakpoint_if_debugging()
#else
    #ifdef _MSC_VER
        #include <intrin.h>
        #define BREAKPOINT_IF_DEBUGGING() __debugbreak()
    #else
        #include <csignal>
        #define BREAKPOINT_IF_DEBUGGING() std::raise(SIGTRAP)
    #endif
#endif
