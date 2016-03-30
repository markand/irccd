/*
 * js.h -- JavaScript C++14 wrapper for Duktape
 *
 * Copyright (c) 2016 David Demelier <markand@malikania.fr>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef JS_H
#define JS_H

/**
 * @file js.h
 * @brief Bring JavaScript using Duktape.
 *
 * This file provides usual Duktape function renamed and placed into `duk` namespace. It also replaces error
 * code with exceptions when possible.
 *
 * For convenience, this file also provides templated functions, overloads and much more.
 */

#include <cassert>
#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include <duktape.h>

namespace irccd {

/**
 * Duktape C++ namespace wrapper.
 */
namespace duk {

class Context;

using CodePoint = duk_codepoint_t;
using ContextPtr = duk_context *;
using Index = duk_idx_t;
using Ret = duk_ret_t;
using Size = duk_size_t;

/**
 * @class StackAssert
 * @brief Stack sanity checker.
 *
 * Instanciate this class where you need to manipulate the Duktape stack outside a Duktape/C function, its destructor
 * will examinate if the stack size matches the user expected size.
 *
 * When compiled with NDEBUG, this class does nothing.
 *
 * To use it, just declare an lvalue at the beginning of your function.
 */
class StackAssert {
#if !defined(NDEBUG)
private:
	ContextPtr m_context;
	unsigned m_expected;
	unsigned m_begin;
#endif

public:
	/**
	 * Create the stack checker.
	 *
	 * No-op if NDEBUG is set.
	 *
	 * @param ctx the context
	 * @param expected the size expected relative to the already existing values
	 */
	inline StackAssert(ContextPtr ctx, unsigned expected = 0) noexcept
#if !defined(NDEBUG)
		: m_context(ctx)
		, m_expected(expected)
		, m_begin(static_cast<unsigned>(duk_get_top(ctx)))
#endif
	{
#if defined(NDEBUG)
		(void)ctx;
		(void)expected;
#endif
	}

	/**
	 * Verify the expected size.
	 *
	 * No-op if NDEBUG is set.
	 */
	inline ~StackAssert() noexcept
	{
#if !defined(NDEBUG)
		assert((unsigned)duk_get_top(m_context) - m_begin == m_expected);
#endif
	}
};

/**
 * @class Object
 * @brief Empty class tag for push() function.
 */
class Object {
};

/**
 * @class Array
 * @brief Empty class tag for push() function.
 */
class Array {
};

/**
 * @class Global
 * @brief Empty class tag to push the global object.
 */
class Global {
};

/**
 * @class Undefined
 * @brief Empty class tag to push undefined to the stack.
 */
class Undefined {
};

/**
 * @class Null
 * @brief Empty class tag to push null to the stack.
 */
class Null {
};

/**
 * @class This
 * @brief Empty class tag to push this binding to the stack.
 */
class This {
};

/**
 * @class RawPointer
 * @brief Push a non-managed pointer to Duktape, the pointer will never be deleted.
 * @note For a managed pointer with prototype, see Pointer
 */
template <typename T>
class RawPointer {
public:
	/**
	 * The pointer to push.
	 */
	T *object;
};

/**
 * @brief Manage shared_ptr from C++ and JavaScript
 *
 * This class allowed you to push and retrieve shared_ptr from C++ and JavaScript without taking care of ownership
 * and deletion.
 *
 */
template <typename T>
class Shared {
public:
	/**
	 * The shared object.
	 */
	std::shared_ptr<T> object;
};

/**
 * @brief Manage pointers from C++ and JavaScript
 *
 * This class allowed you to push and retrieve C++ pointers from C++ and JavaScript. The object will be deleted when
 * the JavaScript garbage collectors collect them so never store a pointer created with this.
 *
 * The only requirement is to have the function `void prototype(Context &ctx)` in your class T.
 */
template <typename T>
class Pointer {
public:
	/**
	 * The object.
	 */
	T *object{nullptr};
};

/**
 * @class Function
 * @brief Duktape/C function definition.
 *
 * This class wraps the std::function as a Duktape/C function by storing a copied pointer.
 */
class Function {
public:
	/**
	 * The function pointer, must not be null.
	 */
	duk_c_function function;

	/**
	 * Number of args that the function takes
	 */
	duk_idx_t nargs{0};
};

/**
 * Map of functions to set on an object.
 */
using FunctionMap = std::unordered_map<std::string, Function>;

/**
 * Map of string to type, ideal for setting constants like enums.
 */
template <typename Type>
using Map = std::unordered_map<std::string, Type>;

/**
 * @class ErrorInfo
 * @brief Error description.
 *
 * This class fills the fields got in an Error object.
 */
class ErrorInfo : public std::exception {
public:
	std::string name;		//!< name of error
	std::string message;		//!< error message
	std::string stack;		//!< stack if available
	std::string fileName;		//!< filename if applicable
	int lineNumber{0};		//!< line number if applicable

	/**
	 * Get the error message. This effectively returns message field.
	 *
	 * @return the message
	 */
	const char *what() const noexcept override
	{
		return message.c_str();
	}
};

/**
 * @class TypeTraits
 * @brief Type information to implement new types in JavaScript's context.
 *
 * This class depending on your needs may have the following functions:
 *
 * - `static void construct(Context &ctx, Type value)`
 * - `static Type get(Context &ctx, int index)`
 * - `static bool is(Context &ctx, int index)`
 * - `static Type optional(Context &ctx, int index, Type defaultValue)`
 * - `static void push(Context &ctx, Type value)`
 * - `static Type require(Context &ctx, int index)`
 *
 * The `construct` function is used in Context::construct to build a new value as this (e.g. constructors).
 *
 * The `get` function is used in Context::get, Context::getProperty, Context::getGlobal to retrieve a value from the
 * stack.
 *
 * The `is` function is used in Context::is to check if the value on the stack is of type `Type`.
 *
 * The `optional` function is used in Context::optional to get a value or a replacement if not applicable.
 *
 * The `push` function is used in Context::push to usually create a new value on the stack but some specializations
 * may not (e.g. FunctionMap).
 *
 * The `require` function is used in Context::require to get a value from the stack or raise a JavaScript exception if
 * not applicable.
 *
 * This class is fully specialized for: `bool`, `const char *`, `double`, `int`, `std::string`.
 *
 * It is also partially specialized for : `Global`, `Object`, `Array`, `Undefined`, `Null`, `std::vector<Type>`.
 */
template <typename Type>
class TypeTraits {
};

/**
 * @class Context
 * @brief RAII based Duktape handler.
 *
 * This class is implicitly convertible to duk_context for convenience.
 */
class Context {
private:
	using Deleter = void (*)(duk_context *);
	using Handle = std::unique_ptr<duk_context, Deleter>;

	Handle m_handle;

	Context(const Context &) = delete;
	Context &operator=(const Context &) = delete;
	Context(const Context &&) = delete;
	Context &operator=(const Context &&) = delete;

public:
	/**
	 * Create default context.
	 */
	inline Context()
		: m_handle(duk_create_heap_default(), duk_destroy_heap)
	{
	}

	/**
	 * Convert the context to the native Duktape/C type.
	 *
	 * @return the duk_context
	 */
	inline operator duk_context *() noexcept
	{
		return m_handle.get();
	}

