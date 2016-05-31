/*
 * js.hpp -- JavaScript C++14 wrapper for Duktape
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

#ifndef IRCCD_JS_HPP
#define IRCCD_JS_HPP

/**
 * \file js.hpp
 * \brief Bring JavaScript using Duktape.
 * \author David Demelier <markand@malikania.fr>
 *
 * This file provides usual Duktape function renamed and placed into `duk` namespace. It also replaces error
 * code with exceptions when possible.
 *
 * For convenience, this file also provides templated functions, overloads and much more.
 */

/**
 * \page js JavaScript binding
 * \brief JavaScript binding using Duktape
 *
 * This page will show you how to setup this module to host JavaScript environment into your C++ application.
 *
 * ## Duktape
 *
 * Duktape is a C library designed for performance, small footprint and conformance. This wrapper is built top of it and
 * requires it at build and runtime.
 *
 * It is highly recommended that you read the [Duktape guide](http://duktape.org/guide.html) before continuing because
 * a lot of concepts are kept as-is.
 *
 * ## Installation
 *
 * You need the Duktape source amalgamation, it is provided with this module.
 *
 * When compiling, be sure to enable `-DDUK_OPT_CPP_EXCEPTIONS` and that Duktape source file has **cpp** extension.
 *
 * Just copy the **js.hpp** file and include it into your project. The header depends on **duktape.h** so be sure to
 * provide it when compiling.
 *
 *   - \subpage js-init
 *   - \subpage js-types
 *   - \subpage js-basics
 *   - \subpage js-more
 */

/**
 * \page js-init Initialization
 * \brief Context initialization.
 *
 * To host JavaScript, you need a context. Usually one is sufficient but you can create as many as you want but they
 * won't share any resource.
 *
 * \code
 * #include "js.hpp"
 *
 * int main()
 * {
 *   duk::UniqueContext ctx;
 *
 *   return 0;
 * }
 * \endcode
 *
 * The duk::UniqueContext class is a RAII based wrapper around the native duk_context structure. It is automatically created and closed
 * in the destructor.
 *
 * Be sure to not keep any pointer to it.
 */

/**
 * \page js-types Predefined types
 * \brief Default duk::TypeTraits specializations
 *
 * The following specializations are provided with libjs.
 *
 * ## Primitive types
 *
 * | Type           | Support                          | Remarks                               |
 * |----------------|----------------------------------|---------------------------------------|
 * | `int`          | get, is, optional, push, require |                                       |
 * | `bool`         | get, is, optional, push, require |                                       |
 * | `double`       | get, is, optional, push, require |                                       |
 * | `std::string`  | get, is, optional, push, require | can contain '\0' and binary data      |
 * | `const char *` | get, is, optional, push, require |                                       |
 * | `unsigned`     | get, is, optional, push, require |                                       |
 * | T *            | get, is, optional, push, require | raw pointer, never deleted by Duktape |
 *
 * ## Special JavaScript types
 *
 * The following types are used to create or inspect JavaScript types.
 *
 * | Type             | Support  | Remarks                                |
 * |------------------|----------|----------------------------------------|
 * | duk::Array       | is, push |                                        |
 * | duk::Function    | is, push | is just check if the value is callable |
 * | duk::FunctionMap | put      |                                        |
 * | duk::Global      | push     |                                        |
 * | duk::Null        | is, push |                                        |
 * | duk::Object      | is, push |                                        |
 * | duk::This        | push     |                                        |
 * | duk::Undefined   | is, push |                                        |
 *
 * ## Partial specializations
 *
 * These partial specializations will use complete specialization for T.
 *
 * | Type                               | Support   | Remarks                |
 * |------------------------------------|-----------|------------------------|
 * | std::unordered_map<std::string, T> | push, put | push or put properties |
 * | std::vector<T>                     | push, put | push array of values   |
 */

/**
 * \page js-basics Basics
 * \brief Basics use case.
 *
 * The following topics are sample use case of the C++ front end. It does not use extended features that this module
 * provides.
 *
 *   - \subpage js-basics-t1
 *   - \subpage js-basics-t2
 *   - \subpage js-basics-t3
 */

/**
 * \page js-basics-t1 Example 1: call JavaScript code from C++
 * \brief Evaluate JavaScript code from C++.
 *
 * Let use JavaScript to compute a simple expression.
 *
 * \code
 * #include "js.hpp"
 *
 * int main()
 * {
 *   duk::UniqueContext ctx;
 *
 *   duk::pevalString(ctx, "1 + 1");
 *   std::cout << duk::get<int>(ctx -1) << std::endl;
 *
 *   return 0;
 * }
 * \endcode
 */

/**
 * \page js-basics-t2 Example 2: call C++ code from JavaScript
 * \brief Evaluate a function from JavaScript.
 *
 * In that example, we will add a C++ function to JavaScript and call it from the script. The function just compute the
 * two arguments that are passed through the function and return the result.
 *
 * We take the benefits of C++11 to map the function. The lambda can not have a capture because Duktape use raw C
 * function pointers and this module keep them.
 *
 * \code
 * #include "js.hpp"
 *
 * int main()
 * {
 *   duk::UniqueContext ctx;
 *
 *   // Push a function as global "add"
 *   duk::putGlobal(ctx, "add", duk::Function{[] (duk::Context *ctx) -> duk::Ret {
 *     int x = duk::require<int>(ctx, 0);
 *     int y = duk::require<int>(ctx, 1);
 *
 *     duk::push(ctx, x + y);
 *
 *     return 1;
 *   }, 2});
 *
 *   // Evaluate from JavaScript
 *   duk::pevalString(ctx, "add(1, 2)");
 *
 *   return 0;
 * }
 * \endcode
 *
 * Please note the **2** at end of lambda which indicates that the function takes 2 arguments. If number of arguments
 * is variadic, pass DUK_VARARGS there.
 */

/**
 * \page js-basics-t3 Example 3: pushing and getting values
 * \brief Manage values between JavaScript and C++.
 *
 * As you have seen in the previous examples, we were pushing some values to the Duktape context.
 *
 * With the C Duktape frontend, you usually use duk_push_int, duk_push_string and all other related functions. The libjs
 * module provides an uniform and convenient way for sharing values with the same functions.
 *
 * See the description of duk::TypeTraits to see the supported types.
 *
 * ## Push
 *
 * The duk::push function is a template that accept most of the primitives types. It uses the specializations of the
 * duk::TypeTraits class (described later).
 *
 * Example of code
 *
 * \code
 * duk::push(ctx, 123); // an integer
 * duk::push(ctx, true); // a boolean
 * \endcode
 *
 * The push function add a new value to the stack for any specialization of TypeTraits provided by libjs.
 *
 * ## Get
 *
 * The duk::get function is similar to duk_get_ functions. It usually does not check the value and return a sane default
 * value.
 *
 * This template function does not take the template argument so it can't be deduced and must be specified explicitly.
 *
 * \code
 * duk::get<int>(ctx, 0) // get an int at index 0
 * duk::get<std::string>(ctx, 1) // get a std::string at index 1
 * \endcode
 *
 * ## Require
 *
 * The duk::require function is similar to duk_require functions. It requires the exact type at the given index. If the
 * value is invalid a JavaScript error is propagated.
 *
 * \code
 * duk::require<int>(ctx, 0) // require an int at index 0 or raise an error
 * \endcode
 *
 * ## Put
 *
 * This special function is similar to push except that it applies to the existing object at the top of the stack. It
 * is usually implemented for map and vector.
 *
 * \code
 * // Fill the object at the top of the stack with this map
 * std:unordered_map<std::string, int> map{
 *   { "value1", 1 },
 *   { "value2", 2 }
 * };
 *
 * duk::put(ctx, map);
 * \endcode
 *
 * ## Is
 *
 * This function duk::is checks if the value at the given index is of the given type and return true.
 *
 * Just like duk::get, this function need the explicit template parameter.
 *
 * \code
 * duk::push(ctx, 1);
 * duk::is<int>(ctx, -1); // true
 * \endcode
 *
 * ## Optional
 *
 * The duk::optional function has no equivalent in Duktape C API. It is a convenient way to get values with a default
 * replacement is not available.
 *
 * The common implementation uses duk::is and then duk::get.
 *
 * \code
 * duk::optional<int>(ctx, -1, 123); // 123 is -1 has no value or is not an int
 * \endcode
 */

/**
 * \page js-more Extensions and advanced features.
 * \brief Evolved extensions provided by libjs.
 *
 * The following topics are provided by libjs and have no equivalent in Duktape C API.
 *
 * \subpage js-more-t1
 */

