/******************************************************************************
* @file include/plushie/cstl/containers/vector.h
* @brief Type-safe resizable array generated per element type
* @license MIT
* @authors L Green
*
* I'm a fan of type safety, so instead of a generic void* vector, this provides
* macros for generating a per-type implementation, which a user can override if
* they wish (see PLUSHIE_CSTL_VECTOR_USE_CUSTOM_DECL / _DEFN).
*
* Usage:
*   // header scope
*   PlushieVectorDecl(MyIntVec, int, my_int);
*   // exactly one translation unit
*   PlushieVectorDefine(MyIntVec, int, my_int);
*
* Generates the struct `MyIntVec` and the functions:
*   plushie_vec_my_int_init(MyIntVec*, PlushieAllocator*)
*   plushie_vec_my_int_reserve(MyIntVec*, PlushieUSize)
*   plushie_vec_my_int_push_back(MyIntVec*, int)
*   plushie_vec_my_int_free(MyIntVec*)
*******************************************************************************/
#ifndef PLUSHIE_CSTL_CONTAINERS_VECTOR_H
#define PLUSHIE_CSTL_CONTAINERS_VECTOR_H

#include "plushie/cstl/core/platform.h"
#include "plushie/cstl/core/fixed_width.h"
#include "plushie/cstl/core/error_codes.h"
#include "plushie/cstl/core/alloc.h"
#include "plushie/cstl/core/mem.h"

#ifndef PLUSHIE_CSTL_VECTOR_USE_CUSTOM_DECL
#  define PLUSHIE_CSTL_VECTOR_USE_CUSTOM_DECL 0
#endif

#ifndef PLUSHIE_CSTL_VECTOR_USE_CUSTOM_DEFN
#  define PLUSHIE_CSTL_VECTOR_USE_CUSTOM_DEFN 0
#endif

// Growth: capacity used when pushing into an empty vector.
#ifndef PLUSHIE_CSTL_VECTOR_INITIAL_CAP
#  define PLUSHIE_CSTL_VECTOR_INITIAL_CAP 8
#endif

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
* Declaration macro
*******************************************************************************/

#if PLUSHIE_CSTL_VECTOR_USE_CUSTOM_DECL == 0
#ifndef PlushieVectorDecl

/**
* @brief Declares the struct and function prototypes for a new vector type
*/
#define PlushieVectorDecl(OutTypeName, ElementType, FuncPostfix)               \
    typedef struct OutTypeName                                                 \
    {                                                                          \
        PlushieAllocator* alloc;                                               \
        ElementType*      ptr;                                                 \
        PlushieUSize      size;                                                \
        PlushieUSize      cap;                                                 \
    } OutTypeName;                                                             \
                                                                               \
    PlushieResult plushie_vec_##FuncPostfix##_init(                            \
        OutTypeName*      out_vec,                                             \
        PlushieAllocator* alloc                                                \
    );                                                                         \
                                                                               \
    PlushieResult plushie_vec_##FuncPostfix##_reserve(                         \
        OutTypeName*      vec,                                                 \
        PlushieUSize      new_cap                                              \
    );                                                                         \
                                                                               \
    PlushieResult plushie_vec_##FuncPostfix##_push_back(                       \
        OutTypeName*      vec,                                                 \
        ElementType       value                                               \
    );                                                                         \
                                                                               \
    void plushie_vec_##FuncPostfix##_free(                                     \
        OutTypeName*      vec                                                  \
    );

#endif // !PlushieVectorDecl
#endif // !PLUSHIE_CSTL_VECTOR_USE_CUSTOM_DECL

/******************************************************************************
* Definition macro
*******************************************************************************/

#if PLUSHIE_CSTL_VECTOR_USE_CUSTOM_DEFN == 0
#ifndef PlushieVectorDefine

