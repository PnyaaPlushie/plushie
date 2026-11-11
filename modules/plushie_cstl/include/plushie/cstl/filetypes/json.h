/******************************************************************************
* @file include/plushie/cstl/filetypes/json.h
* @brief Small recursive Json parser
* @license MIT
* @authors L Green
*
* @note This json parser was primarily designed around gltf parsing in mind.
* Since gltf has a very small stack depth, it is acceptable for this
* implementation to perform a recursive descent. A parsing context has a
* configurable depth limit, defaulted on creation.
*******************************************************************************/
#ifndef PLUSHIE_CSTL_FILETYPES_JSON_H
#define PLUSHIE_CSTL_FILETYPES_JSON_H

#include "plushie/cstl/core/fixed_width.h"
#include "plushie/cstl/core/error_codes.h"
#include "plushie/cstl/core/alloc.h"
#include "plushie/cstl/core/mem.h"
#include "plushie/cstl/containers/span.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
* Settings defines
*******************************************************************************/

/**
* @brief Macro that the user needs to set to 1 before including this file in
* one compilation unit to provide the implementation
*/
#ifndef PLUSHIE_CSTL_FILETYPES_JSON_PROVIDE_IMPL
#  define PLUSHIE_CSTL_FILETYPES_JSON_PROVIDE_IMPL 0
#endif

/**
* @brief The default maximum depth for a json parser to go. This is used to
* prevent stack overflows from the recursive nature of json parsing. Remains
* configurable per parser object for dealing with a particularly large file.
*/
#ifndef PLUSHIE_CSTL_JSON_DEFAULT_MAX_DEPTH
#  define PLUSHIE_CSTL_JSON_DEFAULT_MAX_DEPTH 256
#endif

/**
* @brief Plushie CSTL is designed to be flexible, and potentially usable
* without the libc runtime. It is assumed that most users have libc, so the
* default behaviour uses libc functions. Currently it is low priority for
* plushie to provide those definitions, so we leave it to the user (for now).
*/
#ifndef PLUSHIE_CSTL_JSON_USE_LIBC_RUNTIME
#  define PLUSHIE_CSTL_JSON_USE_LIBC_RUNTIME 1
#endif

/******************************************************************************
* Span type used for slices into the source buffer
*******************************************************************************/

PLUSHIE_SPAN_DEFINE(PlushieU8StrSpan, PlushieU8StrSpan, PlushieUInt08)

/******************************************************************************
* Json nodes which consist of a key/value pair.
*******************************************************************************/

typedef enum PlushieJson_ValueType
{
	kPlushieJson_ValueType_Null,
	kPlushieJson_ValueType_Bool,
	kPlushieJson_ValueType_Number,
	kPlushieJson_ValueType_String,
	kPlushieJson_ValueType_Array,
	kPlushieJson_ValueType_Object
}
PlushieJson_Type;

typedef struct PlushieJson_ValueTypeArray
{
	struct PlushieJson_Value** items;
	PlushieUSize count;
	PlushieUSize capacity;
}
PlushieJson_ValueTypeArray;

typedef struct PlushieJson_ValueTypeObject
{
	struct PlushieJson_Node* members;
	PlushieUSize count;
	PlushieUSize capacity;
}
PlushieJson_ValueTypeObject;

typedef struct PlushieJson_Value
{
	PlushieJson_Type type;
	union
	{
		int                         boolean;
		double                      number;
		PlushieU8StrSpan            string;
		PlushieJson_ValueTypeArray  array;
		PlushieJson_ValueTypeObject object;
	}
	as;
}
PlushieJson_Value;

typedef struct PlushieJson_Node
{
	PlushieU8StrSpan  key;
	PlushieJson_Value value;
}
PlushieJson_Node;

/******************************************************************************
* Json parsing object
*******************************************************************************/

typedef struct PlushieJson_ParserCtx
{
	PlushieAllocator* allocator;
	PlushieU8StrSpan  source_span;
	unsigned int      max_depth;
}
PlushieJson_ParserCtx;

/**
* @brief Gives the user a new json parser, which will deserialize the given
* object in the source string view.
* @note The source string must stay valid while parsing is taking place
*/
PLUSHIE_CSTL_API PlushieResult plushie_json_parser_ctx_new(
	PlushieJson_ParserCtx* out_ctx,
	PlushieAllocator*      alloc,
	PlushieU8StrSpan       source_span
);

