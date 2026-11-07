/******************************************************************************
* @file include/plushie/cstl/core/atomic.h
* @brief Lockfree atomic fixed width integers
* @license MIT
* @authors L Green
*
* @note Real atomicity is provided by the C11 <stdatomic.h> backend or the
* Win32 Interlocked backend. The `volatile` on the Win32 typedefs only marks
* the storage; the ordering guarantees come from the Interlocked intrinsics,
* not from `volatile` itself.
*******************************************************************************/
#ifndef PLUSHIE_CSTL_CORE_ATOMIC_H
#define PLUSHIE_CSTL_CORE_ATOMIC_H

#include "plushie/cstl/core/platform.h"
#include "plushie/cstl/core/fixed_width.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
* Win32 backend
* Preferred on Windows because MSVC's C11 <stdatomic.h> support is recent and
* not always present in the default C mode.
*******************************************************************************/
#if PLUSHIE_TARGET_OS_ID == PLUSHIE_TARGET_OS_ID_WINDOWS
#include <Windows.h>

#define PlushieAtomicUInt16 volatile PlushieUInt16
#define PlushieAtomicUInt32 volatile PlushieUInt32
#define PlushieAtomicUInt64 volatile PlushieUInt64

/**
* @brief Sets a u16 to a new value via an atomic operation
*/
static inline PlushieUInt16 plushie_atomic_u16_exchange(
	PlushieAtomicUInt16* in_out,
	PlushieUInt16        set_value
)
{
	return (PlushieUInt16)InterlockedExchange16(
		(volatile SHORT*)in_out, (SHORT)set_value);
}

/**
* @brief Sets a u16 to a new value via an atomic operation
*/
static inline PlushieUInt32 plushie_atomic_u32_exchange(
	PlushieAtomicUInt32* in_out,
	PlushieUInt32        set_value
)
{
	return (PlushieUInt32)InterlockedExchange(
		(volatile LONG*)in_out, (LONG)set_value);
}


static inline PlushieUInt16 plushie_atomic_u16_compare_exchange(
	PlushieAtomicUInt16* in_out,
	PlushieUInt16        set_value,
	PlushieUInt16        compare_value
)
{
	return (PlushieUInt16)InterlockedCompareExchange16(
		(volatile SHORT*)in_out, (SHORT)set_value, (SHORT)compare_value);
}

/******************************************************************************
* C11 backend
* Available when the toolchain is C11+ and does not advertise
* __STDC_NO_ATOMICS__ (which is only defined when atomics are unavailable).
*******************************************************************************/
#elif PLUSHIE_C_STD >= 2011 && !defined(__STDC_NO_ATOMICS__)
#include <stdatomic.h>

#define PlushieAtomicUInt16 _Atomic PlushieUInt16
#define PlushieAtomicUInt32 _Atomic PlushieUInt32
#define PlushieAtomicUInt64 _Atomic PlushieUInt64

static inline PlushieUInt16 plushie_atomic_u16_exchange(
	PlushieAtomicUInt16* in_out,
	PlushieUInt16        set_value
)
{
	return atomic_exchange(in_out, set_value);
}

static inline PlushieUInt16 plushie_atomic_u16_compare_exchange(
	PlushieAtomicUInt16* in_out,
	PlushieUInt16        set_value,
	PlushieUInt16        compare_value
)
{
	// Mirror the Win32 Interlocked semantics: return the value that was
	// observed in *in_out before the operation.
	PlushieUInt16 expected = compare_value;
	atomic_compare_exchange_strong(in_out, &expected, set_value);
	return expected;
}

#else
#  error "plushie atomics require a C11 <stdatomic.h> or Win32 backend"
#endif

/******************************************************************************
* Processor yield hint (spin-wait relaxation)
*******************************************************************************/
static inline void plushie_yield_processor(
	void
)
{
#if PLUSHIE_TARGET_OS_ID == PLUSHIE_TARGET_OS_ID_WINDOWS
	YieldProcessor();
#elif (PLUSHIE_COMPILER_ID == PLUSHIE_COMPILER_ID_GCC) \
   || (PLUSHIE_COMPILER_ID == PLUSHIE_COMPILER_ID_CLANG)
#  if defined(__i386__) || defined(__x86_64__)
	__builtin_ia32_pause();
#  else
	__asm__ __volatile__("" ::: "memory");
#  endif
#else
	// No portable hint available; nothing to do.
	((void)0);
#endif
}

/******************************************************************************
* End of file content
*******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif // !PLUSHIE_CSTL_CORE_ATOMIC_H
