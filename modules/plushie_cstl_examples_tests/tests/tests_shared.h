#ifndef PLUSHIE_TESTS_SHARED_HEADER_GUARD_H
#define PLUSHIE_TESTS_SHARED_HEADER_GUARD_H
#include "unity.h"

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

#endif //!PLUSHIE_TESTS_SHARED_HEADER_GUARD_H