/**
* @brief Parses the entire json tree and fills the top node for the caller on
* success
*/
PLUSHIE_CSTL_API PlushieResult plushie_json_parse(
	PlushieJson_Value*     out_val,
	PlushieJson_ParserCtx* ctx
);

/******************************************************************************
* LibC Runtime macros
*******************************************************************************/
#if PLUSHIE_CSTL_FILETYPES_JSON_PROVIDE_IMPL

// Define the macros used for memcmp and strtod
#if PLUSHIE_CSTL_JSON_USE_LIBC_RUNTIME
#  include <string.h> // memcmp
#  include <stdlib.h> // strtod

#  define PLUSHIE_CSTL_JSON_LIBC_MEMCMP(BUFF1, BUFF2, SIZE) \
     memcmp(BUFF1, BUFF2, SIZE)

#  define PLUSHIE_CSTL_JSON_LIBC_STRTOD(STR, END_PTR) \
     strtod(STR, END_PTR)

#endif

// Check we have all the runtime libc replacement macros
#if (!defined PLUSHIE_CSTL_JSON_LIBC_MEMCMP) || \
    (!defined PLUSHIE_CSTL_JSON_LIBC_STRTOD)
#  error User is required to supply their own macros if they have no libc
#endif

#endif

/******************************************************************************
* Non-Inline Implementation
*******************************************************************************/
#if PLUSHIE_CSTL_FILETYPES_JSON_PROVIDE_IMPL
#ifndef PLUSHIE_CSTL_FILETYPES_JSON_PROVIDE_IMPL_GUARD
#define PLUSHIE_CSTL_FILETYPES_JSON_PROVIDE_IMPL_GUARD

/* Small helpers that operate by slicing ctx->source_span */
/* Consume n bytes from the ctx source span */
static inline void s_consume_bytes(PlushieJson_ParserCtx* ctx, PlushieUSize n)
{
	if (n >= ctx->source_span.len)
	{
		ctx->source_span.ptr += ctx->source_span.len;
		ctx->source_span.len = 0;
	}
	else
	{
		ctx->source_span.ptr += n;
		ctx->source_span.len -= n;
	}
}

/* Skip leading whitespace in the ctx source span */
static inline void s_skip_whitespace(PlushieJson_ParserCtx* ctx)
{
	PlushieUSize skip = 0;
	while (skip < ctx->source_span.len)
	{
		PlushieUInt08 c = ctx->source_span.ptr[skip];
		if (c == ' ' || c == '\t' || c == '\n' || c == '\r') { skip++; continue; }
		break;
	}
	s_consume_bytes(ctx, skip);
}

/* Forward declaration for recursive calling */
static PlushieResult s_parse_value(PlushieJson_Value* out_val, PlushieJson_ParserCtx* ctx, unsigned int depth);

/* Parse a quoted string. On success returns Success and fills out_span and
 * advances the ctx->source_span. Expects current first byte is opening quote. */
static PlushieResult s_parse_string(PlushieU8StrSpan* out_span, PlushieJson_ParserCtx* ctx)
{
	/* ensure opening quote */
	if (ctx->source_span.len == 0) return kPlushieResultErrParse;
	if (ctx->source_span.ptr[0] != '"') return kPlushieResultErrParse;
	/* mark start inside original buffer */
	const PlushieUInt08* start_ptr = ctx->source_span.ptr + 1;
	PlushieUSize i = 1;

	while (i < ctx->source_span.len)
	{
		PlushieUInt08 c = ctx->source_span.ptr[i];
		if (c == '"')
		{
			/* Found end: set span relative to original buffer */
			out_span->ptr = start_ptr;
			out_span->len = i - 1;
			/* consume up to and including closing quote */
			s_consume_bytes(ctx, i + 1);
			return kPlushieResultSuccess;
		}
		if (c == '\\')
		{
			/* Skip escape and next byte if present (we don't decode in-place) */
			if (i + 1 >= ctx->source_span.len) return kPlushieResultErrParse;
			i += 2;
			continue;
		}
		i++;
	}

	/* Unterminated string */
	return kPlushieResultErrParse;
}