	/**
	 * Convert the context to the native Duktape/C type.
	 *
	 * @return the duk_context
	 */
	inline operator duk_context *() const noexcept
	{
		return m_handle.get();
	}
};

/**
 * Get the error object when a JavaScript error has been thrown (e.g. eval failure).
 *
 * @param ctx the context
 * @param index the index
 * @return the information
 */
inline ErrorInfo error(ContextPtr ctx, int index)
{
	ErrorInfo error;

	index = duk_normalize_index(ctx, index);

	duk_get_prop_string(ctx, index, "name");
	error.name = duk_to_string(ctx, -1);
	duk_get_prop_string(ctx, index, "message");
	error.message = duk_to_string(ctx, -1);
	duk_get_prop_string(ctx, index, "fileName");
	error.fileName = duk_to_string(ctx, -1);
	duk_get_prop_string(ctx, index, "lineNumber");
	error.lineNumber = duk_to_int(ctx, -1);
	duk_get_prop_string(ctx, index, "stack");
	error.stack = duk_to_string(ctx, -1);
	duk_pop_n(ctx, 5);

	return error;
}

/**
 * Wrapper for [duk_base64_decode](http://duktape.org/api.html#duk_base64_decode).
 *
 * @param ctx the context
 * @param index the index
 */
inline void base64Decode(ContextPtr ctx, Index index)
{
	duk_base64_decode(ctx, index);
}

/**
 * Wrapper for [duk_base64_encode](http://duktape.org/api.html#duk_base64_encode).
 *
 * @param ctx the context
 * @param index the index
 */
inline void base64Encode(ContextPtr ctx, Index index)
{
	duk_base64_encode(ctx, index);
}

/**
 * Wrapper for [duk_call](http://duktape.org/api.html#duk_call).
 *
 * @param ctx the context
 * @param nargs the number of arguments
 */
inline void call(ContextPtr ctx, Index nargs = 0)
{
	duk_call(ctx, nargs);
}

/**
 * Wrapper for [duk_call_method](http://duktape.org/api.html#duk_call_method).
 *
 * @param ctx the context
 * @param nargs the number of arguments
 */
inline void callMethod(ContextPtr ctx, Index nargs = 0)
{
	duk_call_method(ctx, nargs);
}

/**
 * Wrapper for [duk_call_prop](http://duktape.org/api.html#duk_call_prop).
 *
 * @param ctx the context
 * @param index the object index
 * @param nargs the number of arguments
 */
inline void callProperty(ContextPtr ctx, Index index, Index nargs = 0)
{
	duk_call_prop(ctx, index, nargs);
}

/**
 * Wrapper for [duk_char_code_at](http://duktape.org/api.html#duk_char_code_at).
 *
 * @param ctx the context
 * @param index the index
 * @param charOffset the offset
 */
inline CodePoint charCodeAt(ContextPtr ctx, Index index, Size charOffset)
{
	return duk_char_code_at(ctx, index, charOffset);
}

/**
 * Wrapper for [duk_check_stack](http://duktape.org/api.html#duk_check_stack).
 *
 * @param ctx the context
 * @param extra the extra space
 * @return true if space is available
 */
inline bool checkStack(ContextPtr ctx, Index extra)
{
	return duk_check_stack(ctx, extra);
}

/**
 * Wrapper for [duk_check_stack_top](http://duktape.org/api.html#duk_check_stack_top).
 *
 * @param ctx the context
 * @param top the extra space
 * @return true if space is available
 */
inline bool checkStackTop(ContextPtr ctx, Index top)
{
	return duk_check_stack_top(ctx, top);
}

/**
 * Wrapper for [duk_check_type](http://duktape.org/api.html#duk_check_type).
 *
 * @param ctx the context
 * @param index the value index
 * @param type the desired type
 * @return true if object is given type
 */
inline bool checkType(ContextPtr ctx, Index index, int type)
{
	return duk_check_type(ctx, index, type);
}

/**
 * Wrapper for [duk_check_type_mask](http://duktape.org/api.html#duk_check_type_mask).
 *
 * @param ctx the context
 * @param index the value index
 * @param mask the desired mask
 * @return true if object is one of the type
 */
inline bool checkTypeMask(ContextPtr ctx, Index index, unsigned mask)
{
	return duk_check_type_mask(ctx, index, mask);
}

/**
 * Wrapper for [duk_compact](http://duktape.org/api.html#duk_compact).
 *
 * @param ctx the context
 * @param objIndex the object index
 */
inline void compact(ContextPtr ctx, Index objIndex)
{
	duk_compact(ctx, objIndex);
}

/**
 * Wrapper for [duk_concat](http://duktape.org/api.html#duk_concat).
 *
 * @param ctx the context
 * @param count the number of values
 */
inline void concat(ContextPtr ctx, Index count)
{
	duk_concat(ctx, count);
}

/**
 * Wrapper for [duk_copy](http://duktape.org/api.html#duk_copy).
 *
 * @param from the from index
 * @param to the destination
 */
inline void copy(ContextPtr ctx, Index from, Index to)
{
	duk_copy(ctx, from, to);
}

/**
 * Wrapper for [duk_def_prop](http://duktape.org/api.html#duk_def_prop).
 *
 * @param index the object index
 * @param flags the flags
 */
inline void defineProperty(ContextPtr ctx, Index index, unsigned flags)
{
	duk_def_prop(ctx, index, flags);
}

/**
 * Wrapper for [duk_del_prop](http://duktape.org/api.html#duk_del_prop).
 *
 * @param index the object index
 * @return true if deleted
 */
inline bool deleteProperty(ContextPtr ctx, Index index)
{
	return duk_del_prop(ctx, index);
}

/**
 * Wrapper for [duk_del_prop](http://duktape.org/api.html#duk_del_prop).
 *
 * @param index the object index
 * @param position the property index
 * @return true if deleted
 */
inline bool deleteProperty(ContextPtr ctx, Index index, unsigned position)
{
	return duk_del_prop_index(ctx, index, position);
}

/**
 * Wrapper for [duk_del_prop](http://duktape.org/api.html#duk_del_prop).
 *
 * @param index the object index
 * @param name the property name
 * @return true if deleted
 */
inline bool deleteProperty(ContextPtr ctx, Index index, const std::string &name)
{
	return duk_del_prop_string(ctx, index, name.c_str());
}

/**
 * Wrapper for [duk_dup](http://duktape.org/api.html#duk_dup).
 *
 * @param index the value to copy
 */
inline void dup(ContextPtr ctx, int index = -1)
{
	duk_dup(ctx, index);
}

/**
 * Wrapper for [duk_equals](http://duktape.org/api.html#duk_equals).
 *
 * @param ctx the context
 * @param index1 the first value
 * @param index2 the second value
 * @return true if they equal
 */
inline bool equals(ContextPtr ctx, Index index1, Index index2)
{
	return duk_equals(ctx, index1, index2);
}

/**
 * Wrapper for [duk_eval](http://duktape.org/api.html#duk_eval).
 *
 * @param ctx the context
 */
inline void eval(ContextPtr ctx)
{
	duk_eval(ctx);
}

/**
 * Wrapper for [duk_eval_file](http://duktape.org/api.html#duk_eval_file).
 *
 * @param ctx the context
 * @param path the path
 * @param result true to get the result at the top of the stack
 */
inline void evalFile(ContextPtr ctx, const std::string &path, bool result = true)
{
	if (result)
		duk_eval_file(ctx, path.c_str());
	else
		duk_eval_file_noresult(ctx, path.c_str());
}

/**
 * Wrapper for [duk_eval_string](http://duktape.org/api.html#duk_eval_string).
 *
 * @param ctx the context
 * @param src the source script
 * @param result true to get the result at the top of the stack
 */
inline void evalString(ContextPtr ctx, const std::string &src, bool result = true)
{
	if (result)
		duk_eval_string(ctx, src.c_str());
	else
		duk_eval_string_noresult(ctx, src.c_str());
}
/**
 * Wrapper for [duk_gc](http://duktape.org/api.html#duk_gc).
 *
 * @param ctx the context
 * @param flags the flags
 */
inline void gc(ContextPtr ctx, unsigned flags = 0)
{
	duk_gc(ctx, flags);
}

/**
 * Wrapper for [duk_has_prop](http://duktape.org/api.html#duk_has_prop).
 *
 * @param ctx the context
 * @param index the object index
 * @return true if has
 */
inline bool hasProperty(ContextPtr ctx, Index index)
{
	return duk_has_prop(ctx, index);
}

/**
 * Wrapper for [duk_has_prop](http://duktape.org/api.html#duk_has_prop).
 *
 * @param ctx the context
 * @param index the object index
 * @param position the property index
 * @return true if has
 */
inline bool hasProperty(ContextPtr ctx, Index index, unsigned position)
{
	return duk_has_prop_index(ctx, index, position);
}

/**
 * Wrapper for [duk_has_prop](http://duktape.org/api.html#duk_has_prop).
 *
 * @param ctx the context
 * @param index the object index
 * @param name the property name
 * @return true if has
 */
inline bool hasProperty(ContextPtr ctx, int index, const std::string &name)
{
	return duk_has_prop_string(ctx, index, name.c_str());
}

/**
 * Wrapper for [duk_insert](http://duktape.org/api.html#duk_insert).
 *
 * @param ctx the context
 * @param to the destination
 * @note Wrapper of duk_insert
 */
inline void insert(ContextPtr ctx, Index to)
{
	duk_insert(ctx, to);
}

/**
 * Wrapper for [duk_instanceof](http://duktape.org/api.html#duk_instanceof).
 *
 * @param ctx the context
 * @param idx1 the value to test
 * @param idx2 the instance requested
 * @return true if idx1 is instance of idx2
 */
inline bool instanceof(ContextPtr ctx, Index idx1, Index idx2)
{
	return duk_instanceof(ctx, idx1, idx2);
}

/**
 * Wrapper for [duk_join](http://duktape.org/api.html#duk_join).
 *
 * @param ctx the context
 * @param count the number of values
 */
inline void join(ContextPtr ctx, Index count)
{
	duk_join(ctx, count);
}

/**
 * Wrapper for [duk_json_decode](http://duktape.org/api.html#duk_json_decode).
 *
 * @param ctx the context
 * @param index the index
 */
inline void jsonDecode(ContextPtr ctx, Index index)
{
	duk_json_decode(ctx, index);
}

/**
 * Wrapper for [duk_json_encode](http://duktape.org/api.html#duk_json_encode).
 *
 * @param ctx the context
 * @param index the index
 */
inline void jsonEncode(ContextPtr ctx, Index index)
{
	duk_json_encode(ctx, index);
}

/**
 * Wrapper for [duk_normalize_index](http://duktape.org/api.html#duk_normalize_index).
 *
 * @param ctx the context
 * @param index the index
 */
inline Index normalizeIndex(ContextPtr ctx, Index index)
{
	return duk_normalize_index(ctx, index);
}

/**
 * Wrapper for [duk_pcall](http://duktape.org/api.html#duk_pcall).
 *
 * @param ctx the context
 * @param nargs the number of arguments
 */
inline int pcall(ContextPtr ctx, Index nargs = 0)
{
	return duk_pcall(ctx, nargs);
}

/**
 * Wrapper for [duk_peval](http://duktape.org/api.html#duk_peval).
 *
 * @param ctx the context
 */
inline int peval(ContextPtr ctx)
{
	return duk_peval(ctx);
}

/**
 * Wrapper for [duk_peval_file](http://duktape.org/api.html#duk_peval_file).
 *
 * @param ctx the context
 * @param path the path
 * @param result true to get the result at the top of the stack
 */
inline int pevalFile(ContextPtr ctx, const std::string &path, bool result = true)
{
	return result ? duk_peval_file(ctx, path.c_str()) : duk_peval_file_noresult(ctx, path.c_str());
}

/**
 * Wrapper for [duk_peval_string](http://duktape.org/api.html#duk_peval_string).
 *
 * @param ctx the context
 * @param src the source script
 * @param result true to get the result at the top of the stack
 */
inline int pevalString(ContextPtr ctx, const std::string &src, bool result = true)
{
	return result ? duk_peval_string(ctx, src.c_str()) : duk_peval_string_noresult(ctx, src.c_str());
}

/**
 * Wrapper for [duk_pop_n](http://duktape.org/api.html#duk_pop_n).
 *
 * @param ctx the context
 * @param count the number of values to pop
 */
inline void pop(ContextPtr ctx, Index count = 1)
{
	duk_pop_n(ctx, count);
}

/**
 * Wrapper for [duk_remove](http://duktape.org/api.html#duk_remove).
 *
 * @param ctx the context
 * @param index the value to remove
 */
inline void remove(ContextPtr ctx, Index index)
{
	duk_remove(ctx, index);
}

/**
 * Wrapper for [duk_replace](http://duktape.org/api.html#duk_replace).
 *
 * @param ctx the context
 * @param index the value to replace by the value at the top of the stack
 */
inline void replace(ContextPtr ctx, Index index)
{
	duk_replace(ctx, index);
}

/**
 * Wrapper for [duk_set_prototype](http://duktape.org/api.html#duk_set_prototype).
 *
 * @param ctx the context
 * @param index the value index
 */
inline void setPrototype(ContextPtr ctx, Index index)
{
	duk_set_prototype(ctx, index);
}

/**
 * Wrapper for [duk_swap](http://duktape.org/api.html#duk_swap).
 *
 * @param ctx the context
 * @param index1 the first index
 * @param index2 the second index
 */
inline void swap(ContextPtr ctx, Index index1, Index index2)
{
	duk_swap(ctx, index1, index2);
}

/**
 * Wrapper for [duk_swap_top](http://duktape.org/api.html#duk_swap_top).
 *
 * @param ctx the context
 * @param index the index
 */
inline void swapTop(ContextPtr ctx, Index index)
{
	duk_swap_top(ctx, index);
}

/**
 * Wrapper for [duk_get_top](http://duktape.org/api.html#duk_get_top).
 *
 * @param ctx the context
 * @return the stack size
 */
inline int top(ContextPtr ctx) noexcept
{
	return duk_get_top(ctx);
}

/**
 * Wrapper for [duk_get_type](http://duktape.org/api.html#duk_get_type).
 *
 * @param ctx the context
 * @param index the idnex
 * @return the type
 */
inline int type(ContextPtr ctx, Index index) noexcept
{
	return duk_get_type(ctx, index);
}

/*
 * Push / Get / Require / Is / Optional
 * ----------------------------------------------------------
 *
 * The following functions are used to push, get or check values from the stack. They use specialization
 * of TypeTraits class.
 */

/**
 * Push a value into the stack. Calls TypeTraits<T>::push(*this, value);
 *
 * @param value the value to forward
 */
template <typename Type>
inline void push(ContextPtr ctx, Type &&value)
{
	TypeTraits<std::decay_t<Type>>::push(ctx, std::forward<Type>(value));
}

/**
 * Generic template function to get a value from the stack.
 *
 * @param index the index
 * @return the value
 */
template <typename Type>
inline auto get(ContextPtr ctx, int index) -> decltype(TypeTraits<Type>::get(ctx, 0))
{
	return TypeTraits<Type>::get(ctx, index);
}

/**
 * Require a type at the specified index.
 *
 * @param index the index
 * @return the value
 */
template <typename Type>
inline auto require(ContextPtr ctx, int index) -> decltype(TypeTraits<Type>::require(ctx, 0))
{
	return TypeTraits<Type>::require(ctx, index);
}

/**
 * Check if a value is a type of T.
 *
 * The TypeTraits<T> must have `static bool is(ContextPtr ptr, int index)`.
 *
 * @param index the value index
 * @return true if is the type
 */
template <typename T>
inline bool is(ContextPtr ctx, int index)
{
	return TypeTraits<T>::is(ctx, index);
}

/**
 * Get an optional value from the stack, if the value is not available of not the correct type,
 * return defaultValue instead.
 *
 * The TypeTraits<T> must have `static T optional(Context &, int index, T &&defaultValue)`.
 *
 * @param index the value index
 * @param defaultValue the value replacement
 * @return the value or defaultValue
 */
template <typename Type>
inline auto optional(ContextPtr ctx, int index, Type &&defaultValue)
{
	return TypeTraits<std::decay_t<Type>>::optional(ctx, index, std::forward<Type>(defaultValue));
}

/*
 * Properties management
 * ----------------------------------------------------------
 *
 * The following functions are used to read or set properties on objects or globals also using TypeTraits.
 */

/**
 * Get the property `name' as value from the object at the specified index.
 *
 * @param index the object index
 * @param name the property name
 * @return the value
 * @note The stack is unchanged
 */
template <typename Type, typename std::enable_if_t<!std::is_void<Type>::value> * = nullptr>
inline auto getProperty(ContextPtr ctx, int index, const std::string &name) -> decltype(get<Type>(ctx, 0))
{
	duk_get_prop_string(ctx, index, name.c_str());
	decltype(get<Type>(ctx, 0)) value = get<Type>(ctx, -1);
	duk_pop(ctx);

	return value;
}

/**
 * Get a property by index, for arrays.
 *
 * @param index the object index
 * @param position the position int the object
 * @return the value
 * @note The stack is unchanged
 */
template <typename Type, typename std::enable_if_t<!std::is_void<Type>::value> * = nullptr>
inline auto getProperty(ContextPtr ctx, int index, int position) -> decltype(get<Type>(ctx, 0))
{
	duk_get_prop_index(ctx, index, position);
	decltype(get<Type>(ctx, 0)) value = get<Type>(ctx, -1);
	duk_pop(ctx);

	return value;
}

/**
 * Get the property `name' and push it to the stack from the object at the specified index.
 *
 * @param index the object index
 * @param name the property name
 * @note The stack contains the property value
 */
template <typename Type, typename std::enable_if_t<std::is_void<Type>::value> * = nullptr>
inline void getProperty(ContextPtr ctx, int index, const std::string &name)
{
	duk_get_prop_string(ctx, index, name.c_str());
}

/**
 * Get the property by index and push it to the stack from the object at the specified index.
 *
 * @param index the object index
 * @param position the position in the object
 * @note The stack contains the property value
 */
template <typename Type, typename std::enable_if_t<std::is_void<Type>::value> * = nullptr>
inline void getProperty(ContextPtr ctx, int index, int position)
{
	duk_get_prop_index(ctx, index, position);
}

/**
 * Get an optional property `name` from the object at the specified index.
 *
 * @param index the object index
 * @param name the property name
 * @param def the default value
 * @return the value or def
 * @note The stack is unchanged
 */
template <typename Type, typename DefaultValue>
inline auto optionalProperty(ContextPtr ctx, int index, const std::string &name, DefaultValue &&def) -> decltype(optional(ctx, 0, std::forward<DefaultValue>(def)))
{
	duk_get_prop_string(ctx, index, name.c_str());
	decltype(optional(ctx, 0, std::forward<DefaultValue>(def))) value = optional(ctx, -1, std::forward<DefaultValue>(def));
	duk_pop(ctx);

	return value;
}

/**
 * Get an optional property by index, for arrays
 *
 * @param index the object index
 * @param position the position int the object
 * @param def the default value
 * @return the value or def
 * @note The stack is unchanged
 */
template <typename Type, typename DefaultValue>
inline auto optionalProperty(ContextPtr ctx, int index, int position, DefaultValue &&def) -> decltype(optional(ctx, 0, std::forward<DefaultValue>(def)))
{
	duk_get_prop_index(ctx, index, position);
	decltype(optional(ctx, 0, std::forward<DefaultValue>(def))) value = optional(ctx, -1, std::forward<DefaultValue>(def));
	duk_pop(ctx);

	return value;
}

/**
 * Set a property to the object at the specified index.
 *
 * @param index the object index
 * @param name the property name
 * @param value the value to forward
 * @note The stack is unchanged
 */
template <typename Type>
void putProperty(ContextPtr ctx, int index, const std::string &name, Type &&value)
{
	index = duk_normalize_index(ctx, index);

	push(ctx, std::forward<Type>(value));
	duk_put_prop_string(ctx, index, name.c_str());
}

/**
 * Set a property by index, for arrays.
 *
 * @param index the object index
 * @param position the position in the object
 * @param value the value to forward
 * @note The stack is unchanged
 */
template <typename Type>
void putProperty(ContextPtr ctx, int index, int position, Type &&value)
{
	index = duk_normalize_index(ctx, index);

	push(ctx, std::forward<Type>(value));
	duk_put_prop_index(ctx, index, position);
}

/**
 * Put the value that is at the top of the stack as property to the object.
 *
 * @param index the object index
 * @param name the property name
 */
inline void putProperty(ContextPtr ctx, int index, const std::string &name)
{
	duk_put_prop_string(ctx, index, name.c_str());
}

/**
 * Put the value that is at the top of the stack to the object as index.
 *
 * @param index the object index
 * @param position the position in the object
 */
inline void putProperty(ContextPtr ctx, int index, int position)
{
	duk_put_prop_index(ctx, index, position);
}

/**
 * Get a global value.
 *
 * @param name the name of the global variable
 * @return the value
 */
template <typename Type>
inline auto getGlobal(ContextPtr ctx, const std::string &name, std::enable_if_t<!std::is_void<Type>::value> * = nullptr) -> decltype(get<Type>(ctx, 0))
{
	duk_get_global_string(ctx, name.c_str());
	decltype(get<Type>(ctx, 0)) value = get<Type>(ctx, -1);
	duk_pop(ctx);

	return value;
}

/**
 * Overload that push the value at the top of the stack instead of returning it.
 */
template <typename Type>
inline void getGlobal(ContextPtr ctx, const std::string &name, std::enable_if_t<std::is_void<Type>::value> * = nullptr) noexcept
{
	duk_get_global_string(ctx, name.c_str());
}

/**
 * Set a global variable.
 *
 * @param name the name of the global variable
 * @param type the value to set
 */
template <typename Type>
inline void putGlobal(ContextPtr ctx, const std::string &name, Type&& type)
{
	push(ctx, std::forward<Type>(type));
	duk_put_global_string(ctx, name.c_str());
}

/**
 * Put the value at the top of the stack as global property.
 *
 * @param name the property name
 */
inline void putGlobal(ContextPtr ctx, const std::string &name)
{
	duk_put_global_string(ctx, name.c_str());
}

/*
 * Extra functions
 * ----------------------------------------------------------
 *
 * The following functions are implemented for convenience and do not exists in the native Duktape API.
 */

/**
 * Enumerate an object or an array at the specified index.
 *
 * @param index the object or array index
 * @param flags the optional flags to pass to duk_enum
 * @param getvalue set to true if you want to extract the value
 * @param func the function to call for each properties
 */
template <typename Func>
void enumerate(ContextPtr ctx, int index, duk_uint_t flags, duk_bool_t getvalue, Func &&func)
{
	duk_enum(ctx, index, flags);

	while (duk_next(ctx, -1, getvalue)) {
		func(ctx);
		duk_pop_n(ctx, 1 + (getvalue ? 1 : 0));
	}

	duk_pop(ctx);
}

/**
 * Return the this binding of the current function.
 *
 * @return the this binding as the template given
 */
template <typename T>
inline auto self(ContextPtr ctx) -> decltype(TypeTraits<T>::get(ctx, 0))
{
	duk_push_this(ctx);
	decltype(TypeTraits<T>::get(ctx, 0)) value = TypeTraits<T>::get(ctx, -1);
	duk_pop(ctx);

	return value;
}

/**
 * Throw an ECMAScript exception.
 *
 * @param ex the exception
 */
template <typename Exception>
void raise(ContextPtr ctx, const Exception &ex)
{
	ex.raise(ctx);
}

/**
 * Wrapper for duk_throw.
 *
 * @param ctx the context
 */
inline void raise(ContextPtr ctx)
{
	duk_throw(ctx);
}

/**
 * Wrapper for duk_error.
 *
 * @param ctx the context
 * @param type the error type (e.g. DUK_ERR_REFERENCE_ERROR)
 * @param fmt the format string
 * @param args the arguments
 */
template <typename... Args>
inline void raise(ContextPtr ctx, int type, const char *fmt, Args&&... args)
{
	duk_error(ctx, type, fmt, std::forward<Args>(args)...);
}

/**
 * Wrapper for duk_new.
 *
 * @param ctx the context
 * @param nargs the number of arguments
 */
inline void create(ContextPtr ctx, int nargs = 0)
{
	duk_new(ctx, nargs);
}

/**
 * Construct the object in place, setting value as this binding.
 *
 * The TypeTraits<T> must have the following requirements:
 *
 * - static void construct(Context &, T): must update this with the value and keep the stack unchanged
 *
 * @param value the value to forward
 * @see self
 */
template <typename T>
inline void construct(ContextPtr ctx, T &&value)
{
	TypeTraits<std::decay_t<T>>::construct(ctx, std::forward<T>(value));
}

/**
 * Sign the given object with the name from T.
 *
 * This is automatically done for when constructing/pushing object with Shared and Pointer helpers, however you need
 * to manually add it when using inheritance.
 */
template <typename T>
inline void sign(ContextPtr ctx, Index index)
{
	StackAssert sa(ctx, 0);

	index = duk_normalize_index(ctx, index);

	duk_push_string(ctx, TypeTraits<T>::name().c_str());
	duk_push_boolean(ctx, true);
	duk_def_prop(ctx, index < 0 ? index : index, DUK_DEFPROP_HAVE_VALUE);

	/* Do for inherited classes */
	for (const std::string &parent : TypeTraits<T>::inherits()) {
		duk_push_string(ctx, parent.c_str());
		duk_push_boolean(ctx, true);
		duk_def_prop(ctx, index < 0 ? index : index, DUK_DEFPROP_HAVE_VALUE);
	}
}

/**
 * Check if the object at the given index is signed by T or raise TypeError if not.
 *
 * @param ctx the context
 * @param index the index
 * @see sign
 */
template <typename T>
inline void checkSignature(ContextPtr ctx, Index index)
{
	StackAssert sa(ctx, 0);

	if (!is<Object>(ctx, index) || !getProperty<bool>(ctx, index, TypeTraits<T>::name()))
		raise(ctx, DUK_ERR_TYPE_ERROR, "invalid this binding");
}

/**
 * Tells if the object at the specified index is of type T.
 *
 * @param ctx the context
 * @param index the index
 */
template <typename T>
inline bool isSigned(ContextPtr ctx, Index index)
{
	StackAssert sa(ctx, 0);

	return is<Object>(ctx, index) && getProperty<bool>(ctx, index, TypeTraits<T>::name());
}

/* ------------------------------------------------------------------
 * Exception handling
 * ------------------------------------------------------------------ */

/**
 * @class Error
 * @brief Base ECMAScript error class.
 * @warning Override the function create for your own exceptions
 */
class Error {
private:
	int m_type{DUK_ERR_ERROR};
	std::string m_message;

protected:
	/**
	 * Constructor with a type of error specified, specially designed for derived errors.
	 *
	 * @param type of error (e.g. DUK_ERR_ERROR)
	 * @param message the message
	 */
	inline Error(int type, std::string message) noexcept
		: m_type(type)
		, m_message(std::move(message))
	{
	}

public:
	/**
	 * Constructor with a message.
	 *
	 * @param message the message
	 */
	inline Error(std::string message) noexcept
		: m_message(std::move(message))
	{
	}