/**
* @brief Defines the implementation for the user's vector type
*/
#define PlushieVectorDefine(OutTypeName, ElementType, FuncPostfix)             \
    PlushieResult plushie_vec_##FuncPostfix##_init(                            \
        OutTypeName*      out_vec,                                             \
        PlushieAllocator* alloc                                                \
    )                                                                          \
    {                                                                          \
        if (out_vec == NULL || alloc == NULL)                                  \
            return kPlushieResultErrNullptr;                                   \
        out_vec->alloc = alloc;                                                \
        out_vec->ptr   = NULL;                                                 \
        out_vec->size  = 0;                                                    \
        out_vec->cap   = 0;                                                    \
        return kPlushieResultSuccess;                                          \
    }                                                                          \
                                                                               \
    PlushieResult plushie_vec_##FuncPostfix##_reserve(                         \
        OutTypeName*      vec,                                                 \
        PlushieUSize      new_cap                                              \
    )                                                                          \
    {                                                                          \
        if (vec == NULL) return kPlushieResultErrNullptr;                      \
        if (new_cap <= vec->cap) return kPlushieResultSuccess;                 \
        if (vec->alloc == NULL || vec->alloc->alloc == NULL)                   \
            return kPlushieResultErrAllocPfnMissing;                           \
        if (new_cap > ((PlushieUSize)-1) / sizeof(ElementType))               \
            return kPlushieResultErrRange;                                     \
                                                                               \
        PlushieUSize bytes = new_cap * sizeof(ElementType);                    \
        ElementType* new_ptr = (ElementType*)vec->alloc->alloc(               \
            vec->alloc, bytes, PLUSHIE_ALIGNOF(ElementType),                   \
            kPlushieAllocatorAllocFlagsNone, NULL);                            \
        if (new_ptr == NULL) return kPlushieResultErrAllocOom;                 \
                                                                               \
        if (vec->ptr != NULL)                                                  \
        {                                                                      \
            plushie_mem_copy(new_ptr, vec->ptr,                                \
                vec->size * sizeof(ElementType));                             \
            if (vec->alloc->free != NULL)                                      \
                vec->alloc->free(vec->alloc, vec->ptr);                        \
        }                                                                      \
                                                                               \
        vec->ptr = new_ptr;                                                    \
        vec->cap = new_cap;                                                    \
        return kPlushieResultSuccess;                                          \
    }                                                                          \
                                                                               \
    PlushieResult plushie_vec_##FuncPostfix##_push_back(                       \
        OutTypeName*      vec,                                                 \
        ElementType       value                                               \
    )                                                                          \
    {                                                                          \
        if (vec == NULL) return kPlushieResultErrNullptr;                      \
        if (vec->size >= vec->cap)                                             \
        {                                                                      \
            PlushieUSize grow = (vec->cap == 0)                                \
                ? (PlushieUSize)PLUSHIE_CSTL_VECTOR_INITIAL_CAP                \
                : vec->cap * 2;                                                \
            PlushieResult r =                                                  \
                plushie_vec_##FuncPostfix##_reserve(vec, grow);               \
            if (r != kPlushieResultSuccess) return r;                          \
        }                                                                      \
        vec->ptr[vec->size] = value;                                          \
        vec->size += 1;                                                        \
        return kPlushieResultSuccess;                                          \
    }                                                                          \
                                                                               \
    void plushie_vec_##FuncPostfix##_free(                                     \
        OutTypeName*      vec                                                  \
    )                                                                          \
    {                                                                          \
        if (vec == NULL) return;                                               \
        if (vec->ptr != NULL && vec->alloc != NULL                            \
            && vec->alloc->free != NULL)                                       \
            vec->alloc->free(vec->alloc, vec->ptr);                            \
        vec->ptr  = NULL;                                                      \
        vec->size = 0;                                                         \
        vec->cap  = 0;                                                         \
    }

#endif // !PlushieVectorDefine
#endif // !PLUSHIE_CSTL_VECTOR_USE_CUSTOM_DEFN

/******************************************************************************
* End of file content
*******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif // !PLUSHIE_CSTL_CONTAINERS_VECTOR_H