/* Parse an object: expects '{' at current pos. Produces an allocated members array in out_val->as.object.members */
static PlushieResult s_parse_object(PlushieJson_Value* out_val, PlushieJson_ParserCtx* ctx, unsigned int depth)
{
	if (depth == 0) return kPlushieResultErrRange;
	/* ensure '{' */
	if (ctx->source_span.len == 0) return kPlushieResultErrParse;
	if (ctx->source_span.ptr[0] != '{') return kPlushieResultErrParse;
	/* consume '{' */
	s_consume_bytes(ctx, 1);
	s_skip_whitespace(ctx);

	/* Prepare dynamic array */
	PlushieJson_Node* members = NULL;
	PlushieUSize count = 0;
	PlushieUSize capacity = 0;

	/* Empty object check */
	if (ctx->source_span.len == 0) goto parse_error;
	if (ctx->source_span.ptr[0] == '}')
	{
		s_consume_bytes(ctx, 1); /* consume '}' */
		out_val->type = kPlushieJson_ValueType_Object;
		out_val->as.object.members = NULL;
		out_val->as.object.count = 0;
		out_val->as.object.capacity = 0;
		return kPlushieResultSuccess;
	}

	while (1)
	{
		s_skip_whitespace(ctx);
		/* Expect key string */
		if (ctx->source_span.len == 0) goto parse_error;
		if (ctx->source_span.ptr[0] != '"') goto parse_error;
		PlushieU8StrSpan key_span = { 0 };
		PlushieResult r = s_parse_string(&key_span, ctx);
		if (r != kPlushieResultSuccess) goto parse_error;

		s_skip_whitespace(ctx);
		/* Expect ':' */
		if (ctx->source_span.len == 0) goto parse_error;
		if (ctx->source_span.ptr[0] != ':') goto parse_error;
		s_consume_bytes(ctx, 1); /* skip ':' */
		s_skip_whitespace(ctx);

		/* Parse value */
		PlushieJson_Value value = { 0 };
		r = s_parse_value(&value, ctx, depth - 1);
		if (r != kPlushieResultSuccess)
		{
			/* free allocated members before returning error */
			if (members) plushie_free(ctx->allocator, members);
			return r;
		}

		/* push member */
		if (count == capacity)
		{
			PlushieUSize new_capacity = capacity ? (capacity * 2) : 4;
			PlushieUSize old_size = capacity * sizeof(PlushieJson_Node);
			PlushieUSize new_size = new_capacity * sizeof(PlushieJson_Node);
			void* new_mem = NULL;
			if (members == NULL)
			{
				new_mem = plushie_alloc(ctx->allocator, new_size);
			}
			else
			{
				new_mem = plushie_realloc(ctx->allocator, members, old_size, new_size);
			}
			if (new_mem == NULL)
			{
				if (members) plushie_free(ctx->allocator, members);
				return kPlushieResultErrAllocOom;
			}
			members = (PlushieJson_Node*)new_mem;
			capacity = new_capacity;
		}

		/* store node: copy key span pointer/len and value struct */
		members[count].key = key_span;
		members[count].value = value;
		count++;

		s_skip_whitespace(ctx);
		/* Next: either ',' or '}' */
		if (ctx->source_span.len == 0) goto parse_error;
		PlushieUInt08 next = ctx->source_span.ptr[0];
		if (next == ',')
		{
			s_consume_bytes(ctx, 1);
			s_skip_whitespace(ctx);
			continue;
		}
		else if (next == '}')
		{
			s_consume_bytes(ctx, 1); /* consume '}' */
			break;
		}
		else
		{
			goto parse_error;
		}
	}

	/* Fill out object value */
	out_val->type = kPlushieJson_ValueType_Object;
	out_val->as.object.members = members;
	out_val->as.object.count = count;
	out_val->as.object.capacity = capacity;
	return kPlushieResultSuccess;

parse_error:
	if (members) plushie_free(ctx->allocator, members);
	return kPlushieResultErrParse;
}

