#include "tests_shared.h"
#include "plushie/cstl/core/fixed_width.h"

static TestFunctions* sTestFuncs[] =
{
	&gTestFuncsContainersSpan,
	&gTestFuncsContainersVector,
	&gTestFuncsCoreAlloc,
	&gTestFuncsCoreErrorCodes,
	&gTestFuncsFileTypeElf,
	&gTestFuncsFileTypeJson,
};
static PlushieUSize sTestFuncsCount = sizeof(sTestFuncs) / sizeof(sTestFuncs[0]);

void setUp(void) 
{
	PlushieUSize i;
	for (i = 0; i < sTestFuncsCount; i++)
	{
		if (sTestFuncs[i] == PLUSHIE_NULL)              continue;
		if (sTestFuncs[i]->setupPtr == PLUSHIE_NULL)    continue;
		sTestFuncs[i]->setupPtr();
	}
}

void tearDown(void) 
{
	PlushieUSize i;
	for (i = 0; i < sTestFuncsCount; i++)
	{
		if (sTestFuncs[i] == PLUSHIE_NULL)              continue;
		if (sTestFuncs[i]->teardownPtr == PLUSHIE_NULL) continue;
		sTestFuncs[i]->teardownPtr();
	}
}


int main(void)
{
	PlushieUSize i;
	UNITY_BEGIN();
	for (i = 0; i < sTestFuncsCount; i++)
	{
		if (sTestFuncs[i] == PLUSHIE_NULL)              continue;
		if (sTestFuncs[i]->mainPtr == PLUSHIE_NULL)     continue;
		sTestFuncs[i]->mainPtr();
	}
	return UNITY_END();
}

