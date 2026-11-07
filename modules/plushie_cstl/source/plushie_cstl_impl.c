// Source file which is only used if you're building a static or dynamic lib
// version of plushie_cstl for reuse.
//
// It simply includes each module header with its PROVIDE_IMPL define set so the
// out-of-line implementations are emitted exactly once, into this translation
// unit.

#include "plushie/cstl/core/platform.h"

#if PLUSHIE_TARGET_OS_ID == PLUSHIE_TARGET_OS_ID_WINDOWS
#include <Windows.h>
#endif

// Request the implementations from each module that provides out-of-line
// definitions.
#define PLUSHIE_CSTL_CORE_ERROR_CODES_PROVIDE_IMPL   1
#define PLUSHIE_CSTL_CORE_DIAGNOSTICS_PROVIDE_IMPL   1

#define PLUSHIE_CSTL_ALLOCATORS_LIBC_PROVIDE_IMPL    1
#define PLUSHIE_CSTL_ALLOCATORS_SYSTEM_PROVIDE_IMPL  1
#define PLUSHIE_CSTL_ALLOCATORS_ALLOCA_PROVIDE_IMPL  1

#define PLUSHIE_CSTL_CONTAINERS_STRING_PROVIDE_IMPL  1

#define PLUSHIE_CSTL_FILETYPES_ELF_PROVIDE_IMPL      1
#define PLUSHIE_CSTL_FILETYPES_JSON_PROVIDE_IMPL     1

#define PLUSHIE_CSTL_HOST_CONIO_PROVIDE_IMPL         1
#define PLUSHIE_CSTL_HOST_FILEHANDLE_PROVIDE_IMPL    1
#define PLUSHIE_CSTL_HOST_THREADING_PROVIDE_IMPL     1
#define PLUSHIE_CSTL_HOST_CMDARG_PROVIDE_IMPL        1

// Core
#include "plushie/cstl/core/error_codes.h"

// Allocators
#include "plushie/cstl/allocators/libc.h"
#include "plushie/cstl/allocators/system.h"
#include "plushie/cstl/allocators/alloca.h"

// Containers (string emits the PlushieVecStr vector implementation too)
#include "plushie/cstl/containers/string.h"

// Diagnostics (depends on string + vector)
#include "plushie/cstl/core/diagnostics.h"

// Filetypes
#include "plushie/cstl/filetypes/elf.h"
#include "plushie/cstl/filetypes/json.h"

// Host
#include "plushie/cstl/host/conio.h"
#include "plushie/cstl/host/filehandle.h"
#include "plushie/cstl/host/threading.h"
#include "plushie/cstl/host/cmdarg.h"
