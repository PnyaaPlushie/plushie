#ifndef PLUSHIE_TEST_EXIT_CODE
#define PLUSHIE_TEST_EXIT_CODE 0
#endif

void __cdecl _RTC_InitBase(void) {}
void __cdecl _RTC_Shutdown(void) {}

int main(void) {return PLUSHIE_TEST_EXIT_CODE; }