	/**
	 * Create the exception on the stack.
	 *
	 * @note the default implementation search for the global variables
	 * @param ctx the context
	 */
	virtual void raise(ContextPtr ctx) const noexcept
	{
		duk_error(ctx, m_type, "%s", m_message.c_str());
	}
};

/**
 * @class EvalError
 * @brief Error in eval() function.
 */
class EvalError : public Error {
public:
	/**
	 * Construct an EvalError.
	 *
	 * @param message the message
	 */
	inline EvalError(std::string message) noexcept
		: Error(DUK_ERR_EVAL_ERROR, std::move(message))
	{
	}
};

/**
 * @class RangeError
 * @brief Value is out of range.
 */
class RangeError : public Error {
public:
	/**
	 * Construct an RangeError.
	 *
	 * @param message the message
	 */
	inline RangeError(std::string message) noexcept
		: Error(DUK_ERR_RANGE_ERROR, std::move(message))
	{
	}
};

/**
 * @class ReferenceError
 * @brief Trying to use a variable that does not exist.
 */
class ReferenceError : public Error {
public:
	/**
	 * Construct an ReferenceError.
	 *
	 * @param message the message
	 */
	inline ReferenceError(std::string message) noexcept
		: Error(DUK_ERR_REFERENCE_ERROR, std::move(message))
	{
	}
};

/**
 * @class SyntaxError
 * @brief Syntax error in the script.
 */
class SyntaxError : public Error {
public:
	/**
	 * Construct an SyntaxError.
	 *
	 * @param message the message
	 */
	inline SyntaxError(std::string message) noexcept
		: Error(DUK_ERR_SYNTAX_ERROR, std::move(message))
	{
	}
};

/**
 * @class TypeError
 * @brief Invalid type given.
 */
class TypeError : public Error {
public:
	/**
	 * Construct an TypeError.
	 *
	 * @param message the message
	 */
	inline TypeError(std::string message) noexcept
		: Error(DUK_ERR_TYPE_ERROR, std::move(message))
	{
	}
};

/**
 * @class URIError
 * @brief URI manipulation failure.
 */
class URIError : public Error {
public:
	/**
	 * Construct an URIError.
	 *
	 * @param message the message
	 */
	inline URIError(std::string message) noexcept
		: Error(DUK_ERR_URI_ERROR, std::move(message))
	{
	}
};

/* ------------------------------------------------------------------
 * Standard overloads for TypeTraits<T>
 * ------------------------------------------------------------------ */

/**
 * @class TypeTraits<int>
 * @brief Default implementation for int.
 *
 * Provides: get, is, optional, push, require.
 */
template <>
class TypeTraits<int> {
public:
	/**
	 * Get an integer, return 0 if not an integer.
	 *
	 * @param ctx the context
	 * @param index the index
	 * @return the integer
	 */
	static inline int get(ContextPtr ctx, int index)
	{
		return duk_get_int(ctx, index);
	}