/**
 * \page js-more-t1 Advanced 1: adding your own types to TypeTraits
 * \brief How to specialize duk::TypeTraits structure.
 *
 * This topic shows how you can specialize the duk::TypeTraits structure to add more types.
 *
 * Specializing the duk::TypeTraits is usually only needed when you want to convert a C++ object into JavaScript.
 *
 * In this example we will convert a C++ small structure containing two integers to JavaScript.
 *
 * \note It is not required to implement all functions from duk::TypeTraits. Just provide which one you need.
 *
 * ## The C++ structure
 *
 * The point does not have any methods, it just a description of two integers.
 *
 * \code
 * struct Point {
 *   int x;
 *   int y;
 * };
 * \endcode
 *
 * ## The push function
 *
 * Let's add a push function that will create a JavaScript object with **x** and **y** properties.
 *
 * \code
 * namespace duk {
 *
 * template <>
 * class TypeTraits<Point> {
 * public:
 *   static void push(Context *ctx, const Point &point)
 *   {
 *     // Create an object
 *     push(ctx, Object());
 *
 *     // Set x
 *     putProperty(ctx, -1, "x", point.x);
 *
 *     // Set y
 *     putProperty(ctx, -1, "y", point.y);
 *   }
 * };
 *
 * }
 * \endcode
 *
 * You can safely use different type of reference as second argument.
 *
 * That's it, you can now safely invoke `duk::push(ctx, Point{100, 200});`.
 *
 * ## The get function
 *
 * The get function usually return a new object. The recommandation is to provide sane defaults except if you have any
 * reason to not do so.
 *
 * \code
 * namespace duk {
 *
 * template <>
 * class TypeTraits<Point> {
 * public:
 *   static Point get(Context *ctx, Index index)
 *   {
 *     Point point{0, 0};
 *
 *     if (is<Object>(ctx, index)) {
 *       point.x = getProperty<int>(ctx, index, "x");
 *       point.y = getProperty<int>(ctx, index, "y");
 *     }
 *
 *     return point;
 *   }
 * };
 *
 * }
 * \endcode
 *
 * Now you can invoke `duk::get<Point>(ctx, 0)` to convert a JavaScript object to the Point structure.
 *
 * ## The require function
 *
 * The require function has the same signature as get. It's up to you to decide which criterias makes the object not
 * suitable for conversion.
 *
 * In that example, we will require that object at the index is a JavaScript object and that **x**, **y** are present.
 *
 * \code
 * namespace duk {
 *
 * template <>
 * class TypeTraits<Point> {
 * public:
 *   static Point require(Context *ctx, Index index)
 *   {
 *     Point point;
 *
 *     // Raise an error if not object
 *     if (!is<Object>(ctx, index))
 *       duk::raise(ctx, TypeError("object required"));
 *
 *     // Get x, for simplicity we just check that x and y exist.
 *     if (!hasProperty(ctx, index, "x"))
 *       duk::raise(ctx, TypeError("property x missing"));
 *
 *     // Get y
 *     if (!hasProperty(ctx, index, "y"))
 *       duk::raise(ctx, TypeError("property y missing"));
 *
 *     // Note: if you need more security, check that types are integers too.
 *     point.x = duk::getProperty<int>(ctx, index, "x");
 *     point.y = duk::getProperty<int>(ctx, index, "y");
 *
 *     return point;
 *   }
 * };
 *
 * }
 * \endcode
 *
 * ## The is function
 *
 * The is function returns a boolean. Again, you decide when the value is appropriate.
 *
 * \code
 *
 * namespace duk {
 *
 * template <>
 * class TypeTraits<Point> {
 * public:
 *   static bool is(Context *ctx, Index index)
 *   {
 *     return is<Object>(ctx, index) && hasProperty(ctx, index, "x") && hasProperty(ctx, index, "y");
 *   }
 * };
 *
 * }
 *
 * \endcode
 *
 * ## The optional function
 *
 * The optional function is like get, you should return a value when it is appropriate for conversion. The
 * recommandation is to return the default value **only if** there is no value at the given index or it it not
 * the correct type.
 *
 * Usual implementation looks like this:
 *
 * \code
 * namespace duk {
 *
 * template <>
 * class TypeTraits<Point> {
 * public:
 *   static Point optional(Context *ctx, Index index, Point def)
 *   {
 *     return is(ctx, index) ? get(ctx, index) : def;
 *   }
 * };
 *
 * }
 * \endcode
 */

#include <cassert>
#include <cstdio>
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

/**
 * \brief Typedef without pointer.
 */
using Context = ::duk_context;

/**
 * \brief Typedef for duk_double_t.
 */
using Double = ::duk_double_t;

/**
 * \brief Typedef for duk_idx_t.
 */
using Index = ::duk_idx_t;

/**
 * \brief Typedef for duk_ret_t.
 */
using Ret = ::duk_ret_t;

/**
 * \brief Typedef for duk_size_t.
 */
using Size = ::duk_size_t;

/**
 * \brief Typedef for duk_int_t.
 */
using Int = ::duk_int_t;

/**
 * \brief Typedef for duk_uint_t;
 */
using Uint = ::duk_uint_t;

/**
 * \class StackAssert
 * \brief Stack sanity checker.
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
	Context *m_context;
	unsigned m_expected;
	unsigned m_begin;
#endif

public:
	/**
	 * Create the stack checker.
	 *
	 * No-op if NDEBUG is set.
	 *
	 * \param ctx the context
	 * \param expected the size expected relative to the already existing values
	 */
	inline StackAssert(Context *ctx, unsigned expected = 0) noexcept
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
		if (static_cast<unsigned>(duk_get_top(m_context)) - m_begin != m_expected) {
			std::fprintf(stderr, "Corrupt stack detection in StackAssert:\n");
			std::fprintf(stderr, "  Size at start:            %u\n", m_begin);
			std::fprintf(stderr, "  Size at end:              %d\n", duk_get_top(m_context));
			std::fprintf(stderr, "  Expected (user):          %u\n", m_expected);
			std::fprintf(stderr, "  Expected (adjusted):      %u\n", m_expected + m_begin);
			std::fprintf(stderr, "  Number of stale values:   %u\n", duk_get_top(m_context) - m_begin - m_expected);
			std::abort();
		}
#endif
	}
};

/**
 * \class TypeTraits
 * \brief Type information to implement new types in JavaScript's context.
 *
 * %This class depending on your needs may have the following functions:
 *
 * ## Construct
 *
 * Used by duk::construct, the function must place the value as this binding when the Duktape C function is
 * new-constructed.
 *
 * \code
 * static void construct(Context *ctx, Type value);
 * \endcode
 *
 * ## Get
 *
 * Convert the value at the given index and return it. Should return default object if value is invalid.
 *
 * \code
 * static Type get(Context *ctx, int index);
 * \endcode
 *
 * ## Is
 *
 * Tells if the value at given index is of the requested type.
 *
 * \code
 * static bool is(Context *ctx, int index);
 * \endcode
 *
 * ## Optional
 *
 * Get the value at the given index or return the defaultValue.
 *
 * \code
 * static Type optional(Context *ctx, int index, Type defaultValue);
 * \endcode
 *
 * ## Push
 *
 * Push the value into the stack.
 *
 * \code
 * static void push(Context *ctx, Type value);
 * \endcode
 *
 * ## Put
 *
 * Apply the value to the object at the top of the stack.
 *
 * \code
 * static void put(Context *ctx, Type value);
 * \endcode
 *
 * ## Require
 *
 * Require a value at the given index.
 *
 * \code
 * static Type require(Context *ctx, int index);
 * \endcode
 *
 */
template <typename Type>
class TypeTraits;

/**
 * \class Object
 * \brief Special type for duk::TypeTraits.
 */
class Object {
};

/**
 * \class Array
 * \brief Special type for duk::TypeTraits.
 */
class Array {
};

/**
 * \class Global
 * \brief Special type for duk::TypeTraits.
 */
class Global {
};

/**
 * \class Undefined
 * \brief Special type for duk::TypeTraits.
 */
class Undefined {
};

/**
 * \class Null
 * \brief Special type for duk::TypeTraits.
 */
class Null {
};

/**
 * \class This
 * \brief Special type for duk::TypeTraits.
 */
class This {
};

/**
 * \class Function
 * \brief Duktape/C function definition.
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

	/**
	 * Constructor for compatibility.
	 *
	 * \param f the function
	 * \param n the number of arguments
	 */
	inline Function(duk_c_function f, duk_idx_t n = 0) noexcept
		: function(f)
		, nargs(n)
	{
	}
};

