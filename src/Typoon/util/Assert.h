#pragma once
#include "util/Debugging.h"


#ifdef __cpp_contracts
    #if __has_cpp_attribute(assume)
        #define ASSERT(expr) contract_assert(expr); [[assume(expr)]]
    #else
        #define ASSERT(expr) contract_assert(expr)
    #endif
#elif __has_cpp_attribute(assume)
    #define ASSERT(expr) if (!(expr)) { BREAKPOINT_IF_DEBUGGING(); } [[assume(expr)]]
#else
    #define ASSERT(expr) if (!(expr)) { BREAKPOINT_IF_DEBUGGING(); }
#endif