	/**
	 * Check if value is an integer.
	 *
	 * @param ctx the context
	 * @param index the index
	 * @return true if integer
	 */
	static inline bool is(ContextPtr ctx, int index)
	{
		return duk_is_number(ctx, index);
	}

	/**
	 * Get an integer, return defaultValue if the value is not an integer.
	 *
	 * @param ctx the context
	 * @param index the index
	 * @param defaultValue the defaultValue
	 * @return the integer or defaultValue
	 */
	static inline int optional(ContextPtr ctx, int index, int defaultValue)
	{
		return is(ctx, index) ? get(ctx, index) : defaultValue;
	}

	/**
	 * Push an integer.
	 *
	 * @param ctx the context
	 * @param value the value
	 */
	static inline void push(ContextPtr ctx, int value)
	{
		duk_push_int(ctx, value);
	}

	/**
	 * Require an integer, throws a JavaScript exception if not an integer.
	 *
	 * @param ctx the context
	 * @param index the index
	 * @return the integer
	 */
	static inline int require(ContextPtr ctx, int index)
	{
		return duk_require_int(ctx, index);
	}
};

/**
 * @class TypeTraits<bool>
 * @brief Default implementation for bool.
 *
 * Provides: get, is, optional, push, require.
 */
template <>
class TypeTraits<bool> {
public:
	/**
	 * Get a boolean, return 0 if not a boolean.
	 *
	 * @param ctx the context
	 * @param index the index
	 * @return the boolean
	 */
	static inline bool get(ContextPtr ctx, int index)
	{
		return duk_get_boolean(ctx, index);
	}