/**
 * \brief Map of functions.
 */
using FunctionMap = std::unordered_map<std::string, Function>;

/**
 * \brief Map of any type.
 */
template <typename T>
using Map = std::unordered_map<std::string, T>;

/**
 * \class Exception
 * \brief Error description.
 *
 * This class fills the fields got in an Error object.
 */
class Exception : public std::exception {
public:
	std::string name;		//!< name of error
	std::string message;		//!< error message
	std::string stack;		//!< stack if available
	std::string fileName;		//!< filename if applicable
	int lineNumber{0};		//!< line number if applicable

	/**
	 * Get the error message. This effectively returns message field.
	 *
	 * \return the message
	 */
	const char *what() const noexcept override
	{
		return message.c_str();
	}
};

/**
 * \brief RAII based Duktape handler.
 *
 * This class is implicitly convertible to duk_context for convenience.
 */
class UniqueContext {
private:
	using Deleter = void (*)(duk_context *);
	using Handle = std::unique_ptr<duk_context, Deleter>;

	Handle m_handle;

	UniqueContext(const UniqueContext &) = delete;
	UniqueContext &operator=(const UniqueContext &) = delete;

public:
	/**
	 * Create default context.
	 */
	inline UniqueContext()
		: m_handle(duk_create_heap_default(), duk_destroy_heap)
	{
	}

	/**
	 * Default move constructor.
	 */
	UniqueContext(UniqueContext &&) noexcept = default;

	/**
	 * Convert the context to the native Duktape/C type.
	 *
	 * \return the duk_context
	 */
	inline operator duk_context *() noexcept
	{
		return m_handle.get();
	}

	/**
	 * Convert the context to the native Duktape/C type.
	 *
	 * \return the duk_context
	 */
	inline operator duk_context *() const noexcept
	{
		return m_handle.get();
	}

	/**
	 * Default move assignment operator.
	 *
	 * \return this
	 */
	UniqueContext &operator=(Context &&) noexcept = delete;
};

/**
 * \name Duktape C functions
 * \brief The following functions are wrappers on top of the Duktape C functions.
 *
 * They are as close as possible to the original functions.
 */

/**
 * \{
 */

/**
 * Wrapper for [duk_allow](http://duktape.org/api.html#duk_allow).
 *
 * \param ctx the context
 * \param size the requested size
 * \return the pointer
 */
inline void *alloc(Context *ctx, Size size)
{
	return duk_alloc(ctx, size);
}

/**
 * Wrapper for [duk_allow_raw](http://duktape.org/api.html#duk_allow_raw).
 *
 * \param ctx the context
 * \param size the requested size
 * \return the pointer
 */
inline void *allocRaw(Context *ctx, Size size)
{
	return duk_alloc_raw(ctx, size);
}

/**
 * Wrapper for [duk_base64_decode](http://duktape.org/api.html#duk_base64_decode).
 *
 * \param ctx the context
 * \param index the index
 */
inline void base64Decode(Context *ctx, Index index)
{
	duk_base64_decode(ctx, index);
}

/**
 * Wrapper for [duk_base64_encode](http://duktape.org/api.html#duk_base64_encode).
 *
 * \param ctx the context
 * \param index the index
 * \return the base64 string
 */
inline std::string base64Encode(Context *ctx, Index index)
{
	return duk_base64_encode(ctx, index);
}

/**
 * Wrapper for [duk_call](http://duktape.org/api.html#duk_call).
 *
 * \param ctx the context
 * \param nargs the number of arguments
 */
inline void call(Context *ctx, Index nargs = 0)
{
	duk_call(ctx, nargs);
}

/**
 * Wrapper for [duk_call_method](http://duktape.org/api.html#duk_call_method).
 *
 * \param ctx the context
 * \param nargs the number of arguments
 */
inline void callMethod(Context *ctx, Index nargs = 0)
{
	duk_call_method(ctx, nargs);
}

/**
 * Wrapper for [duk_call_prop](http://duktape.org/api.html#duk_call_prop).
 *
 * \param ctx the context
 * \param index the object index
 * \param nargs the number of arguments
 */
inline void callProperty(Context *ctx, Index index, Index nargs = 0)
{
	duk_call_prop(ctx, index, nargs);
}

/**
 * Wrapper for [duk_char_code_at](http://duktape.org/api.html#duk_char_code_at).
 *
 * \param ctx the context
 * \param index the index
 * \param charOffset the offset
 * \return the code point
 */
inline duk_codepoint_t charCodeAt(Context *ctx, Index index, duk_size_t charOffset)
{
	return duk_char_code_at(ctx, index, charOffset);
}

/**
 * Wrapper for [duk_check_stack](http://duktape.org/api.html#duk_check_stack).
 *
 * \param ctx the context
 * \param extra the extra space
 * \return true if space is available
 */
inline bool checkStack(Context *ctx, Index extra)
{
	return duk_check_stack(ctx, extra) != 0;
}

/**
 * Wrapper for [duk_check_stack_top](http://duktape.org/api.html#duk_check_stack_top).
 *
 * \param ctx the context
 * \param top the extra space
 * \return true if space is available
 */
inline bool checkStackTop(Context *ctx, Index top)
{
	return duk_check_stack_top(ctx, top) != 0;
}

/**
 * Wrapper for [duk_check_type](http://duktape.org/api.html#duk_check_type).
 *
 * \param ctx the context
 * \param index the value index
 * \param type the desired type
 * \return true if object is given type
 */
inline bool checkType(Context *ctx, Index index, int type)
{
	return duk_check_type(ctx, index, type) != 0;
}

/**
 * Wrapper for [duk_check_type_mask](http://duktape.org/api.html#duk_check_type_mask).
 *
 * \param ctx the context
 * \param index the value index
 * \param mask the desired mask
 * \return true if object is one of the type
 */
inline bool checkTypeMask(Context *ctx, Index index, unsigned mask)
{
	return duk_check_type_mask(ctx, index, mask) != 0;
}

/**
 * Wrapper for [duk_compact](http://duktape.org/api.html#duk_compact).
 *
 * \param ctx the context
 * \param objIndex the object index
 */
inline void compact(Context *ctx, Index objIndex)
{
	duk_compact(ctx, objIndex);
}

/**
 * Wrapper for [duk_concat](http://duktape.org/api.html#duk_concat).
 *
 * \param ctx the context
 * \param count the number of values
 */
inline void concat(Context *ctx, Index count)
{
	duk_concat(ctx, count);
}

/**
 * Wrapper for [duk_copy](http://duktape.org/api.html#duk_copy).
 *
 * \param ctx the context
 * \param from the from index
 * \param to the destination
 */
inline void copy(Context *ctx, Index from, Index to)
{
	duk_copy(ctx, from, to);
}

/**
 * Wrapper for [duk_new](http://duktape.org/api.html#duk_new).
 *
 * \param ctx the context
 * \param nargs the number of arguments
 */
inline void create(Context *ctx, int nargs = 0)
{
	duk_new(ctx, nargs);
}

/**
 * Wrapper for [duk_def_prop](http://duktape.org/api.html#duk_def_prop).
 *
 * \param ctx the context
 * \param index the object index
 * \param flags the flags
 */
inline void defineProperty(Context *ctx, Index index, unsigned flags)
{
	duk_def_prop(ctx, index, flags);
}

/**
 * Wrapper for [duk_del_prop](http://duktape.org/api.html#duk_del_prop).
 *
 * \param ctx the context
 * \param index the object index
 * \return true if deleted
 */
inline bool deleteProperty(Context *ctx, Index index)
{
	return duk_del_prop(ctx, index) != 0;
}

/**
 * Wrapper for [duk_del_prop](http://duktape.org/api.html#duk_del_prop).
 *
 * \param ctx the context
 * \param index the object index
 * \param position the property index
 * \return true if deleted
 */
inline bool deleteProperty(Context *ctx, Index index, unsigned position)
{
	return duk_del_prop_index(ctx, index, position) != 0;
}

/**
 * Wrapper for [duk_del_prop](http://duktape.org/api.html#duk_del_prop).
 *
 * \param ctx the context
 * \param index the object index
 * \param name the property name
 * \return true if deleted
 */
inline bool deleteProperty(Context *ctx, Index index, const std::string &name)
{
	return duk_del_prop_string(ctx, index, name.c_str()) != 0;
}