static PlushieResult s_parse_value(PlushieJson_Value* out_val, PlushieJson_ParserCtx* ctx, unsigned int depth)
{
	if (depth == 0) return kPlushieResultErrRange;
	s_skip_whitespace(ctx);
	if (ctx->source_span.len == 0) return kPlushieResultErrParse;
	PlushieUInt08 c = ctx->source_span.ptr[0];

	if (c == '"')
	{
		PlushieU8StrSpan s = { 0 };
		PlushieResult r = s_parse_string(&s, ctx);
		if (r != kPlushieResultSuccess) return r;
		out_val->type = kPlushieJson_ValueType_String;
		out_val->as.string = s;
		return kPlushieResultSuccess;
	}
	else if (c == '{')
	{
		return s_parse_object(out_val, ctx, depth - 1);
	}
	else if ((c >= '0' && c <= '9') || c == '-')
	{
		/* Minimal number parse: parse bytes until delimiter and use strtod */
		PlushieUSize i = 0;
		while (i < ctx->source_span.len)
		{
			PlushieUInt08 b = ctx->source_span.ptr[i];
			if ((b >= '0' && b <= '9') || b == '-' || b == '+' || b == 'e' || b == 'E' || b == '.')
			{
				i++; continue;
			}
			break;
		}
		if (i == 0) return kPlushieResultErrParse;
		/* create temporary nul-terminated buffer */
		char* tmp = (char*)plushie_alloc(ctx->allocator, i + 1);
		if (tmp == NULL) return kPlushieResultErrAllocOom;
		plushie_mem_copy(tmp, ctx->source_span.ptr, i);
		tmp[i] = '\0';
		char* endptr = NULL;
		double val = PLUSHIE_CSTL_JSON_LIBC_STRTOD(tmp, &endptr);
		int consumed_nothing = (endptr == tmp);
		plushie_free(ctx->allocator, tmp);
		/* basic validation: ensure some chars consumed */
		if (consumed_nothing) return kPlushieResultErrParse;
		out_val->type = kPlushieJson_ValueType_Number;
		out_val->as.number = val;
		/* consume number bytes */
		s_consume_bytes(ctx, i);
		return kPlushieResultSuccess;
	}
	else if (c == 't' || c == 'f')
	{
		/* true / false */
		if (ctx->source_span.len >= 4 && PLUSHIE_CSTL_JSON_LIBC_MEMCMP(ctx->source_span.ptr, "true", 4) == 0)
		{
			out_val->type = kPlushieJson_ValueType_Bool;
			out_val->as.boolean = 1;
			s_consume_bytes(ctx, 4);
			return kPlushieResultSuccess;
		}
		else if (ctx->source_span.len >= 5 && PLUSHIE_CSTL_JSON_LIBC_MEMCMP(ctx->source_span.ptr, "false", 5) == 0)
		{
			out_val->type = kPlushieJson_ValueType_Bool;
			out_val->as.boolean = 0;
			s_consume_bytes(ctx, 5);
			return kPlushieResultSuccess;
		}
		return kPlushieResultErrParse;
	}
	else if (c == 'n')
	{
		/* null */
		if (ctx->source_span.len >= 4 && PLUSHIE_CSTL_JSON_LIBC_MEMCMP(ctx->source_span.ptr, "null", 4) == 0)
		{
			out_val->type = kPlushieJson_ValueType_Null;
			s_consume_bytes(ctx, 4);
			return kPlushieResultSuccess;
		}
		return kPlushieResultErrParse;
	}

	return kPlushieResultErrParse;
}


PLUSHIE_CSTL_API PlushieResult plushie_json_parser_ctx_new(
	PlushieJson_ParserCtx* out_ctx,
	PlushieAllocator*      alloc,
	PlushieU8StrSpan       source_span
)
{
	if (out_ctx == NULL) return kPlushieResultErrNullptr;
	out_ctx->allocator = alloc;
	out_ctx->source_span = source_span;
	out_ctx->max_depth = PLUSHIE_CSTL_JSON_DEFAULT_MAX_DEPTH;
	return kPlushieResultSuccess;
}

PLUSHIE_CSTL_API PlushieResult plushie_json_parse(
	PlushieJson_Value*     out_val,
	PlushieJson_ParserCtx* ctx
)
{
	if (out_val == NULL || ctx == NULL) return kPlushieResultErrNullptr;
	plushie_zero_set(out_val, sizeof(*out_val));
	/* operate directly on ctx->source_span (slicing as we consume) */
	s_skip_whitespace(ctx);

	/* parse top-level value with max depth */
	PlushieResult r = s_parse_value(out_val, ctx, ctx->max_depth);
	if (r != kPlushieResultSuccess) return r;

	/* allow trailing whitespace, but no extra tokens */
	s_skip_whitespace(ctx);
	/* It's OK if input has a trailing null terminator inside the span */
	if (ctx->source_span.len > 0)
	{
		PlushieUSize rem = ctx->source_span.len;
		if (rem == 1 && ctx->source_span.ptr[0] == '\0') return kPlushieResultSuccess;
		/* otherwise consider it an error (extra token) */
		return kPlushieResultErrParse;
	}

	return kPlushieResultSuccess;
}

#endif // !PLUSHIE_CSTL_FILETYPES_JSON_PROVIDE_IMPL_GUARD
#endif // PLUSHIE_CSTL_FILETYPES_JSON_PROVIDE_IMPL

/******************************************************************************
* End of File
*******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif // !PLUSHIE_CSTL_FILETYPES_JSON_H