	/**
	 * Check if value is a boolean.
	 *
	 * @param ctx the context
	 * @param index the index
	 * @return true if boolean
	 */
	static inline bool is(ContextPtr ctx, int index)
	{
		return duk_is_boolean(ctx, index);
	}

	/**
	 * Get a bool, return defaultValue if the value is not a boolean.
	 *
	 * @param ctx the context
	 * @param index the index
	 * @param defaultValue the defaultValue
	 * @return the boolean or defaultValue
	 */
	static inline bool optional(ContextPtr ctx, int index, bool defaultValue)
	{
		return is(ctx, index) ? get(ctx, index) : defaultValue;
	}

	/**
	 * Push a boolean.
	 *
	 * @param ctx the context
	 * @param value the value
	 */
	static inline void push(ContextPtr ctx, bool value)
	{
		duk_push_boolean(ctx, value);
	}

	/**
	 * Require a boolean, throws a JavaScript exception if not a boolean.
	 *
	 * @param ctx the context
	 * @param index the index
	 * @return the boolean
	 */
	static inline bool require(ContextPtr ctx, int index)
	{
		return duk_require_boolean(ctx, index);
	}
};

/**
 * @class TypeTraits<double>
 * @brief Default implementation for double.
 *
 * Provides: get, is, optional, push, require.
 */
template <>
class TypeTraits<double> {
public:
	/**
	 * Get a double, return 0 if not a double.
	 *
	 * @param ctx the context
	 * @param index the index
	 * @return the double
	 */
	static inline double get(ContextPtr ctx, int index)
	{
		return duk_get_number(ctx, index);
	}