/**
 * Wrapper for [duk_dup](http://duktape.org/api.html#duk_dup).
 *
 * \param ctx the context
 * \param index the value to copy
 */
inline void dup(Context *ctx, int index = -1)
{
	duk_dup(ctx, index);
}

/**
 * Wrapper for [duk_equals](http://duktape.org/api.html#duk_equals).
 *
 * \param ctx the context
 * \param index1 the first value
 * \param index2 the second value
 * \return true if they equal
 */
inline bool equals(Context *ctx, Index index1, Index index2)
{
	return duk_equals(ctx, index1, index2) != 0;
}

/**
 * Wrapper for [duk_eval](http://duktape.org/api.html#duk_eval).
 *
 * \param ctx the context
 */
inline void eval(Context *ctx)
{
	duk_eval(ctx);
}

/**
 * Wrapper for [duk_eval_file](http://duktape.org/api.html#duk_eval_file).
 *
 * \param ctx the context
 * \param path the path
 * \param result true to get the result at the top of the stack
 */
inline void evalFile(Context *ctx, const std::string &path, bool result = true)
{
	return result ? duk_eval_file(ctx, path.c_str()) : duk_eval_file_noresult(ctx, path.c_str());
}

/**
 * Wrapper for [duk_eval_string](http://duktape.org/api.html#duk_eval_string).
 *
 * \param ctx the context
 * \param src the source script
 * \param result true to get the result at the top of the stack
 */
inline void evalString(Context *ctx, const std::string &src, bool result = true)
{
	return result ? duk_eval_string(ctx, src.c_str()) : duk_eval_string_noresult(ctx, src.c_str());
}
/**
 * Wrapper for [duk_gc](http://duktape.org/api.html#duk_gc).
 *
 * \param ctx the context
 * \param flags the flags
 */
inline void gc(Context *ctx, unsigned flags = 0)
{
	duk_gc(ctx, flags);
}

/**
 * Wrapper for [duk_has_prop](http://duktape.org/api.html#duk_has_prop).
 *
 * \param ctx the context
 * \param index the object index
 * \return true if has
 */
inline bool hasProperty(Context *ctx, Index index)
{
	return duk_has_prop(ctx, index) != 0;
}

/**
 * Wrapper for [duk_has_prop](http://duktape.org/api.html#duk_has_prop).
 *
 * \param ctx the context
 * \param index the object index
 * \param position the property index
 * \return true if has
 */
inline bool hasProperty(Context *ctx, Index index, unsigned position)
{
	return duk_has_prop_index(ctx, index, position) != 0;
}

/**
 * Wrapper for [duk_has_prop](http://duktape.org/api.html#duk_has_prop).
 *
 * \param ctx the context
 * \param index the object index
 * \param name the property name
 * \return true if has
 */
inline bool hasProperty(Context *ctx, int index, const std::string &name)
{
	return duk_has_prop_string(ctx, index, name.c_str()) != 0;
}

/**
 * Wrapper for [duk_insert](http://duktape.org/api.html#duk_insert).
 *
 * \param ctx the context
 * \param to the destination
 * \note Wrapper of duk_insert
 */
inline void insert(Context *ctx, Index to)
{
	duk_insert(ctx, to);
}

/**
 * Wrapper for [duk_instanceof](http://duktape.org/api.html#duk_instanceof).
 *
 * \param ctx the context
 * \param idx1 the value to test
 * \param idx2 the instance requested
 * \return true if idx1 is instance of idx2
 */
inline bool instanceof(Context *ctx, Index idx1, Index idx2)
{
	return duk_instanceof(ctx, idx1, idx2) != 0;
}

/**
 * Wrapper for [duk_is_constructor_call](http://duktape.org/api.html#duk_is_constructor_call).
 *
 * \param ctx the context
 * \return true if it's a constructor call (new operator)
 */
inline bool isConstructorCall(Context *ctx)
{
	return duk_is_constructor_call(ctx) != 0;
}

/**
 * Wrapper for [duk_join](http://duktape.org/api.html#duk_join).
 *
 * \param ctx the context
 * \param count the number of values
 */
inline void join(Context *ctx, Index count)
{
	duk_join(ctx, count);
}

/**
 * Wrapper for [duk_json_decode](http://duktape.org/api.html#duk_json_decode).
 *
 * \param ctx the context
 * \param index the index
 */
inline void jsonDecode(Context *ctx, Index index)
{
	duk_json_decode(ctx, index);
}

/**
 * Wrapper for [duk_json_encode](http://duktape.org/api.html#duk_json_encode).
 *
 * \param ctx the context
 * \param index the index
 * \return the JSON string
 */
inline std::string jsonEncode(Context *ctx, Index index)
{
	return duk_json_encode(ctx, index);
}

/**
 * Wrapper for [duk_normalize_index](http://duktape.org/api.html#duk_normalize_index).
 *
 * \param ctx the context
 * \param index the index
 * \return the absolute index
 */
inline Index normalizeIndex(Context *ctx, Index index)
{
	return duk_normalize_index(ctx, index);
}

/**
 * Wrapper for [duk_pcall](http://duktape.org/api.html#duk_pcall).
 *
 * \param ctx the context
 * \param nargs the number of arguments
 * \return non zero on failure
 */
inline int pcall(Context *ctx, Index nargs = 0)
{
	return duk_pcall(ctx, nargs);
}

/**
 * Wrapper for [duk_pcall_method](http://duktape.org/api.html#duk_pcall_method).
 *
 * \param ctx the context
 * \param nargs the number of arguments
 * \return non zero on failure
 */
inline int pcallMethod(Context *ctx, Index nargs = 0)
{
	return duk_pcall_method(ctx, nargs);
}

/**
 * Wrapper for [duk_pcall_prop](http://duktape.org/api.html#duk_pcall_prop).
 *
 * \param ctx the context
 * \param index the object index
 * \param nargs the number of arguments
 * \return non zero on failure
 */
inline int pcallProperty(Context *ctx, Index index, Index nargs = 0)
{
	return duk_pcall_prop(ctx, index, nargs);
}

/**
 * Wrapper for [duk_peval](http://duktape.org/api.html#duk_peval).
 *
 * \param ctx the context
 * \return non zero on failure
 */
inline int peval(Context *ctx)
{
	return duk_peval(ctx);
}

/**
 * Wrapper for [duk_peval_file](http://duktape.org/api.html#duk_peval_file).
 *
 * \param ctx the context
 * \param path the path
 * \param result true to get the result at the top of the stack
 * \return non zero on failure
 */
inline int pevalFile(Context *ctx, const std::string &path, bool result = true)
{
	return result ? duk_peval_file(ctx, path.c_str()) : duk_peval_file_noresult(ctx, path.c_str());
}

/**
 * Wrapper for [duk_peval_string](http://duktape.org/api.html#duk_peval_string).
 *
 * \param ctx the context
 * \param src the source script
 * \param result true to get the result at the top of the stack
 * \return non zero on failure
 */
inline int pevalString(Context *ctx, const std::string &src, bool result = true)
{
	return result ? duk_peval_string(ctx, src.c_str()) : duk_peval_string_noresult(ctx, src.c_str());
}

/**
 * Wrapper for [duk_pop_n](http://duktape.org/api.html#duk_pop_n).
 *
 * \param ctx the context
 * \param count the number of values to pop
 */
inline void pop(Context *ctx, Index count = 1)
{
	duk_pop_n(ctx, count);
}

/**
 * Wrapper for [duk_put_prop](http://duktape.org/api.html#duk_put_prop).
 *
 * \param ctx the context
 * \param index the object index
 */
inline void putProperty(Context *ctx, Index index)
{
	duk_put_prop(ctx, index);
}

/**
 * Wrapper for [duk_put_prop_string](http://duktape.org/api.html#duk_put_prop_string).
 *
 * \param ctx the context
 * \param index the object index
 * \param name the property name
 */
inline void putProperty(Context *ctx, Index index, const std::string &name)
{
	duk_put_prop_string(ctx, index, name.c_str());
}

/**
 * Wrapper for [duk_put_prop_index](http://duktape.org/api.html#duk_put_prop_index).
 *
 * \param ctx the context
 * \param index the object index
 * \param position the array position
 */
inline void putProperty(Context *ctx, Index index, unsigned position)
{
	duk_put_prop_index(ctx, index, position);
}

/**
 * Wrapper for [duk_remove](http://duktape.org/api.html#duk_remove).
 *
 * \param ctx the context
 * \param index the value to remove
 */
inline void remove(Context *ctx, Index index)
{
	duk_remove(ctx, index);
}

