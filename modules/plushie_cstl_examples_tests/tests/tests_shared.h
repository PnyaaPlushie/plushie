#ifndef PLUSHIE_TESTS_SHARED_HEADER_GUARD_H
#define PLUSHIE_TESTS_SHARED_HEADER_GUARD_H
#include "unity.h"

// The test files are always compiled as C, but main may be compiled as C++
#ifdef __cplusplus
extern "C" {
#endif

typedef struct TestFunctions
{
	void (*setupPtr)(void);
	void (*teardownPtr)(void);
	void (*mainPtr)(void);
}
TestFunctions;

extern TestFunctions gTestFuncsContainersSpan;
extern TestFunctions gTestFuncsContainersVector;
extern TestFunctions gTestFuncsCoreAlloc;
extern TestFunctions gTestFuncsCoreErrorCodes;
extern TestFunctions gTestFuncsFileTypeElf;
extern TestFunctions gTestFuncsFileTypeJson;

#ifdef __cplusplus
} // extern "C"
#endif

#endif //!PLUSHIE_TESTS_SHARED_HEADER_GUARD_H