	/**
	 * Check if value is a double.
	 *
	 * @param ctx the context
	 * @param index the index
	 * @return true if double
	 */
	static inline bool is(ContextPtr ctx, int index)
	{
		return duk_is_number(ctx, index);
	}

	/**
	 * Get a double, return defaultValue if the value is not a double.
	 *
	 * @param ctx the context
	 * @param index the index
	 * @param defaultValue the defaultValue
	 * @return the double or defaultValue
	 */
	static inline double optional(ContextPtr ctx, int index, double defaultValue)
	{
		return is(ctx, index) ? get(ctx, index) : defaultValue;
	}

	/**
	 * Push a double.
	 *
	 * @param ctx the context
	 * @param value the value
	 */
	static inline void push(ContextPtr ctx, double value)
	{
		duk_push_number(ctx, value);
	}

	/**
	 * Require a double, throws a JavaScript exception if not a double.
	 *
	 * @param ctx the context
	 * @param index the index
	 * @return the double
	 */
	static inline double require(ContextPtr ctx, int index)
	{
		return duk_require_number(ctx, index);
	}
};

/**
 * @class TypeTraits<std::string>
 * @brief Default implementation for std::string.
 *
 * Provides: get, is, optional, push, require.
 *
 * Note: the functions allows embedded '\0'.
 */
template <>
class TypeTraits<std::string> {
public:
	/**
	 * Get a string, return 0 if not a string.
	 *
	 * @param ctx the context
	 * @param index the index
	 * @return the string
	 */
	static inline std::string get(ContextPtr ctx, int index)
	{
		duk_size_t size;
		const char *text = duk_get_lstring(ctx, index, &size);

		return std::string{text, size};
	}

	/**
	 * Check if value is a string.
	 *
	 * @param ctx the context
	 * @param index the index
	 * @return true if string
	 */
	static inline bool is(ContextPtr ctx, int index)
	{
		return duk_is_string(ctx, index);
	}

	/**
	 * Get a string, return defaultValue if the value is not an string.
	 *
	 * @param ctx the context
	 * @param index the index
	 * @param defaultValue the defaultValue
	 * @return the string or defaultValue
	 */
	static inline std::string optional(ContextPtr ctx, int index, std::string defaultValue)
	{
		return is(ctx, index) ? get(ctx, index) : defaultValue;
	}

	/**
	 * Push a string.
	 *
	 * @param ctx the context
	 * @param value the value
	 */
	static inline void push(ContextPtr ctx, const std::string &value)
	{
		duk_push_lstring(ctx, value.c_str(), value.length());
	}