/**
 * Wrapper for [duk_replace](http://duktape.org/api.html#duk_replace).
 *
 * \param ctx the context
 * \param index the value to replace by the value at the top of the stack
 */
inline void replace(Context *ctx, Index index)
{
	duk_replace(ctx, index);
}

/**
 * Wrapper for [duk_set_finalizer](http://duktape.org/api.html#duk_set_finalizer).
 *
 * \param ctx the context
 * \param index the value index
 */
inline void setFinalizer(Context *ctx, Index index)
{
	duk_set_finalizer(ctx, index);
}

/**
 * Wrapper for [duk_set_prototype](http://duktape.org/api.html#duk_set_prototype).
 *
 * \param ctx the context
 * \param index the value index
 */
inline void setPrototype(Context *ctx, Index index)
{
	duk_set_prototype(ctx, index);
}

/**
 * Wrapper for [duk_swap](http://duktape.org/api.html#duk_swap).
 *
 * \param ctx the context
 * \param index1 the first index
 * \param index2 the second index
 */
inline void swap(Context *ctx, Index index1, Index index2)
{
	duk_swap(ctx, index1, index2);
}

/**
 * Wrapper for [duk_swap_top](http://duktape.org/api.html#duk_swap_top).
 *
 * \param ctx the context
 * \param index the index
 */
inline void swapTop(Context *ctx, Index index)
{
	duk_swap_top(ctx, index);
}

/**
 * Wrapper for [duk_get_top](http://duktape.org/api.html#duk_get_top).
 *
 * \param ctx the context
 * \return the stack size
 */
inline int top(Context *ctx)
{
	return duk_get_top(ctx);
}

/**
 * Wrapper for [duk_throw](http://duktape.org/api.html#duk_throw).
 *
 * \param ctx the context
 */
inline void raise(Context *ctx)
{
	duk_throw(ctx);
}

/**
 *Wrapper for [duk_error](http://duktape.org/api.html#duk_error).
 *
 * \param ctx the context
 * \param type the error type (e.g. DUK_ERR_REFERENCE_ERROR)
 * \param fmt the format string
 * \param args the arguments
 */
template <typename... Args>
inline void raise(Context *ctx, int type, const char *fmt, Args&&... args)
{
	duk_error(ctx, type, fmt, std::forward<Args>(args)...);
}

/**
 * Wrapper for [duk_get_type](http://duktape.org/api.html#duk_get_type).
 *
 * \param ctx the context
 * \param index the index
 * \return the type
 */
inline int type(Context *ctx, Index index)
{
	return duk_get_type(ctx, index);
}

/**
 * \}
 */

/**
 * \name Extended functions
 * \brief Extended functions for libjs.
 *
 * The following functions are largely modified or extensions to Duktape.
 */

/**
 * \{
 */

/**
 * Get the error object when a JavaScript error has been thrown (e.g. eval failure).
 *
 * \param ctx the context
 * \param index the index
 * \param pop if true, also remove the exception from the stack
 * \return the information
 */
inline Exception exception(Context *ctx, int index, bool pop = true)
{
	Exception ex;

	index = duk_normalize_index(ctx, index);

	duk_get_prop_string(ctx, index, "name");
	ex.name = duk_to_string(ctx, -1);
	duk_get_prop_string(ctx, index, "message");
	ex.message = duk_to_string(ctx, -1);
	duk_get_prop_string(ctx, index, "fileName");
	ex.fileName = duk_to_string(ctx, -1);
	duk_get_prop_string(ctx, index, "lineNumber");
	ex.lineNumber = duk_to_int(ctx, -1);
	duk_get_prop_string(ctx, index, "stack");
	ex.stack = duk_to_string(ctx, -1);
	duk_pop_n(ctx, 5);

	if (pop)
		duk_remove(ctx, index);

	return ex;
}

/**
 * Push a value into the stack. Calls TypeTraits<T>::push(ctx, value);
 *
 * \param ctx the context
 * \param value the value to forward
 */
template <typename Type>
inline void push(Context *ctx, Type &&value)
{
	TypeTraits<std::decay_t<Type>>::push(ctx, std::forward<Type>(value));
}

/**
 * Put the value to the object at the top of the stack. Calls TypeTraits<T>::put(ctx, value);
 *
 * \param ctx the context
 * \param value the value to apply
 */
template <typename Type>
inline void put(Context *ctx, Type &&value)
{
	TypeTraits<std::decay_t<Type>>::put(ctx, std::forward<Type>(value));
}

/**
 * Generic template function to get a value from the stack.
 *
 * \param ctx the context
 * \param index the index
 * \return the value
 */
template <typename Type>
inline auto get(Context *ctx, int index) -> decltype(TypeTraits<Type>::get(ctx, 0))
{
	return TypeTraits<Type>::get(ctx, index);
}

/**
 * Require a type at the specified index.
 *
 * \param ctx the context
 * \param index the index
 * \return the value
 */
template <typename Type>
inline auto require(Context *ctx, int index) -> decltype(TypeTraits<Type>::require(ctx, 0))
{
	return TypeTraits<Type>::require(ctx, index);
}

/**
 * Check if a value is a type of T.
 *
 * The TypeTraits<T> must have `static bool is(ContextPtr ptr, int index)`.
 *
 * \param ctx the context
 * \param index the value index
 * \return true if is the type
 */
template <typename T>
inline bool is(Context *ctx, int index)
{
	return TypeTraits<T>::is(ctx, index);
}

/**
 * Get an optional value from the stack, if the value is not available of not the correct type,
 * return defaultValue instead.
 *
 * The TypeTraits<T> must have `static T optional(Context &, int index, T &&defaultValue)`.
 *
 * \param ctx the context
 * \param index the value index
 * \param defaultValue the value replacement
 * \return the value or defaultValue
 */
template <typename Type>
inline auto optional(Context *ctx, int index, Type &&defaultValue)
{
	return TypeTraits<std::decay_t<Type>>::optional(ctx, index, std::forward<Type>(defaultValue));
}

/**
 * Get the property `name' as value from the object at the specified index.
 *
 * \param ctx the context
 * \param index the object index
 * \param name the property name
 * \return the value
 * \note The stack is unchanged
 */
template <typename Type, typename std::enable_if_t<!std::is_void<Type>::value> * = nullptr>
inline auto getProperty(Context *ctx, int index, const std::string &name) -> decltype(get<Type>(ctx, 0))
{
	duk_get_prop_string(ctx, index, name.c_str());
	decltype(get<Type>(ctx, 0)) value = get<Type>(ctx, -1);
	duk_pop(ctx);

	return value;
}

/**
 * Get a property by index, for arrays.
 *
 * \param ctx the context
 * \param index the object index
 * \param position the position int the object
 * \return the value
 * \note The stack is unchanged
 */
template <typename Type, typename std::enable_if_t<!std::is_void<Type>::value> * = nullptr>
inline auto getProperty(Context *ctx, int index, int position) -> decltype(get<Type>(ctx, 0))
{
	duk_get_prop_index(ctx, index, position);
	decltype(get<Type>(ctx, 0)) value = get<Type>(ctx, -1);
	duk_pop(ctx);

	return value;
}

/**
 * Get the property `name' and push it to the stack from the object at the specified index.
 *
 * \param ctx the context
 * \param index the object index
 * \param name the property name
 * \note The stack contains the property value
 */
template <typename Type, typename std::enable_if_t<std::is_void<Type>::value> * = nullptr>
inline void getProperty(Context *ctx, int index, const std::string &name)
{
	duk_get_prop_string(ctx, index, name.c_str());
}

/**
 * Get the property by index and push it to the stack from the object at the specified index.
 *
 * \param ctx the context
 * \param index the object index
 * \param position the position in the object
 * \note The stack contains the property value
 */
template <typename Type, typename std::enable_if_t<std::is_void<Type>::value> * = nullptr>
inline void getProperty(Context *ctx, int index, int position)
{
	duk_get_prop_index(ctx, index, position);
}

/**
 * Get an optional property `name` from the object at the specified index.
 *
 * \param ctx the context
 * \param index the object index
 * \param name the property name
 * \param def the default value
 * \return the value or def
 * \note The stack is unchanged
 */
template <typename Type, typename DefaultValue>
inline auto optionalProperty(Context *ctx, int index, const std::string &name, DefaultValue &&def) -> decltype(optional(ctx, 0, std::forward<DefaultValue>(def)))
{
	duk_get_prop_string(ctx, index, name.c_str());
	decltype(optional(ctx, 0, std::forward<DefaultValue>(def))) value = optional(ctx, -1, std::forward<DefaultValue>(def));
	duk_pop(ctx);

	return value;
}

