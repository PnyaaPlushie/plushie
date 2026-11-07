/******************************************************************************
* @file include/plushie/cstl/core/diagnostics.h
* @brief Diagnostics and logging
* @license MIT
* @authors L Green
*
* @note Works by allowing the user to provide sinks for different
* implementation requirements.
*
* A diagnostics host and its sinks gate sending their diagnostics based on a
* matching flag for the diagnostic string, instead of having levels. There are
* some default flags for reference, but a user can use any they like.
*******************************************************************************/
#ifndef PLUSHIE_CSTL_CORE_DIAGNOSTICS_H
#define PLUSHIE_CSTL_CORE_DIAGNOSTICS_H

#include "plushie/cstl/core/error_codes.h"
#include "plushie/cstl/core/fixed_width.h"
#include "plushie/cstl/containers/string.h"
#include "plushie/cstl/containers/vector.h"

#ifndef PLUSHIE_CSTL_CORE_DIAGNOSTICS_PROVIDE_IMPL
#  define PLUSHIE_CSTL_CORE_DIAGNOSTICS_PROVIDE_IMPL 0
#endif

// Whether the ring buffer sink is available
#ifndef PLUSHIE_CSTL_DIAGNOSTICS_RING_BUFF
#  define PLUSHIE_CSTL_DIAGNOSTICS_RING_BUFF 1
#endif

// A very small optional static sink for pre-console diagnostics
#ifndef PLUSHIE_CSTL_DIAGNOSTICS_PROVIDE_SINK_STATIC_BUFF
#  define PLUSHIE_CSTL_DIAGNOSTICS_PROVIDE_SINK_STATIC_BUFF 0
#endif
#ifndef PLUSHIE_CSTL_DIAGNOSTICS_STATIC_BUFF_ENTRY_COUNT
#  define PLUSHIE_CSTL_DIAGNOSTICS_STATIC_BUFF_ENTRY_COUNT 4
#endif
#ifndef PLUSHIE_CSTL_DIAGNOSTICS_STATIC_BUFF_ENTRY_LENGTH
#  define PLUSHIE_CSTL_DIAGNOSTICS_STATIC_BUFF_ENTRY_LENGTH 256
#endif

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
* Public struct declarations
*******************************************************************************/

/**
* @brief Host for receiving diagnostic entries, used for flushing and
* allocating new strings and entries.
*/
typedef struct PlushieDiagnosticsHost PlushieDiagnosticsHost;

/**
* @brief An object for a user to provide/receive a message
*/
typedef struct PlushieDiagnosticSink PlushieDiagnosticSink;

/**
* @brief Function pointer struct for one user diagnostic sink
*/
typedef struct PlushieDiagnosticSinkVTable PlushieDiagnosticSinkVTable;

/**
* @brief Diagnostic sink which exists to be used without dynamic allocation or
* io. This is useful for pre-init errors (like before the console appears).
*/
typedef struct PlushieDiagnosticSinkRingBuffer PlushieDiagnosticSinkRingBuffer;

/******************************************************************************
* Struct and API declarations
*******************************************************************************/

PlushieResult plushie_diagnostic_send(
	PlushieDiagnosticsHost*   host,
	PlushieUInt32             signal_val,
	PlushieStr                msg
);

PlushieResult plushie_diagnostic_send_cstr(
	PlushieDiagnosticsHost*   host,
	PlushieUInt32             signal_val,
	const char* const         msg
);


struct PlushieDiagnosticSinkVTable
{
	void (*write)(PlushieStr str, PlushieUInt32 mask, void* udata);
};

struct PlushieDiagnosticSink
{
	PlushieDiagnosticSinkVTable vtable;
	void*                       udata;
};

PlushieVectorDecl(PlushieVecDiagnosticSink, PlushieDiagnosticSink, diagnostic_sink);

struct PlushieDiagnosticsHost
{
	PlushieVecDiagnosticSink sinks;
	PlushieUInt32            signal_mask;
};

//
// Ring buffer
//

struct PlushieDiagnosticSinkRingBuffer
{
	PlushieStr*  string_buffer;
	PlushieUSize buffer_length;
	PlushieUSize head_index;
	PlushieUSize tail_index;
};


void plushie_diagnostic_sink_ring_buffer_write(
	PlushieStr    str,
	PlushieUInt32 mask,
	void*         udata
);


/******************************************************************************
* Implementation
*******************************************************************************/
#if PLUSHIE_CSTL_CORE_DIAGNOSTICS_PROVIDE_IMPL
#ifndef PLUSHIE_CSTL_CORE_DIAGNOSTICS_PROVIDE_IMPL_GUARD
#define PLUSHIE_CSTL_CORE_DIAGNOSTICS_PROVIDE_IMPL_GUARD

PlushieResult plushie_diagnostic_send(
	PlushieDiagnosticsHost* host,
	PlushieUInt32           signal_val,
	PlushieStr              msg
)
{
	(void)signal_val;
	(void)msg;
	if (host == NULL) return kPlushieResultErrNullptr;

	// TODO: fan the message out to each registered sink whose mask matches.
	return kPlushieResultSuccess;
}

PlushieResult plushie_diagnostic_send_cstr(
	PlushieDiagnosticsHost* host,
	PlushieUInt32           signal_val,
	const char* const       msg
)
{
	(void)signal_val;
	(void)msg;
	if (host == NULL) return kPlushieResultErrNullptr;

	// TODO: wrap msg in a PlushieStr view and forward to plushie_diagnostic_send.
	return kPlushieResultSuccess;
}

//
// Ring buffer
//

void plushie_diagnostic_sink_ring_buffer_write(
	PlushieStr    str,
	PlushieUInt32 mask,
	void*         udata
)
{
	(void)str;
	(void)mask;
	PlushieDiagnosticSinkRingBuffer* ring = (PlushieDiagnosticSinkRingBuffer*)udata;
	if (ring == NULL) return;

	// TODO: push the entry into the ring, advancing head/tail.
}

// Provide the out-of-line definition for the sink vector type
PlushieVectorDefine(PlushieVecDiagnosticSink, PlushieDiagnosticSink, diagnostic_sink);

#endif // !PLUSHIE_CSTL_CORE_DIAGNOSTICS_PROVIDE_IMPL_GUARD
#endif // !PLUSHIE_CSTL_CORE_DIAGNOSTICS_PROVIDE_IMPL

/******************************************************************************
* End of file
*******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif // !PLUSHIE_CSTL_CORE_DIAGNOSTICS_H
