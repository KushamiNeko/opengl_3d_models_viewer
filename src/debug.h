#ifndef DEBUG_H
#define DEBUG_H
#include <assert.h>

// use assertion macro to perform development assertion check, turn __DEBUG to 0
// when production build
// #define __DEBUG 1

#define __DEBUG 0

#if __DEBUG
#define __ASSERT(expr) assert(expr)
#define __VERIFY(expr) assert(expr)
#else
#define __ASSERT(expr) NULL
#define __VERIFY(expr) (expr)
#endif

#endif
