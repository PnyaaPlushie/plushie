/******************************************************************************
* @file include/plushie/cstl/core/error_codes.h
* @brief Defines all of the error codes in the plushie cstl
* @license MIT
* @authors L Green
*
* @note Uses an X-Macro and schema approach to make the enum and the lookup
* tables. As a result the numeric error code values are not stable across
* versions; compare against the named enumerators, never the raw integers.
*******************************************************************************/
#ifndef PLUSHIE_CSTL_CORE_ERROR_CODES_H
#define PLUSHIE_CSTL_CORE_ERROR_CODES_H

#ifdef __cplusplus
extern "C" {
#endif

// Macro to force the header to provide the implementation for the functions
#ifndef PLUSHIE_CSTL_CORE_ERROR_CODES_PROVIDE_IMPL
#  define PLUSHIE_CSTL_CORE_ERROR_CODES_PROVIDE_IMPL 0
#endif

// Macro to allow the lookup functions to be generated instead of stubs. Can be
// disabled to save rom space if you don't need to decode error codes.
#ifndef PLUSHIE_CSTL_CORE_ERROR_CODE_INCLUDE_LOOKUP
#  define PLUSHIE_CSTL_CORE_ERROR_CODE_INCLUDE_LOOKUP 1
#endif


/**
* @brief Schema which defines the enum and the name/description lookups.
* @note Success must be first (value 0) and Count must be last.
*/
#define PLUSHIE_CSTL_ERROR_CODE_SCHEMA                                                  \
    PLUSHIE_X (Success,               "Operation completed without error"              )\
    PLUSHIE_X (ErrUnclassified,       "Unclassified error"                             )\
    PLUSHIE_X (ErrNullptr,            "A required pointer argument was null"           )\
    PLUSHIE_X (ErrInvalidParam,       "An argument was outside its valid range"        )\
    PLUSHIE_X (ErrOs,                 "The underlying OS reported a failure"           )\
    PLUSHIE_X (ErrAllocPfnMissing,    "The allocator has no function for the op"       )\
    PLUSHIE_X (ErrAllocOom,           "The allocator could not satisfy the request"    )\
    PLUSHIE_X (ErrRange,              "An index or size exceeded the valid bounds"     )\
    PLUSHIE_X (ErrEof,                "Input ended before the operation completed"     )\
    PLUSHIE_X (ErrParse,              "Input was malformed and could not be parsed"    )\
    PLUSHIE_X (ErrUnsupported,        "The requested feature or format is unsupported" )\
                                                                                        \
    PLUSHIE_X (ErrVkExtNotPresent,    "The requested vulkan extension was not found"   )\
    PLUSHIE_X (ErrVkLayerNotPresent,  "The requested vulkan layer was not found"       )\
    PLUSHIE_X (ErrVkCallFailed,       "A required vulkan function call failed"         )\
                                                                                        \
    PLUSHIE_X (Count,                 "Number of result enums"                         )


/******************************************************************************
* Define enums
*******************************************************************************/

#define PLUSHIE_X(VAR_NAME, DESC) \
    kPlushieResult##VAR_NAME ,

/**
* @brief General result type for plushie and the cstl
*/
typedef enum PlushieResult
{
    PLUSHIE_CSTL_ERROR_CODE_SCHEMA
}
PlushieResult;

#undef PLUSHIE_X


/******************************************************************************
* Lookup declarations
*******************************************************************************/

/**
* @brief Gets the (stable) name of an error code, e.g. "ErrNullptr"
*/
const char* plushie_result_decode_name(
    PlushieResult result
);

/**
* @brief Gets the human readable description of an error code
*/
const char* plushie_result_decode_desc(
    PlushieResult result
);


/******************************************************************************
* Lookup implementation
*******************************************************************************/

#if PLUSHIE_CSTL_CORE_ERROR_CODES_PROVIDE_IMPL
#ifndef PLUSHIE_CSTL_CORE_ERROR_CODES_PROVIDE_IMPL_GUARD
#define PLUSHIE_CSTL_CORE_ERROR_CODES_PROVIDE_IMPL_GUARD

#if PLUSHIE_CSTL_CORE_ERROR_CODE_INCLUDE_LOOKUP

#define PLUSHIE_X(VAR_NAME, DESC) \
    case kPlushieResult##VAR_NAME: return #VAR_NAME;

const char* plushie_result_decode_name(
    PlushieResult result
)
{
    switch (result)
    {
        PLUSHIE_CSTL_ERROR_CODE_SCHEMA
    default:
        return "Error not found";
    }
}

#undef PLUSHIE_X

#define PLUSHIE_X(VAR_NAME, DESC) \
    case kPlushieResult##VAR_NAME: return DESC;

const char* plushie_result_decode_desc(
    PlushieResult result
)
{
    switch (result)
    {
        PLUSHIE_CSTL_ERROR_CODE_SCHEMA
    default:
        return "Error not found";
    }
}

#undef PLUSHIE_X

#else // !PLUSHIE_CSTL_CORE_ERROR_CODE_INCLUDE_LOOKUP

// Stub implementation if the user doesn't want any lookup functions

const char* plushie_result_decode_name(
    PlushieResult result
)
{
    (void)result;
    return "";
}

const char* plushie_result_decode_desc(
    PlushieResult result
)
{
    (void)result;
    return "";
}

#endif // !PLUSHIE_CSTL_CORE_ERROR_CODE_INCLUDE_LOOKUP

#endif // !PLUSHIE_CSTL_CORE_ERROR_CODES_PROVIDE_IMPL_GUARD
#endif // !PLUSHIE_CSTL_CORE_ERROR_CODES_PROVIDE_IMPL


/******************************************************************************
* End of file
*******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif // !PLUSHIE_CSTL_CORE_ERROR_CODES_H