	/**
	 * Require a string, throws a JavaScript exception if not a string.
	 *
	 * @param ctx the context
	 * @param index the index
	 * @return the string
	 */
	static inline std::string require(ContextPtr ctx, int index)
	{
		duk_size_t size;
		const char *text = duk_require_lstring(ctx, index, &size);

		return std::string{text, size};
	}
};

/**
 * @class TypeTraits<const char *>
 * @brief Default implementation for const char literals.
 *
 * Provides: get, is, optional, push, require.
 */
template <>
class TypeTraits<const char *> {
public:
	/**
	 * Get a string, return 0 if not a string.
	 *
	 * @param ctx the context
	 * @param index the index
	 * @return the string
	 */
	static inline const char *get(ContextPtr ctx, int index)
	{
		return duk_get_string(ctx, index);
	}

	/**
	 * Check if value is a string.
	 *
	 * @param ctx the context
	 * @param index the index
	 * @return true if string
	 */
	static inline bool is(ContextPtr ctx, int index)
	{
		return duk_is_string(ctx, index);
	}

	/**
	 * Get an integer, return defaultValue if the value is not an integer.
	 *
	 * @param ctx the context
	 * @param index the index
	 * @param defaultValue the defaultValue
	 * @return the integer or defaultValue
	 */
	static inline const char *optional(ContextPtr ctx, int index, const char *defaultValue)
	{
		return is(ctx, index) ? get(ctx, index) : defaultValue;
	}

	/**
	 * Push a string.
	 *
	 * @param ctx the context
	 * @param value the value
	 */
	static inline void push(ContextPtr ctx, const char *value)
	{
		duk_push_string(ctx, value);
	}

	/**
	 * Require a string, throws a JavaScript exception if not a string.
	 *
	 * @param ctx the context
	 * @param index the index
	 * @return the string
	 */
	static inline const char *require(ContextPtr ctx, int index)
	{
		return duk_require_string(ctx, index);
	}
};

/**
 * @brief Implementation for non-managed pointers.
 *
 * Provides: get, is, optional, push, require.
 */
template <typename T>
class TypeTraits<RawPointer<T>> {
public:
	/**
	 * Get a pointer, return nullptr if not a pointer.
	 *
	 * @param ctx the context
	 * @param index the index
	 * @return the pointer
	 */
	static inline T *get(ContextPtr ctx, int index)
	{
		return static_cast<T *>(duk_to_pointer(ctx, index));
	}

	/**
	 * Check if value is a pointer.
	 *
	 * @param ctx the context
	 * @param index the index
	 * @return true if pointer
	 */
	static inline bool is(ContextPtr ctx, int index)
	{
		return duk_is_pointer(ctx, index);
	}

	/**
	 * Get a pointer, return defaultValue if the value is not a pointer.
	 *
	 * @param ctx the context
	 * @param index the index
	 * @param defaultValue the defaultValue
	 * @return the pointer or defaultValue
	 */
	static inline T *optional(ContextPtr ctx, int index, RawPointer<T> defaultValue)
	{
		return is(ctx, index) ? get(ctx, index) : defaultValue.object;
	}

	/**
	 * Push a pointer.
	 *
	 * @param ctx the context
	 * @param value the value
	 */
	static inline void push(ContextPtr ctx, const RawPointer<T> &value)
	{
		duk_push_pointer(ctx, value.object);
	}

	/**
	 * Require a pointer, throws a JavaScript exception if not a pointer.
	 *
	 * @param ctx the context
	 * @param index the index
	 * @return the pointer
	 */
	static inline T *require(ContextPtr ctx, int index)
	{
		return static_cast<T *>(duk_require_pointer(ctx, index));
	}
};

/**
 * @class TypeTraits<Function>
 * @brief Push C++ function to the stack.
 *
 * Provides: push.
 *
 * This implementation push a Duktape/C function that is wrapped as C++ for convenience.
 */
template <>
class TypeTraits<Function> {
public:
	/**
	 * Check if the value at the given index is callable.
	 *
	 * @param ctx the context
	 * @param index the value index
	 * @return true if the value is callable
	 */
	static bool is(ContextPtr ctx, Index index)
	{
		return duk_is_callable(ctx, index);
	}

	/**
	 * Push the C++ function, it is wrapped as Duktape/C function and allocated on the heap by moving the
	 * std::function.
	 *
	 * @param ctx the context
	 * @param fn the function
	 */
	static void push(ContextPtr ctx, Function fn)
	{
		duk_push_c_function(ctx, fn.function, fn.nargs);
	}
};

/**
 * @class TypeTraits<FunctionMap>
 * @brief Put the functions to the object at the top of the stack.
 *
 * Provides: push.
 */
template <>
class TypeTraits<FunctionMap> {
public:
	/**
	 * Push all functions to the object at the top of the stack.
	 *
	 * @param ctx the context
	 * @param map the map of function
	 */
	static inline void push(ContextPtr ctx, const FunctionMap &map)
	{
		StackAssert sa(ctx, 0);

		for (const auto &entry : map) {
			duk_push_c_function(ctx, entry.second.function, entry.second.nargs);
			duk_put_prop_string(ctx, -2, entry.first.c_str());
		}
	}
};

/**
 * @class TypeTraits<Object>
 * @brief Push empty object to the stack.
 *
 * Provides: is, push.
 */
template <>
class TypeTraits<Object> {
public:
	/**
	 * Check if value is an object.
	 *
	 * @param ctx the context
	 * @param index the index
	 * @return true if object
	 */
	static inline bool is(ContextPtr ctx, int index)
	{
		return duk_is_object(ctx, index);
	}

	/**
	 * Create an empty object on the stack.
	 *
	 * @param ctx the context
	 */
	static inline void push(ContextPtr ctx, const Object &)
	{
		duk_push_object(ctx);
	}
};

/**
 * @class TypeTraits<Array>
 * @brief Push empty array to the stack.
 *
 * Provides: is, push.
 */
template <>
class TypeTraits<Array> {
public:
	/**
	 * Check if value is a array.
	 *
	 * @param ctx the context
	 * @param index the index
	 * @return true if array
	 */
	static inline bool is(ContextPtr ctx, int index)
	{
		return duk_is_array(ctx, index);
	}

	/**
	 * Create an empty array on the stack.
	 *
	 * @param ctx the context
	 */
	static inline void push(ContextPtr ctx, const Array &)
	{
		duk_push_array(ctx);
	}
};

/**
 * @class TypeTraits<Undefined>
 * @brief Push undefined value to the stack.
 *
 * Provides: is, push.
 */
template <>
class TypeTraits<Undefined> {
public:
	/**
	 * Check if value is undefined.
	 *
	 * @param ctx the context
	 * @param index the index
	 * @return true if undefined
	 */
	static inline bool is(ContextPtr ctx, int index)
	{
		return duk_is_undefined(ctx, index);
	}

	/**
	 * Push undefined value on the stack.
	 *
	 * @param ctx the context
	 */
	static inline void push(ContextPtr ctx, const Undefined &)
	{
		duk_push_undefined(ctx);
	}
};

/**
 * @class TypeTraits<Null>
 * @brief Push null value to the stack.
 *
 * Provides: is, push.
 */
template <>
class TypeTraits<Null> {
public:
	/**
	 * Check if value is null.
	 *
	 * @param ctx the context
	 * @param index the index
	 * @return true if null
	 */
	static inline bool is(ContextPtr ctx, int index)
	{
		return duk_is_null(ctx, index);
	}

