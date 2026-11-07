/******************************************************************************
* @file include/plushie/cstl/core/serde.h
* @brief Provides type per type into buffer serialization and deserialization
* @license MIT
* @authors L Green
*
* @note We're using an X-Macro theme for serialization. Structs which get
* serialized have their fields declared with a schema. A schema takes multiple
* macro params, for fixed sized arrays vs regular fields
*
```c
#define DECL_SCHEMA ( \
  PLUSHIE_XFIELD,     \
  PLUSHIE_XARRAY,     \
  PLUSHIE_XBLOB,      \
  )                   \
    PLUSHIE_XFIELD(FieldName, Type, SerdeFunctNameSuffix)          \
	PLUSHIE_XARRAY(FieldName, Type, Count, SerdeFunctNameSuffix)   \
	PLUSHIE_XBLOB (FieldName, ByteCount)                            \
```
* Serialization/deserialization functions have the name
* plushie_serde_to_${SerdeFunctNameSuffix}
*
* With the arrays, since they'll be fixed sized structs, the compiler should
* unroll any of the loops for us. Blobs get turned directly into a memcopy.
*******************************************************************************/
#ifndef PLUSHIE_CSTL_CORE_SERDE_H
#define PLUSHIE_CSTL_CORE_SERDE_H

#include "plushie/cstl/core/fixed_width.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
* Struct declaration (X-macro helpers a user's schema can expand to)
*******************************************************************************/

#define PLUSHIE_SERDE_DECL_FIELD(name, type, serde_suffix) \
  type name;

#define PLUSHIE_SERDE_DECL_ARRAY(name, type, count, serde_suffix) \
  type name[count];

#define PLUSHIE_SERDE_DECL_BLOB(name, count) \
  PlushieUInt08 name[count];

// TODO: field/array/blob serialize X-macro helpers (schema-driven serialize).

/******************************************************************************
* U32
*******************************************************************************/

/**
* @brief Serializes a UInt32 from the current page's endianness to little
* endian in a buffer
*/
static inline void plushie_serialize_to_ui32_le(
	PlushieUInt32  value,
	PlushieUInt08* out_buffer
)
{
	out_buffer[0] = (PlushieUInt08)((value & 0x000000ff));
	out_buffer[1] = (PlushieUInt08)((value & 0x0000ff00) >> 8);
	out_buffer[2] = (PlushieUInt08)((value & 0x00ff0000) >> 16);
	out_buffer[3] = (PlushieUInt08)((value & 0xff000000) >> 24);
}

/**
* @brief Serializes a UInt32 from the current page's endianness to big endian
* in a buffer
*/
static inline void plushie_serialize_to_ui32_be(
	PlushieUInt32  value,
	PlushieUInt08* out_buffer
)
{
	out_buffer[0] = (PlushieUInt08)((value & 0xff000000) >> 24);
	out_buffer[1] = (PlushieUInt08)((value & 0x00ff0000) >> 16);
	out_buffer[2] = (PlushieUInt08)((value & 0x0000ff00) >> 8);
	out_buffer[3] = (PlushieUInt08)((value & 0x000000ff));
}

/******************************************************************************
* End of file content
*******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif // !PLUSHIE_CSTL_CORE_SERDE_H