/**
 * Get an optional property by index, for arrays
 *
 * \param ctx the context
 * \param index the object index
 * \param position the position int the object
 * \param def the default value
 * \return the value or def
 * \note The stack is unchanged
 */
template <typename Type, typename DefaultValue>
inline auto optionalProperty(Context *ctx, int index, int position, DefaultValue &&def) -> decltype(optional(ctx, 0, std::forward<DefaultValue>(def)))
{
	duk_get_prop_index(ctx, index, position);
	decltype(optional(ctx, 0, std::forward<DefaultValue>(def))) value = optional(ctx, -1, std::forward<DefaultValue>(def));
	duk_pop(ctx);

	return value;
}

/**
 * Set a property to the object at the specified index.
 *
 * \param ctx the context
 * \param index the object index
 * \param name the property name
 * \param value the value to forward
 * \note The stack is unchanged
 */
template <typename Type>
void putProperty(Context *ctx, int index, const std::string &name, Type &&value)
{
	index = duk_normalize_index(ctx, index);

	push(ctx, std::forward<Type>(value));
	duk_put_prop_string(ctx, index, name.c_str());
}

/**
 * Set a property by index, for arrays.
 *
 * \param ctx the context
 * \param index the object index
 * \param position the position in the object
 * \param value the value to forward
 * \note The stack is unchanged
 */
template <typename Type>
void putProperty(Context *ctx, int index, int position, Type &&value)
{
	index = duk_normalize_index(ctx, index);

	push(ctx, std::forward<Type>(value));
	duk_put_prop_index(ctx, index, position);
}

/**
 * Get a global value.
 *
 * \param ctx the context
 * \param name the name of the global variable
 * \return the value
 */
template <typename Type>
inline auto getGlobal(Context *ctx, const std::string &name, std::enable_if_t<!std::is_void<Type>::value> * = nullptr) -> decltype(get<Type>(ctx, 0))
{
	duk_get_global_string(ctx, name.c_str());
	decltype(get<Type>(ctx, 0)) value = get<Type>(ctx, -1);
	duk_pop(ctx);

	return value;
}

/**
 * Overload that push the value at the top of the stack instead of returning it.
 *
 * \param ctx the context
 * \param name the name of the global variable
 */
template <typename Type>
inline void getGlobal(Context *ctx, const std::string &name, std::enable_if_t<std::is_void<Type>::value> * = nullptr)
{
	duk_get_global_string(ctx, name.c_str());
}

/**
 * Set a global variable.
 *
 * \param ctx the context
 * \param name the name of the global variable
 * \param type the value to set
 */
template <typename Type>
inline void putGlobal(Context *ctx, const std::string &name, Type&& type)
{
	push(ctx, std::forward<Type>(type));
	duk_put_global_string(ctx, name.c_str());
}

/**
 * Put the value at the top of the stack as global property.
 *
 * \param ctx the context
 * \param name the property name
 */
inline void putGlobal(Context *ctx, const std::string &name)
{
	duk_put_global_string(ctx, name.c_str());
}

/**
 * Enumerate an object or an array at the specified index.
 *
 * \param ctx the context
 * \param index the object or array index
 * \param flags the optional flags to pass to duk_enum
 * \param getvalue set to true if you want to extract the value
 * \param func the function to call for each properties
 */
template <typename Func>
void enumerate(Context *ctx, int index, duk_uint_t flags, duk_bool_t getvalue, Func &&func)
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
 * \param ctx the context
 * \return the this binding as the template given
 */
template <typename T>
inline auto self(Context *ctx) -> decltype(TypeTraits<T>::require(ctx, 0))
{
	duk_push_this(ctx);
	decltype(TypeTraits<T>::require(ctx, 0)) value = TypeTraits<T>::require(ctx, -1);
	duk_pop(ctx);

	return value;
}

/**
 * Throw an ECMAScript exception.
 *
 * \param ctx the context
 * \param ex the exception
 */
template <typename Exception>
void raise(Context *ctx, const Exception &ex)
{
	ex.raise(ctx);
}

/**
 * Construct the object in place, setting value as this binding.
 *
 * The TypeTraits<T> must have the following requirements:
 *
 * - static void construct(Context &, T): must update this with the value and keep the stack unchanged
 *
 * \param ctx the context
 * \param value the value to forward
 * \see self
 */
template <typename T>
inline void construct(Context *ctx, T &&value)
{
	TypeTraits<std::decay_t<T>>::construct(ctx, std::forward<T>(value));
}

/**
 * \}
 */

/**
 * \class Error
 * \brief Base ECMAScript error class.
 * \warning Override the function create for your own exceptions
 */
class Error {
private:
	int m_type{DUK_ERR_ERROR};
	std::string m_message;

protected:
	/**
	 * Constructor with a type of error specified, specially designed for derived errors.
	 *
	 * \param type of error (e.g. DUK_ERR_ERROR)
	 * \param message the message
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
	 * \param message the message
	 */
	inline Error(std::string message) noexcept
		: m_message(std::move(message))
	{
	}

	/**
	 * Create the exception on the stack.
	 *
	 * \note the default implementation search for the global variables
	 * \param ctx the context
	 */
	virtual void raise(Context *ctx) const
	{
		duk_error(ctx, m_type, "%s", m_message.c_str());
	}
};

/**
 * \class EvalError
 * \brief Error in eval() function.
 */
class EvalError : public Error {
public:
	/**
	 * Construct an EvalError.
	 *
	 * \param message the message
	 */
	inline EvalError(std::string message) noexcept
		: Error(DUK_ERR_EVAL_ERROR, std::move(message))
	{
	}
};

/**
 * \class RangeError
 * \brief Value is out of range.
 */
class RangeError : public Error {
public:
	/**
	 * Construct an RangeError.
	 *
	 * \param message the message
	 */
	inline RangeError(std::string message) noexcept
		: Error(DUK_ERR_RANGE_ERROR, std::move(message))
	{
	}
};

/**
 * \class ReferenceError
 * \brief Trying to use a variable that does not exist.
 */
class ReferenceError : public Error {
public:
	/**
	 * Construct an ReferenceError.
	 *
	 * \param message the message
	 */
	inline ReferenceError(std::string message) noexcept
		: Error(DUK_ERR_REFERENCE_ERROR, std::move(message))
	{
	}
};

/**
 * \class SyntaxError
 * \brief Syntax error in the script.
 */
class SyntaxError : public Error {
public:
	/**
	 * Construct an SyntaxError.
	 *
	 * \param message the message
	 */
	inline SyntaxError(std::string message) noexcept
		: Error(DUK_ERR_SYNTAX_ERROR, std::move(message))
	{
	}
};

/**
 * \class TypeError
 * \brief Invalid type given.
 */
class TypeError : public Error {
public:
	/**
	 * Construct an TypeError.
	 *
	 * \param message the message
	 */
	inline TypeError(std::string message) noexcept
		: Error(DUK_ERR_TYPE_ERROR, std::move(message))
	{
	}
};

/**
 * \class URIError
 * \brief URI manipulation failure.
 */
class URIError : public Error {
public:
	/**
	 * Construct an URIError.
	 *
	 * \param message the message
	 */
	inline URIError(std::string message) noexcept
		: Error(DUK_ERR_URI_ERROR, std::move(message))
	{
	}
};

/**
 * \class TypeTraits<int>
 * \brief Default implementation for int.
 *
 * Provides: get, is, optional, push, require.
 */
template <>
class TypeTraits<int> {
public:
	/**
	 * Get an integer, return 0 if not an integer.
	 *
	 * \param ctx the context
	 * \param index the index
	 * \return the integer
	 */
	static inline int get(Context *ctx, int index)
	{
		return duk_get_int(ctx, index);
	}

	/**
	 * Check if value is an integer.
	 *
	 * \param ctx the context
	 * \param index the index
	 * \return true if integer
	 */
	static inline bool is(Context *ctx, int index)
	{
		return duk_is_number(ctx, index) != 0;
	}

	/**
	 * Get an integer, return defaultValue if the value is not an integer.
	 *
	 * \param ctx the context
	 * \param index the index
	 * \param defaultValue the defaultValue
	 * \return the integer or defaultValue
	 */
	static inline int optional(Context *ctx, int index, int defaultValue)
	{
		return is(ctx, index) ? get(ctx, index) : defaultValue;
	}