	/**
	 * Push null value on the stack.
	 *
	 * @param ctx the context
	 */
	static inline void push(ContextPtr ctx, const Null &)
	{
		duk_push_null(ctx);
	}
};

/**
 * @brief Push this binding into the stack.
 *
 * Provides: push.
 */
template <>
class TypeTraits<This> {
public:
	/**
	 * Push this function into the stack.
	 *
	 * @param ctx the context
	 */
	static inline void push(ContextPtr ctx, const This &)
	{
		duk_push_this(ctx);
	}
};

/**
 * @class TypeTraits<Global>
 * @brief Push the global object to the stack.
 *
 * Provides: push.
 */
template <>
class TypeTraits<Global> {
public:
	/**
	 * Push the global object into the stack.
	 *
	 * @param ctx the context
	 */
	static inline void push(ContextPtr ctx, const Global &)
	{
		duk_push_global_object(ctx);
	}
};

/**
 * @brief Push a map of key-value pair as objects.
 *
 * Provides: push.
 *
 * This class is convenient for settings constants such as enums, string and such.
 */
template <typename T>
class TypeTraits<std::unordered_map<std::string, T>> {
public:
	/**
	 * Put all values from the map as properties to the object at the top of the stack.
	 *
	 * @param ctx the context
	 * @param map the values
	 * @note You need an object at the top of the stack before calling this function
	 */
	static void push(ContextPtr ctx, const std::unordered_map<std::string, T> &map)
	{
		StackAssert sa(ctx, 0);

		for (const auto &pair : map) {
			TypeTraits<T>::push(ctx, pair.second);
			duk_put_prop_string(ctx, -2, pair.first.c_str());
		}
	}
};

/**
 * @brief Push or get vectors as JavaScript arrays.
 *
 * Provides: get, push.
 */
template <typename T>
class TypeTraits<std::vector<T>> {
public:
	/**
	 * Get an array from the stack.
	 *
	 * @param ctx the context
	 * @param index the array index
	 * @return the array or empty array if the value is not an array
	 */
	static std::vector<T> get(ContextPtr ctx, int index)
	{
		StackAssert sa(ctx, 0);

		std::vector<T> result;

		if (!duk_is_array(ctx, -1))
			return result;

		int total = duk_get_length(ctx, index);

		for (int i = 0; i < total; ++i)
			result.push_back(getProperty<T>(ctx, index, i));

		return result;
	}

	/**
	 * Create an array with the specified values.
	 *
	 * @param ctx the context
	 * @param array the values
	 */
	static void push(ContextPtr ctx, const std::vector<T> &array)
	{
		StackAssert sa(ctx, 1);

		duk_push_array(ctx);

		unsigned i = 0;
		for (const auto &v : array) {
			TypeTraits<T>::push(ctx, v);
			duk_put_prop_index(ctx, -2, i++);
		}
	}
};

/**
 * @brief Implementation of managed shared_ptr
 * @see Shared
 */
template <typename T>
class TypeTraits<Shared<T>> {
private:
	static void apply(ContextPtr ctx, std::shared_ptr<T> value)
	{
		StackAssert sa(ctx, 0);

		sign<T>(ctx, -1);

		duk_push_pointer(ctx, new std::shared_ptr<T>(std::move(value)));
		duk_put_prop_string(ctx, -2, "\xff""\xff""js-shared-ptr");
		duk_push_c_function(ctx, [] (duk_context *ctx) -> Ret {
			duk_get_prop_string(ctx, 0, "\xff""\xff""js-shared-ptr");
			delete static_cast<std::shared_ptr<T> *>(duk_to_pointer(ctx, -1));
			duk_pop(ctx);
			duk_push_null(ctx);
			duk_put_prop_string(ctx, 0, "\xff""\xff""js-ptr");

			return 0;
		}, 1);
		duk_set_finalizer(ctx, -2);
	}

public:
	/**
	 * Construct the shared_ptr as this.
	 *
	 * @param ctx the context
	 * @param value the value
	 */
	static void construct(ContextPtr ctx, Shared<T> value)
	{
		StackAssert sa(ctx, 0);

		duk_push_this(ctx);
		apply(ctx, std::move(value.object));
		duk_pop(ctx);
	}

	/**
	 * Push a managed shared_ptr as object.
	 *
	 * @param ctx the context
	 * @param value the value
	 */
	static void push(ContextPtr ctx, Shared<T> value)
	{
		StackAssert sa(ctx, 1);

		duk_push_object(ctx);
		apply(ctx, value.object);
		TypeTraits<T>::prototype(ctx);
		duk_set_prototype(ctx, -2);
	}

	/**
	 * Get a managed shared_ptr from the stack.
	 *
	 * @param ctx the context
	 * @param index the object index
	 * @return the shared_ptr
	 */
	static std::shared_ptr<T> get(ContextPtr ctx, int index)
	{
		StackAssert sa(ctx, 0);

		checkSignature<T>(ctx, index);

		duk_get_prop_string(ctx, index, "\xff""\xff""js-shared-ptr");
		std::shared_ptr<T> value = *static_cast<std::shared_ptr<T> *>(duk_to_pointer(ctx, -1));
		duk_pop(ctx);

		return value;
	}
};

/**
 * @brief Implementation of managed pointers
 * @see Pointer
 */
template <typename T>
class TypeTraits<Pointer<T>> {
private:
	static void apply(ContextPtr ctx, T *value)
	{
		StackAssert sa(ctx, 0);

		sign<T>(ctx, -1);

		duk_push_pointer(ctx, value);
		duk_put_prop_string(ctx, -2, "\xff""\xff""js-ptr");
		duk_push_c_function(ctx, [] (duk_context *ctx) -> Ret {
			duk_get_prop_string(ctx, 0, "\xff""\xff""js-ptr");
			delete static_cast<T *>(duk_to_pointer(ctx, -1));
			duk_pop(ctx);
			duk_push_null(ctx);
			duk_put_prop_string(ctx, 0, "\xff""\xff""js-ptr");

			return 0;
		}, 1);
		duk_set_finalizer(ctx, -2);
	}

public:
	/**
	 * Construct the pointer as this.
	 *
	 * @param ctx the context
	 * @param value the value
	 */
	static void construct(ContextPtr ctx, Pointer<T> value)
	{
		StackAssert sa(ctx, 0);

		duk_push_this(ctx);
		apply(ctx, value.object);
		duk_pop(ctx);
	}

	/**
	 * Push a managed pointer as object.
	 *
	 * @param ctx the context
	 * @param value the value
	 */
	static void push(ContextPtr ctx, Pointer<T> value)
	{
		StackAssert sa(ctx, 1);

		duk_push_object(ctx);
		apply(ctx, value.object);
		TypeTraits<T>::prototype(ctx);
		duk_set_prototype(ctx, -2);
	}

	/**
	 * Get a managed pointer from the stack.
	 *
	 * @param ctx the context
	 * @param index the object index
	 * @return the pointer
	 * @warning Do not store the pointer into the C++ side, the object can be deleted at any time
	 */
	static T *get(ContextPtr ctx, int index)
	{
		StackAssert sa(ctx, 0);

		checkSignature<T>(ctx, index);

		duk_get_prop_string(ctx, index, "\xff""\xff""js-ptr");
		T *value = static_cast<T *>(duk_to_pointer(ctx, -1));
		duk_pop(ctx);

		return value;
	}
};

} // !duk

} // !irccd

#endif // !JS_H
