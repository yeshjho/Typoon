#pragma once


#ifndef __cpp_contracts
    #include <cassert>
#endif

#ifdef __cpp_contracts
    #if __has_cpp_attribute(assume)
        #define ASSERT(expr) contract_assert(expr); [[assume(expr)]]
    #else
        #define ASSERT(expr) contract_assert(expr)
    #endif
#elif __has_cpp_attribute(assume)
    #define ASSERT(expr) assert((expr)); [[assume(expr)]]
#else
    #define ASSERT(expr) assert((expr))
#endif