	/**
	 * Push an integer.
	 *
	 * \param ctx the context
	 * \param value the value
	 */
	static inline void push(Context *ctx, int value)
	{
		duk_push_int(ctx, value);
	}

	/**
	 * Require an integer, throws a JavaScript exception if not an integer.
	 *
	 * \param ctx the context
	 * \param index the index
	 * \return the integer
	 */
	static inline int require(Context *ctx, int index)
	{
		return duk_require_int(ctx, index);
	}
};

/**
 * \class TypeTraits<bool>
 * \brief Default implementation for bool.
 *
 * Provides: get, is, optional, push, require.
 */
template <>
class TypeTraits<bool> {
public:
	/**
	 * Get a boolean, return 0 if not a boolean.
	 *
	 * \param ctx the context
	 * \param index the index
	 * \return the boolean
	 */
	static inline bool get(Context *ctx, int index)
	{
		return duk_get_boolean(ctx, index) != 0;
	}

	/**
	 * Check if value is a boolean.
	 *
	 * \param ctx the context
	 * \param index the index
	 * \return true if boolean
	 */
	static inline bool is(Context *ctx, int index)
	{
		return duk_is_boolean(ctx, index) != 0;
	}

	/**
	 * Get a bool, return defaultValue if the value is not a boolean.
	 *
	 * \param ctx the context
	 * \param index the index
	 * \param defaultValue the defaultValue
	 * \return the boolean or defaultValue
	 */
	static inline bool optional(Context *ctx, int index, bool defaultValue)
	{
		return is(ctx, index) ? get(ctx, index) : defaultValue;
	}

	/**
	 * Push a boolean.
	 *
	 * \param ctx the context
	 * \param value the value
	 */
	static inline void push(Context *ctx, bool value)
	{
		duk_push_boolean(ctx, value);
	}

	/**
	 * Require a boolean, throws a JavaScript exception if not a boolean.
	 *
	 * \param ctx the context
	 * \param index the index
	 * \return the boolean
	 */
	static inline bool require(Context *ctx, int index)
	{
		return duk_require_boolean(ctx, index) != 0;
	}
};

/**
 * \class TypeTraits<double>
 * \brief Default implementation for double.
 *
 * Provides: get, is, optional, push, require.
 */
template <>
class TypeTraits<double> {
public:
	/**
	 * Get a double, return 0 if not a double.
	 *
	 * \param ctx the context
	 * \param index the index
	 * \return the double
	 */
	static inline double get(Context *ctx, int index)
	{
		return duk_get_number(ctx, index);
	}

	/**
	 * Check if value is a double.
	 *
	 * \param ctx the context
	 * \param index the index
	 * \return true if double
	 */
	static inline bool is(Context *ctx, int index)
	{
		return duk_is_number(ctx, index) != 0;
	}

	/**
	 * Get a double, return defaultValue if the value is not a double.
	 *
	 * \param ctx the context
	 * \param index the index
	 * \param defaultValue the defaultValue
	 * \return the double or defaultValue
	 */
	static inline double optional(Context *ctx, int index, double defaultValue)
	{
		return is(ctx, index) ? get(ctx, index) : defaultValue;
	}

	/**
	 * Push a double.
	 *
	 * \param ctx the context
	 * \param value the value
	 */
	static inline void push(Context *ctx, double value)
	{
		duk_push_number(ctx, value);
	}

	/**
	 * Require a double, throws a JavaScript exception if not a double.
	 *
	 * \param ctx the context
	 * \param index the index
	 * \return the double
	 */
	static inline double require(Context *ctx, int index)
	{
		return duk_require_number(ctx, index);
	}
};

/**
 * \class TypeTraits<std::string>
 * \brief Default implementation for std::string.
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
	 * \param ctx the context
	 * \param index the index
	 * \return the string
	 */
	static inline std::string get(Context *ctx, int index)
	{
		duk_size_t size;
		const char *text = duk_get_lstring(ctx, index, &size);

		return std::string{text, size};
	}

	/**
	 * Check if value is a string.
	 *
	 * \param ctx the context
	 * \param index the index
	 * \return true if string
	 */
	static inline bool is(Context *ctx, int index)
	{
		return duk_is_string(ctx, index) != 0;
	}

	/**
	 * Get a string, return defaultValue if the value is not an string.
	 *
	 * \param ctx the context
	 * \param index the index
	 * \param defaultValue the defaultValue
	 * \return the string or defaultValue
	 */
	static inline std::string optional(Context *ctx, int index, std::string defaultValue)
	{
		return is(ctx, index) ? get(ctx, index) : defaultValue;
	}

	/**
	 * Push a string.
	 *
	 * \param ctx the context
	 * \param value the value
	 */
	static inline void push(Context *ctx, const std::string &value)
	{
		duk_push_lstring(ctx, value.c_str(), value.length());
	}

	/**
	 * Require a string, throws a JavaScript exception if not a string.
	 *
	 * \param ctx the context
	 * \param index the index
	 * \return the string
	 */
	static inline std::string require(Context *ctx, int index)
	{
		duk_size_t size;
		const char *text = duk_require_lstring(ctx, index, &size);

		return std::string{text, size};
	}
};

/**
 * \class TypeTraits<const char *>
 * \brief Default implementation for const char literals.
 *
 * Provides: get, is, optional, push, require.
 */
template <>
class TypeTraits<const char *> {
public:
	/**
	 * Get a string, return 0 if not a string.
	 *
	 * \param ctx the context
	 * \param index the index
	 * \return the string
	 */
	static inline const char *get(Context *ctx, int index)
	{
		return duk_get_string(ctx, index);
	}

	/**
	 * Check if value is a string.
	 *
	 * \param ctx the context
	 * \param index the index
	 * \return true if string
	 */
	static inline bool is(Context *ctx, int index)
	{
		return duk_is_string(ctx, index) != 0;
	}

	/**
	 * Get an integer, return defaultValue if the value is not an integer.
	 *
	 * \param ctx the context
	 * \param index the index
	 * \param defaultValue the defaultValue
	 * \return the integer or defaultValue
	 */
	static inline const char *optional(Context *ctx, int index, const char *defaultValue)
	{
		return is(ctx, index) ? get(ctx, index) : defaultValue;
	}

	/**
	 * Push a string.
	 *
	 * \param ctx the context
	 * \param value the value
	 */
	static inline void push(Context *ctx, const char *value)
	{
		duk_push_string(ctx, value);
	}

	/**
	 * Require a string, throws a JavaScript exception if not a string.
	 *
	 * \param ctx the context
	 * \param index the index
	 * \return the string
	 */
	static inline const char *require(Context *ctx, int index)
	{
		return duk_require_string(ctx, index);
	}
};

/**
 * \class TypeTraits<unsigned>
 * \brief Default implementation for unsigned.
 *
 * Provides: get, is, optional, push, require.
 */
template <>
class TypeTraits<unsigned> {
public:
	/**
	 * Get an integer, return 0 if not an integer.
	 *
	 * \param ctx the context
	 * \param index the index
	 * \return the integer
	 */
	static inline unsigned get(Context *ctx, int index)
	{
		return duk_get_uint(ctx, index);
	}

	/**
	 * Check if value is an integer.
	 *
	 * \param ctx the context
	 * \param index the index
	 * \return true if integer
	 */
	static inline bool is(Context *ctx, int index)
	{
		return duk_is_number(ctx, index) != 0;
	}

	/**
	 * Get an integer, return defaultValue if the value is not an integer.
	 *
	 * \param ctx the context
	 * \param index the index
	 * \param defaultValue the defaultValue
	 * \return the integer or defaultValue
	 */
	static inline unsigned optional(Context *ctx, int index, unsigned defaultValue)
	{
		return is(ctx, index) ? get(ctx, index) : defaultValue;
	}

	/**
	 * Push an integer.
	 *
	 * \param ctx the context
	 * \param value the value
	 */
	static inline void push(Context *ctx, unsigned value)
	{
		duk_push_uint(ctx, value);
	}

	/**
	 * Require an integer, throws a JavaScript exception if not an integer.
	 *
	 * \param ctx the context
	 * \param index the index
	 * \return the integer
	 */
	static inline unsigned require(Context *ctx, int index)
	{
		return duk_require_uint(ctx, index);
	}
};

/**
 * \brief Implementation for non-managed pointers.
 *
 * Provides: get, is, optional, push, require.
 */
template <typename T>
class TypeTraits<T *> {
public:
	/**
	 * Get a pointer, return nullptr if not a pointer.
	 *
	 * \param ctx the context
	 * \param index the index
	 * \return the pointer
	 */
	static inline T *get(Context *ctx, int index)
	{
		return static_cast<T *>(duk_to_pointer(ctx, index));
	}

	/**
	 * Check if value is a pointer.
	 *
	 * \param ctx the context
	 * \param index the index
	 * \return true if pointer
	 */
	static inline bool is(Context *ctx, int index)
	{
		return duk_is_pointer(ctx, index) != 0;
	}

	/**
	 * Get a pointer, return defaultValue if the value is not a pointer.
	 *
	 * \param ctx the context
	 * \param index the index
	 * \param defaultValue the defaultValue
	 * \return the pointer or defaultValue
	 */
	static inline T *optional(Context *ctx, int index, T *defaultValue)
	{
		return is(ctx, index) ? get(ctx, index) : defaultValue;
	}

	/**
	 * Push a pointer.
	 *
	 * \param ctx the context
	 * \param value the value
	 */
	static inline void push(Context *ctx, T *value)
	{
		duk_push_pointer(ctx, value);
	}

	/**
	 * Require a pointer, throws a JavaScript exception if not a pointer.
	 *
	 * \param ctx the context
	 * \param index the index
	 * \return the pointer
	 */
	static inline T *require(Context *ctx, int index)
	{
		return static_cast<T *>(duk_require_pointer(ctx, index));
	}
};

/**
 * \class TypeTraits<Function>
 * \brief Push C++ function to the stack.
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
	 * \param ctx the context
	 * \param index the value index
	 * \return true if the value is callable
	 */
	static bool is(Context *ctx, Index index)
	{
		return duk_is_callable(ctx, index) != 0;
	}

	/**
	 * Push the C++ function, it is wrapped as Duktape/C function and allocated on the heap by moving the
	 * std::function.
	 *
	 * \param ctx the context
	 * \param fn the function
	 */
	static void push(Context *ctx, Function fn)
	{
		duk_push_c_function(ctx, fn.function, fn.nargs);
	}
};

/**
 * \class TypeTraits<FunctionMap>
 * \brief Put the functions to the object at the top of the stack.
 *
 * Provides: put.
 */
template <>
class TypeTraits<FunctionMap> {
public:
	/**
	 * Push all functions to the object at the top of the stack.
	 *
	 * \param ctx the context
	 * \param map the map of function
	 */
	static void put(Context *ctx, const FunctionMap &map)
	{
		StackAssert sa(ctx, 0);

		for (const auto &entry : map) {
			duk_push_c_function(ctx, entry.second.function, entry.second.nargs);
			duk_put_prop_string(ctx, -2, entry.first.c_str());
		}
	}
};

/**
 * \class TypeTraits<Object>
 * \brief Push empty object to the stack.
 *
 * Provides: is, push.
 */
template <>
class TypeTraits<Object> {
public:
	/**
	 * Check if value is an object.
	 *
	 * \param ctx the context
	 * \param index the index
	 * \return true if object
	 */
	static inline bool is(Context *ctx, int index)
	{
		return duk_is_object(ctx, index) != 0;
	}

	/**
	 * Create an empty object on the stack.
	 *
	 * \param ctx the context
	 */
	static inline void push(Context *ctx, const Object &)
	{
		duk_push_object(ctx);
	}
};

/**
 * \class TypeTraits<Array>
 * \brief Push empty array to the stack.
 *
 * Provides: is, push.
 */
template <>
class TypeTraits<Array> {
public:
	/**
	 * Check if value is a array.
	 *
	 * \param ctx the context
	 * \param index the index
	 * \return true if array
	 */
	static inline bool is(Context *ctx, int index)
	{
		return duk_is_array(ctx, index) != 0;
	}

	/**
	 * Create an empty array on the stack.
	 *
	 * \param ctx the context
	 */
	static inline void push(Context *ctx, const Array &)
	{
		duk_push_array(ctx);
	}
};

/**
 * \class TypeTraits<Undefined>
 * \brief Push undefined value to the stack.
 *
 * Provides: is, push.
 */
template <>
class TypeTraits<Undefined> {
public:
	/**
	 * Check if value is undefined.
	 *
	 * \param ctx the context
	 * \param index the index
	 * \return true if undefined
	 */
	static inline bool is(Context *ctx, int index)
	{
		return duk_is_undefined(ctx, index) != 0;
	}

	/**
	 * Push undefined value on the stack.
	 *
	 * \param ctx the context
	 */
	static inline void push(Context *ctx, const Undefined &)
	{
		duk_push_undefined(ctx);
	}
};

/**
 * \class TypeTraits<Null>
 * \brief Push null value to the stack.
 *
 * Provides: is, push.
 */
template <>
class TypeTraits<Null> {
public:
	/**
	 * Check if value is null.
	 *
	 * \param ctx the context
	 * \param index the index
	 * \return true if null
	 */
	static inline bool is(Context *ctx, int index)
	{
		return duk_is_null(ctx, index) != 0;
	}

	/**
	 * Push null value on the stack.
	 *
	 * \param ctx the context
	 */
	static inline void push(Context *ctx, const Null &)
	{
		duk_push_null(ctx);
	}
};

/**
 * \brief Push this binding into the stack.
 *
 * Provides: push.
 */
template <>
class TypeTraits<This> {
public:
	/**
	 * Push this function into the stack.
	 *
	 * \param ctx the context
	 */
	static inline void push(Context *ctx, const This &)
	{
		duk_push_this(ctx);
	}
};

/**
 * \class TypeTraits<Global>
 * \brief Push the global object to the stack.
 *
 * Provides: push.
 */
template <>
class TypeTraits<Global> {
public:
	/**
	 * Push the global object into the stack.
	 *
	 * \param ctx the context
	 */
	static inline void push(Context *ctx, const Global &)
	{
		duk_push_global_object(ctx);
	}
};

/**
 * \brief Push a map of key-value pair as objects.
 *
 * Provides: push, put.
 *
 * This class is convenient for settings constants such as enums, string and such.
 */
template <typename T>
class TypeTraits<std::unordered_map<std::string, T>> {
public:
	/**
	 * Put all values from the map as properties to the object at the top of the stack.
	 *
	 * \param ctx the context
	 * \param map the values
	 * \note You need an object at the top of the stack before calling this function
	 */
	static void push(Context *ctx, const std::unordered_map<std::string, T> &map)
	{
		StackAssert sa(ctx, 1);

		duk_push_object(ctx);
		put(ctx, map);
	}

	/**
	 * Apply the map to the object at the top of the stack.
	 *
	 * \pre top value must be an object
	 * \param ctx the context
	 * \param map the map
	 */
	static void put(Context *ctx, const std::unordered_map<std::string, T> &map)
	{
		assert(type(ctx, -1) == DUK_TYPE_OBJECT);

		StackAssert sa(ctx);

		for (const auto &pair : map) {
			TypeTraits<T>::push(ctx, pair.second);
			duk_put_prop_string(ctx, -2, pair.first.c_str());
		}
	}
};

/**
 * \brief Push or get vectors as JavaScript arrays.
 *
 * Provides: get, push, put.
 */
template <typename T>
class TypeTraits<std::vector<T>> {
public:
	/**
	 * Get an array from the stack.
	 *
	 * \param ctx the context
	 * \param index the array index
	 * \return the array or empty array if the value is not an array
	 */
	static std::vector<T> get(Context *ctx, int index)
	{
		StackAssert sa(ctx, 0);

		std::vector<T> result;

		if (!duk_is_array(ctx, -1))
			return result;

		size_t total = duk_get_length(ctx, index);

		for (size_t i = 0; i < total; ++i)
			result.push_back(getProperty<T>(ctx, index, static_cast<int>(i)));

		return result;
	}

	/**
	 * Create an array with the specified values.
	 *
	 * \param ctx the context
	 * \param array the values
	 */
	static void push(Context *ctx, const std::vector<T> &array)
	{
		StackAssert sa(ctx, 1);

		duk_push_array(ctx);
		put(ctx, array);
	}

	/**
	 * Apply the array to the object at the top of the stack.
	 *
	 * \pre top value must be an object
	 * \param ctx the context
	 * \param array the array
	 */
	static void put(Context *ctx, const std::vector<T> &array)
	{
		assert(type(ctx, -1) == DUK_TYPE_OBJECT);

		StackAssert sa(ctx);

		unsigned i = 0;
		for (const auto &v : array) {
			TypeTraits<T>::push(ctx, v);
			duk_put_prop_index(ctx, -2, i++);
		}
	}
};

} // !duk

} // !irccd

#endif // !IRCCD_JS_HPP
