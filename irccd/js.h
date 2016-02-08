/*
 * js.h -- JavaScript C++14 wrapper for Duktape
 *
 * Copyright (c) 2013-2016 David Demelier <markand@malikania.fr>
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

#ifndef _JS_H_
#define _JS_H_

/**
 * @file Js.h
 * @brief Bring JavaScript using Duktape
 *
 * This file provides usual Duktape function renamed and placed into `js` namespace. It also replaces error
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
namespace js {

class Context;

/**
 * Typedef for readability.
 */
using ContextPtr = duk_context *;

/*
 * Basic types to manipulate with the stack
 * ------------------------------------------------------------------
 *
 * The following types can be used in some of the operations like Context::push or Context::is, they are defined
 * usually as empty classes to determine the appropriate action to execute.
 *
 * For example, `ctx.push(js::Object{})` will push an empty object into the stack.
 */

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

/*
 * Extended type manipulation
 * ------------------------------------------------------------------
 *
 * The following types are different as there are no equivalent in the native Duktape API, they are available for
 * convenience.
 */

/**
 * @brief Manage shared_ptr from C++ and JavaScript
 *
 * This class allowed you to push and retrieve shared_ptr from C++ and JavaScript without taking care of ownership
 * and deletion.
 *
 * The only requirement is to have the function `void prototype(Context &ctx)` in your class T.
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
	std::function<int (Context &)> function;

	/**
	 * Number of args that the function takes
	 */
	int nargs{0};
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
 * @class TypeInfo
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
class TypeInfo {
};

/**
 * @class File
 * @brief Evaluate script from file.
 * @see Context::eval
 * @see Context::peval
 */
class File {
public:
	/**
	 * Path to the file.
	 */
	std::string path;

	/**
	 * Evaluate the file.
	 *
	 * @param ctx the context
	 */
	inline void eval(duk_context *ctx)
	{
		duk_eval_file(ctx, path.c_str());
	}

	/**
	 * Evaluate in protected mode the file.
	 *
	 * @param ctx the context
	 */
	inline int peval(duk_context *ctx)
	{
		return duk_peval_file(ctx, path.c_str());
	}
};

/**
 * @class Script
 * @brief Evaluate script from raw text.
 * @see Context::eval
 * @see Context::peval
 */
class Script {
public:
	/**
	 * The script content.
	 */
	std::string text;

	/**
	 * Evaluate the script.
	 *
	 * @param ctx the context
	 */
	inline void eval(duk_context *ctx)
	{
		duk_eval_string(ctx, text.c_str());
	}

	/**
	 * Evaluate in protected mode the script.
	 *
	 * @param ctx the context
	 */
	inline int peval(duk_context *ctx)
	{
		return duk_peval_string(ctx, text.c_str());
	}
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

	/* Move and copy forbidden */
	Context(const Context &) = delete;
	Context &operator=(const Context &) = delete;
	Context(const Context &&) = delete;
	Context &operator=(const Context &&) = delete;

public:
	/**
	 * Create default context.
	 */
	inline Context()
		: m_handle{duk_create_heap_default(), duk_destroy_heap}
	{
	}

	/**
	 * Create borrowed context that will not be deleted.
	 *
	 * @param ctx the pointer to duk_context
	 */
	inline Context(ContextPtr ctx) noexcept
		: m_handle{ctx, [] (ContextPtr) {}}
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

	/*
	 * Basic functions
	 * ----------------------------------------------------------
	 *
	 * The following functions are just standard wrappers around the native Duktape C functions, they are
	 * defined with the same signature except that for convenience some parameters have default sane values.
	 */

	/**
	 * Call the object at the top of the stack.
	 *
	 * @param ctx the context
	 * @param nargs the number of arguments
	 * @note Non-protected
	 */
	inline void call(unsigned nargs = 0)
	{
		duk_call(m_handle.get(), nargs);
	}

	/**
	 * Copy a value from from to to, overwriting the previous value. If either index is invalid, throws an error.
	 *
	 * @param from the from index
	 * @param to the destination
	 */
	inline void copy(int from, int to)
	{
		duk_copy(m_handle.get(), from, to);
	}

	/**
	 * Define a property.
	 *
	 * @param index the object index
	 * @param flags the flags
	 * @note Wrapper of duk_def_prop
	 */
	inline void defineProperty(int index, int flags)
	{
		duk_def_prop(m_handle.get(), index, flags);
	}

	/**
	 * Delete a property.
	 *
	 * @param index the object index
	 * @return true if deleted
	 * @note Wrapper of duk_del_prop
	 */
	inline bool deleteProperty(int index)
	{
		return duk_del_prop(m_handle.get(), index);
	}

	/**
	 * Delete a property by index.
	 *
	 * @param index the object index
	 * @param position the property index
	 * @return true if deleted
	 * @note Wrapper of duk_del_prop_index
	 */
	inline bool deleteProperty(int index, int position)
	{
		return duk_del_prop_index(m_handle.get(), index, position);
	}

	/**
	 * Delete a property by name.
	 *
	 * @param index the object index
	 * @param name the property name
	 * @return true if deleted
	 * @note Wrapper of duk_del_prop_string
	 */
	inline bool deleteProperty(int index, const std::string &name)
	{
		return duk_del_prop_string(m_handle.get(), index, name.c_str());
	}

	/**
	 * Push a duplicate of value at from_index to the stack. If from_index is invalid, throws an error.
	 *
	 * @param index the value to copy
	 * @note Wrapper of duk_dup
	 */
	inline void dup(int index = -1)
	{
		duk_dup(m_handle.get(), index);
	}

	/**
	 * Evaluate a non-protected chunk that is at the top of the stack.
	 */
	inline void eval()
	{
		duk_eval(m_handle.get());
	}

	/**
	 * Check if the object as a property.
	 *
	 * @param index the object index
	 * @return true if has
	 * @note Wrapper of duk_has_prop
	 */
	inline bool hasProperty(int index)
	{
		return duk_has_prop(m_handle.get(), index);
	}

	/**
	 * Check if the object as a property by index.
	 *
	 * @param index the object index
	 * @param position the property index
	 * @return true if has
	 * @note Wrapper of duk_has_prop_index
	 */
	inline bool hasProperty(int index, int position)
	{
		return duk_has_prop_index(m_handle.get(), index, position);
	}

	/**
	 * Check if the object as a property by string
	 *
	 * @param index the object index
	 * @param name the property name
	 * @return true if has
	 * @note Wrapper of duk_has_prop_string
	 */
	inline bool hasProperty(int index, const std::string &name)
	{
		return duk_has_prop_string(m_handle.get(), index, name.c_str());
	}

	/**
	 * Check if idx1 is an instance of idx2.
	 *
	 * @param ctx the context
	 * @param idx1 the value to test
	 * @param idx2 the instance requested
	 * @return true if idx1 is instance of idx2
	 * @note Wrapper of duk_instanceof
	 */
	inline bool instanceof(int idx1, int idx2)
	{
		return duk_instanceof(m_handle.get(), idx1, idx2);
	}

	/**
	 * Insert a value at to with a value popped from the stack top. The previous value at to and any values above
	 * it are moved up the stack by a step. If to is an invalid index, throws an error.
	 *
	 * @note Negative indices are evaluated prior to popping the value at the stack top
	 * @param to the destination
	 * @note Wrapper of duk_insert
	 */
	inline void insert(int to)
	{
		duk_insert(m_handle.get(), to);
	}

	/**
	 * Pop a certain number of values from the top of the stack.
	 *
	 * @param ctx the context
	 * @param count the number of values to pop
	 * @note Wrapper of duk_pop_n
	 */
	inline void pop(unsigned count = 1)
	{
		duk_pop_n(m_handle.get(), count);
	}

	/**
	 * Remove value at index. Elements above index are shifted down the stack by a step. If to is an invalid index,
	 * throws an error.
	 *
	 * @param index the value to remove
	 * @note Wrapper of duk_remove
	 */
	inline void remove(int index)
	{
		duk_remove(m_handle.get(), index);
	}

	/**
	 * Replace value at to_index with a value popped from the stack top. If to_index is an invalid index,
	 * throws an error.
	 *
	 * @param index the value to replace by the value at the top of the stack
	 * @note Negative indices are evaluated prior to popping the value at the stack top.
	 * @note Wrapper of duk_replace
	 */
	inline void replace(int index)
	{
		duk_replace(m_handle.get(), index);
	}

	/**
	 * Swap values at indices index1 and index2. If the indices are the same, the call is a no-op. If either index
	 * is invalid, throws an error.
	 *
	 * @param index1 the first index
	 * @param index2 the second index
	 * @note Wrapper of duk_swap
	 */
	inline void swap(int index1, int index2)
	{
		duk_swap(m_handle.get(), index1, index2);
	}

	/**
	 * Get the current stack size.
	 *
	 * @param ctx the context
	 * @return the stack size
	 * @note Wrapper of duk_get_top
	 */
	inline int top() noexcept
	{
		return duk_get_top(m_handle.get());
	}

	/**
	 * Get the type of the value at the specified index.
	 *
	 * @param ctx the context
	 * @param index the idnex
	 * @return the type
	 * @note Wrapper of duk_get_type
	 */
	inline int type(int index) noexcept
	{
		return duk_get_type(m_handle.get(), index);
	}

	/*
	 * Extended native functions
	 * ----------------------------------------------------------
	 *
	 * The following functions have different behaviour than the original native Duktape C functions, see their
	 * descriptions for more information
	 */

	/**
	 * Call in protected mode the object at the top of the stack.
	 *
	 * @param nargs the number of arguments
	 * @throw ErrorInfo on errors
	 * @note Wrapper of duk_pcall
	 */
	void pcall(unsigned nargs = 0);

	/**
	 * Evaluate a non-protected source.
	 *
	 * @param source the source
	 * @see File
	 * @see Script
	 * @note Wrapper of duk_eval
	 */
	template <typename Source>
	inline void eval(Source &&source)
	{
		source.eval(m_handle.get());
	}

	/**
	 * Evaluate a protected chunk that is at the top of the stack.
	 *
	 * @throw ErrorInfo the error
	 * @note Wrapper of duk_peval
	 */
	void peval();

	/**
	 * Evaluate a protected source.
	 *
	 * @param source the source
	 * @see File
	 * @see Script
	 * @throw ErrorInfo on failure
	 * @note Wrapper of duk_peval
	 */
	template <typename Source>
	inline void peval(Source &&source)
	{
		if (source.peval(m_handle.get()) != 0) {
			ErrorInfo info = error(-1);
			duk_pop(m_handle.get());

			throw info;
		}
	}

	/*
	 * Push / Get / Require / Is / Optional
	 * ----------------------------------------------------------
	 *
	 * The following functions are used to push, get or check values from the stack. They use specialization
	 * of TypeInfo class.
	 */

	/**
	 * Push a value into the stack. Calls TypeInfo<T>::push(*this, value);
	 *
	 * @param value the value to forward
	 */
	template <typename Type>
	inline void push(Type &&value)
	{
		TypeInfo<std::decay_t<Type>>::push(*this, std::forward<Type>(value));
	}

	/**
	 * Generic template function to get a value from the stack.
	 *
	 * @param index the index
	 * @return the value
	 */
	template <typename Type>
	inline auto get(int index) -> decltype(TypeInfo<Type>::get(*this, 0))
	{
		return TypeInfo<Type>::get(*this, index);
	}

	/**
	 * Require a type at the specified index.
	 *
	 * @param index the index
	 * @return the value
	 */
	template <typename Type>
	inline auto require(int index) -> decltype(TypeInfo<Type>::require(*this, 0))
	{
		return TypeInfo<Type>::require(*this, index);
	}

	/**
	 * Check if a value is a type of T.
	 *
	 * The TypeInfo<T> must have `static bool is(ContextPtr ptr, int index)`.
	 *
	 * @param index the value index
	 * @return true if is the type
	 */
	template <typename T>
	inline bool is(int index)
	{
		return TypeInfo<T>::is(*this, index);
	}

	/**
	 * Get an optional value from the stack, if the value is not available of not the correct type,
	 * return defaultValue instead.
	 *
	 * The TypeInfo<T> must have `static T optional(Context &, int index, T &&defaultValue)`.
	 *
	 * @param index the value index
	 * @param defaultValue the value replacement
	 * @return the value or defaultValue
	 */
	template <typename Type, typename DefaultValue>
	inline auto optional(int index, DefaultValue &&defaultValue)
	{
		return TypeInfo<std::decay_t<Type>>::optional(*this, index, std::forward<DefaultValue>(defaultValue));
	}

	/*
	 * Properties management
	 * ----------------------------------------------------------
	 *
	 * The following functions are used to read or set properties on objects or globals also using TypeInfo.
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
	inline auto getProperty(int index, const std::string &name) -> decltype(get<Type>(0))
	{
		duk_get_prop_string(m_handle.get(), index, name.c_str());
		decltype(get<Type>(0)) value = get<Type>(-1);
		duk_pop(m_handle.get());

		return value;
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
	inline auto optionalProperty(int index, const std::string &name, DefaultValue &&def) -> decltype(optional<Type>(0, std::forward<DefaultValue>(def)))
	{
		duk_get_prop_string(m_handle.get(), index, name.c_str());
		decltype(optional<Type>(0, std::forward<DefaultValue>(def))) value = optional<Type>(-1, std::forward<DefaultValue>(def));
		duk_pop(m_handle.get());

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
	inline auto getProperty(int index, int position) -> decltype(get<Type>(0))
	{
		duk_get_prop_index(m_handle.get(), index, position);
		decltype(get<Type>(0)) value = get<Type>(-1);
		duk_pop(m_handle.get());

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
	inline auto optionalProperty(int index, int position, DefaultValue &&def) -> decltype(optional<Type>(0, std::forward<DefaultValue>(def)))
	{
		duk_get_prop_index(m_handle.get(), index, position);
		decltype(optional<Type>(0, std::forward<DefaultValue>(def))) value = optional<Type>(-1, std::forward<DefaultValue>(def));
		duk_pop(m_handle.get());

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
	inline void getProperty(int index, const std::string &name)
	{
		duk_get_prop_string(m_handle.get(), index, name.c_str());
	}

	/**
	 * Get the property by index and push it to the stack from the object at the specified index.
	 *
	 * @param index the object index
	 * @param position the position in the object
	 * @note The stack contains the property value
	 */
	template <typename Type, typename std::enable_if_t<std::is_void<Type>::value> * = nullptr>
	inline void getProperty(int index, int position)
	{
		duk_get_prop_index(m_handle.get(), index, position);
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
	void putProperty(int index, const std::string &name, Type &&value)
	{
		index = duk_normalize_index(m_handle.get(), index);

		push(std::forward<Type>(value));
		duk_put_prop_string(m_handle.get(), index, name.c_str());
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
	void putProperty(int index, int position, Type &&value)
	{
		index = duk_normalize_index(m_handle.get(), index);

		push(std::forward<Type>(value));
		duk_put_prop_index(m_handle.get(), index, position);
	}

	/**
	 * Put the value that is at the top of the stack as property to the object.
	 *
	 * @param index the object index
	 * @param name the property name
	 */
	inline void putProperty(int index, const std::string &name)
	{
		duk_put_prop_string(m_handle.get(), index, name.c_str());
	}

	/**
	 * Put the value that is at the top of the stack to the object as index.
	 *
	 * @param index the object index
	 * @param position the position in the object
	 */
	inline void putProperty(int index, int position)
	{
		duk_put_prop_index(m_handle.get(), index, position);
	}

	/**
	 * Get a global value.
	 *
	 * @param name the name of the global variable
	 * @return the value
	 */
	template <typename Type>
	inline auto getGlobal(const std::string &name, std::enable_if_t<!std::is_void<Type>::value> * = nullptr) -> decltype(get<Type>(0))
	{
		duk_get_global_string(m_handle.get(), name.c_str());
		decltype(get<Type>(0)) value = get<Type>(-1);
		duk_pop(m_handle.get());

		return value;
	}

	/**
	 * Overload that push the value at the top of the stack instead of returning it.
	 */
	template <typename Type>
	inline void getGlobal(const std::string &name, std::enable_if_t<std::is_void<Type>::value> * = nullptr) noexcept
	{
		duk_get_global_string(m_handle.get(), name.c_str());
	}

	/**
	 * Set a global variable.
	 *
	 * @param name the name of the global variable
	 * @param type the value to set
	 */
	template <typename Type>
	inline void putGlobal(const std::string &name, Type&& type)
	{
		push(std::forward<Type>(type));
		duk_put_global_string(m_handle.get(), name.c_str());
	}

	/**
	 * Put the value at the top of the stack as global property.
	 *
	 * @param name the property name
	 */
	inline void putGlobal(const std::string &name)
	{
		duk_put_global_string(m_handle.get(), name.c_str());
	}

	/*
	 * Extra functions
	 * ----------------------------------------------------------
	 *
	 * The following functions are implemented for convenience and do not exists in the native Duktape API.
	 */

	/**
	 * Get the error object when a JavaScript error has been thrown (e.g. eval failure).
	 *
	 * @param index the index
	 * @return the information
	 */
	ErrorInfo error(int index);

	/**
	 * Enumerate an object or an array at the specified index.
	 *
	 * @param index the object or array index
	 * @param flags the optional flags to pass to duk_enum
	 * @param getvalue set to true if you want to extract the value
	 * @param func the function to call for each properties
	 */
	template <typename Func>
	void enumerate(int index, duk_uint_t flags, duk_bool_t getvalue, Func&& func)
	{
		duk_enum(m_handle.get(), index, flags);

		while (duk_next(m_handle.get(), -1, getvalue)) {
			func(*this);
			duk_pop_n(m_handle.get(), 1 + (getvalue ? 1 : 0));
		}

		duk_pop(m_handle.get());
	}

	/**
	 * Return the this binding of the current function.
	 *
	 * @return the this binding as the template given
	 */
	template <typename T>
	inline auto self() -> decltype(TypeInfo<T>::get(*this, 0))
	{
		duk_push_this(m_handle.get());
		decltype(TypeInfo<T>::get(*this, 0)) value = TypeInfo<T>::get(*this, -1);
		duk_pop(m_handle.get());

		return value;
	}

	inline void raise()
	{
		duk_throw(m_handle.get());
	}

	/**
	 * Throw an ECMAScript exception.
	 *
	 * @param ex the exception
	 */
	template <typename Exception>
	void raise(const Exception &ex)
	{
		ex.create(*this);
		raise();
	}

	/**
	 * Construct the object in place, setting value as this binding.
	 *
	 * The TypeInfo<T> must have the following requirements:
	 *
	 * - static void construct(Context &, T): must update this with the value and keep the stack unchanged
	 *
	 * @param value the value to forward
	 * @see self
	 */
	template <typename T>
	inline void construct(T &&value)
	{
		TypeInfo<std::decay_t<T>>::construct(*this, std::forward<T>(value));
	}
};

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
	Context &m_context;
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
	inline StackAssert(Context &ctx, unsigned expected = 0) noexcept
#if !defined(NDEBUG)
		: m_context{ctx}
		, m_expected{expected}
		, m_begin{static_cast<unsigned>(m_context.top())}
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
		assert(m_context.top() - m_begin == m_expected);
#endif
	}
};

/* ------------------------------------------------------------------
 * Exception handling
 * ------------------------------------------------------------------ */

/**
 * @class Error
 * @brief Base ECMAScript error class.
 * @warning Override the function create for your own exceptions
 */
class Error {
protected:
	std::string m_name;	//!< Name of exception (e.g RangeError)
	std::string m_message;	//!< The message

	/**
	 * Constructor with a type of error specified, specially designed for derived errors.
	 *
	 * @param name the error name (e.g RangeError)
	 * @param message the message
	 */
	inline Error(std::string name, std::string message) noexcept
		: m_name{std::move(name)}
		, m_message{std::move(message)}
	{
	}

public:
	/**
	 * Constructor with a message.
	 *
	 * @param message the message
	 */
	inline Error(std::string message) noexcept
		: m_name{"Error"}
		, m_message{std::move(message)}
	{
	}

	/**
	 * Get the error type (e.g RangeError).
	 *
	 * @return the name
	 */
	inline const std::string &name() const noexcept
	{
		return m_name;
	}

	/**
	 * Create the exception on the stack.
	 *
	 * @note the default implementation search for the global variables
	 * @param ctx the context
	 */
	virtual void create(Context &ctx) const noexcept
	{
		duk_get_global_string(ctx, m_name.c_str());
		duk_push_string(ctx, m_message.c_str());
		duk_new(ctx, 1);
		duk_push_string(ctx, m_name.c_str());
		duk_put_prop_string(ctx, -2, "name");
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
		: Error{"EvalError", std::move(message)}
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
		: Error{"RangeError", std::move(message)}
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
		: Error{"ReferenceError", std::move(message)}
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
		: Error{"SyntaxError", std::move(message)}
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
		: Error{"TypeError", std::move(message)}
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
		: Error{"URIError", std::move(message)}
	{
	}
};

/* ------------------------------------------------------------------
 * Standard overloads for TypeInfo<T>
 * ------------------------------------------------------------------ */

/**
 * @class TypeInfo<int>
 * @brief Default implementation for int.
 *
 * Provides: get, is, optional, push, require.
 */
template <>
class TypeInfo<int> {
public:
	/**
	 * Get an integer, return 0 if not an integer.
	 *
	 * @param ctx the context
	 * @param index the index
	 * @return the integer
	 */
	static inline int get(Context &ctx, int index)
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
	static inline bool is(Context &ctx, int index)
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
	static inline int optional(Context &ctx, int index, int defaultValue)
	{
		if (!duk_is_number(ctx, index)) {
			return defaultValue;
		}

		return duk_get_int(ctx, index);
	}

	/**
	 * Push an integer.
	 *
	 * @param ctx the context
	 * @param value the value
	 */
	static inline void push(Context &ctx, int value)
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
	static inline int require(Context &ctx, int index)
	{
		return duk_require_int(ctx, index);
	}
};

/**
 * @class TypeInfo<bool>
 * @brief Default implementation for bool.
 *
 * Provides: get, is, optional, push, require.
 */
template <>
class TypeInfo<bool> {
public:
	/**
	 * Get a boolean, return 0 if not a boolean.
	 *
	 * @param ctx the context
	 * @param index the index
	 * @return the boolean
	 */
	static inline bool get(Context &ctx, int index)
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
	static inline bool is(Context &ctx, int index)
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
	static inline bool optional(Context &ctx, int index, bool defaultValue)
	{
		if (!duk_is_boolean(ctx, index)) {
			return defaultValue;
		}

		return duk_get_boolean(ctx, index);
	}

	/**
	 * Push a boolean.
	 *
	 * @param ctx the context
	 * @param value the value
	 */
	static inline void push(Context &ctx, bool value)
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
	static inline bool require(Context &ctx, int index)
	{
		return duk_require_boolean(ctx, index);
	}
};

/**
 * @class TypeInfo<double>
 * @brief Default implementation for double.
 *
 * Provides: get, is, optional, push, require.
 */
template <>
class TypeInfo<double> {
public:
	/**
	 * Get a double, return 0 if not a double.
	 *
	 * @param ctx the context
	 * @param index the index
	 * @return the double
	 */
	static inline double get(Context &ctx, int index)
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
	static inline bool is(Context &ctx, int index)
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
	static inline double optional(Context &ctx, int index, double defaultValue)
	{
		if (!duk_is_number(ctx, index)) {
			return defaultValue;
		}

		return duk_get_number(ctx, index);
	}

	/**
	 * Push a double.
	 *
	 * @param ctx the context
	 * @param value the value
	 */
	static inline void push(Context &ctx, double value)
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
	static inline double require(Context &ctx, int index)
	{
		return duk_require_number(ctx, index);
	}
};

/**
 * @class TypeInfo<std::string>
 * @brief Default implementation for std::string.
 *
 * Provides: get, is, optional, push, require.
 *
 * Note: the functions allows embedded '\0'.
 */
template <>
class TypeInfo<std::string> {
public:
	/**
	 * Get a string, return 0 if not a string.
	 *
	 * @param ctx the context
	 * @param index the index
	 * @return the string
	 */
	static inline std::string get(Context &ctx, int index)
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
	static inline bool is(Context &ctx, int index)
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
	static inline std::string optional(Context &ctx, int index, std::string defaultValue)
	{
		if (!duk_is_string(ctx, index)) {
			return defaultValue;
		}

		return get(ctx, index);
	}

	/**
	 * Push a string.
	 *
	 * @param ctx the context
	 * @param value the value
	 */
	static inline void push(Context &ctx, const std::string &value)
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
	static inline std::string require(Context &ctx, int index)
	{
		duk_size_t size;
		const char *text = duk_require_lstring(ctx, index, &size);

		return std::string{text, size};
	}
};

/**
 * @class TypeInfo<const char *>
 * @brief Default implementation for const char literals.
 *
 * Provides: get, is, optional, push, require.
 */
template <>
class TypeInfo<const char *> {
public:
	/**
	 * Get a string, return 0 if not a string.
	 *
	 * @param ctx the context
	 * @param index the index
	 * @return the string
	 */
	static inline const char *get(Context &ctx, int index)
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
	static inline bool is(Context &ctx, int index)
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
	static inline const char *optional(Context &ctx, int index, const char *defaultValue)
	{
		if (!duk_is_string(ctx, index)) {
			return defaultValue;
		}

		return duk_get_string(ctx, index);
	}

	/**
	 * Push a string.
	 *
	 * @param ctx the context
	 * @param value the value
	 */
	static inline void push(Context &ctx, const char *value)
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
	static inline const char *require(Context &ctx, int index)
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
class TypeInfo<RawPointer<T>> {
public:
	/**
	 * Get a pointer, return nullptr if not a pointer.
	 *
	 * @param ctx the context
	 * @param index the index
	 * @return the pointer
	 */
	static inline T *get(Context &ctx, int index)
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
	static inline bool is(Context &ctx, int index)
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
	static inline T *optional(Context &ctx, int index, RawPointer<T> defaultValue)
	{
		if (!duk_is_pointer(ctx, index)) {
			return defaultValue.object;
		}

		return static_cast<T *>(duk_to_pointer(ctx, index));
	}

	/**
	 * Push a pointer.
	 *
	 * @param ctx the context
	 * @param value the value
	 */
	static inline void push(Context &ctx, const RawPointer<T> &value)
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
	static inline T *require(Context &ctx, int index)
	{
		return static_cast<T *>(duk_require_pointer(ctx, index));
	}
};

/**
 * @class TypeInfo<Function>
 * @brief Push C++ function to the stack.
 *
 * Provides: push.
 *
 * This implementation push a Duktape/C function that is wrapped as C++ for convenience.
 */
template <>
class TypeInfo<Function> {
public:
	/**
	 * Push the C++ function, it is wrapped as Duktape/C function and allocated on the heap by moving the
	 * std::function.
	 *
	 * @param ctx the context
	 * @param fn the function
	 */
	static void push(Context &ctx, Function fn);
};

/**
 * @class TypeInfo<FunctionMap>
 * @brief Put the functions to the object at the top of the stack.
 *
 * Provides: push.
 */
template <>
class TypeInfo<FunctionMap> {
public:
	/**
	 * Push a map of function to the object at the top of the stack.
	 *
	 * @param ctx the context
	 * @param map the map of function
	 */
	static inline void push(Context &ctx, const FunctionMap &map)
	{
		for (const auto &entry : map) {
			ctx.putProperty(-1, entry.first, entry.second);
		}
	}
};

/**
 * @class TypeInfo<Object>
 * @brief Push empty object to the stack.
 *
 * Provides: is, push.
 */
template <>
class TypeInfo<Object> {
public:
	/**
	 * Check if value is an object.
	 *
	 * @param ctx the context
	 * @param index the index
	 * @return true if object
	 */
	static inline bool is(Context &ctx, int index)
	{
		return duk_is_object(ctx, index);
	}

	/**
	 * Create an empty object on the stack.
	 *
	 * @param ctx the context
	 */
	static inline void push(Context &ctx, const Object &)
	{
		duk_push_object(ctx);
	}
};

/**
 * @class TypeInfo<Array>
 * @brief Push empty array to the stack.
 *
 * Provides: is, push.
 */
template <>
class TypeInfo<Array> {
public:
	/**
	 * Check if value is a array.
	 *
	 * @param ctx the context
	 * @param index the index
	 * @return true if array
	 */
	static inline bool is(Context &ctx, int index)
	{
		return duk_is_array(ctx, index);
	}

	/**
	 * Create an empty array on the stack.
	 *
	 * @param ctx the context
	 */
	static inline void push(Context &ctx, const Array &)
	{
		duk_push_array(ctx);
	}
};

/**
 * @class TypeInfo<Undefined>
 * @brief Push undefined value to the stack.
 *
 * Provides: is, push.
 */
template <>
class TypeInfo<Undefined> {
public:
	/**
	 * Check if value is undefined.
	 *
	 * @param ctx the context
	 * @param index the index
	 * @return true if undefined
	 */
	static inline bool is(Context &ctx, int index)
	{
		return duk_is_undefined(ctx, index);
	}

	/**
	 * Push undefined value on the stack.
	 *
	 * @param ctx the context
	 */
	static inline void push(Context &ctx, const Undefined &)
	{
		duk_push_undefined(ctx);
	}
};

/**
 * @class TypeInfo<Null>
 * @brief Push null value to the stack.
 *
 * Provides: is, push.
 */
template <>
class TypeInfo<Null> {
public:
	/**
	 * Check if value is null.
	 *
	 * @param ctx the context
	 * @param index the index
	 * @return true if null
	 */
	static inline bool is(Context &ctx, int index)
	{
		return duk_is_null(ctx, index);
	}

	/**
	 * Push null value on the stack.
	 *
	 * @param ctx the context
	 */
	static inline void push(Context &ctx, const Null &)
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
class TypeInfo<This> {
public:
	/**
	 * Push this function into the stack.
	 *
	 * @param ctx the context
	 */
	static inline void push(Context &ctx, const This &)
	{
		duk_push_this(ctx);
	}
};

/**
 * @class TypeInfo<Global>
 * @brief Push the global object to the stack.
 *
 * Provides: push.
 */
template <>
class TypeInfo<Global> {
public:
	/**
	 * Push the global object into the stack.
	 *
	 * @param ctx the context
	 */
	static inline void push(Context &ctx, const Global &)
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
class TypeInfo<std::unordered_map<std::string, T>> {
public:
	/**
	 * Put all values from the map as properties to the object at the top of the stack.
	 *
	 * @param ctx the context
	 * @param map the values
	 * @note You need an object at the top of the stack before calling this function
	 */
	static void push(Context &ctx, const std::unordered_map<std::string, T> &map)
	{
		for (const auto &pair : map) {
			TypeInfo<T>::push(ctx, pair.second);
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
class TypeInfo<std::vector<T>> {
public:
	/**
	 * Get an array from the stack.
	 *
	 * @param ctx the context
	 * @param index the array index
	 * @return the array or empty array if the value is not an array
	 */
	static std::vector<T> get(Context &ctx, int index)
	{
		std::vector<T> result;

		if (!duk_is_array(ctx, -1)) {
			return result;
		}

		int total = duk_get_length(ctx, index);
		for (int i = 0; i < total; ++i) {
			result.push_back(ctx.getProperty<T>(index, i));
		}

		return result;
	}

	/**
	 * Create an array with the specified values.
	 *
	 * @param ctx the context
	 * @param array the values
	 */
	static void push(Context &ctx, const std::vector<T> &array)
	{
		duk_push_array(ctx);

		int i = 0;
		for (const auto &v : array) {
			TypeInfo<T>::push(ctx, v);
			duk_put_prop_index(ctx, -2, i++);
		}
	}
};

/**
 * @brief Implementation of managed shared_ptr
 * @see Shared
 */
template <typename T>
class TypeInfo<Shared<T>> {
private:
	static void apply(Context &ctx, std::shared_ptr<T> value)
	{
		duk_push_boolean(ctx, false);
		duk_put_prop_string(ctx, -2, "\xff""\xff""js-deleted");
		duk_push_pointer(ctx, new std::shared_ptr<T>(value));
		duk_put_prop_string(ctx, -2, "\xff""\xff""js-shared-ptr");
		duk_push_c_function(ctx, [] (duk_context *ctx) -> duk_ret_t {
			duk_get_prop_string(ctx, 0, "\xff""\xff""js-deleted");

			if (!duk_to_boolean(ctx, -1)) {
				duk_push_boolean(ctx, true);
				duk_put_prop_string(ctx, 0, "\xff""\xff""js-deleted");
				duk_get_prop_string(ctx, 0, "\xff""\xff""js-shared-ptr");
				delete static_cast<std::shared_ptr<T> *>(duk_to_pointer(ctx, -1));
				duk_pop(ctx);
			}

			duk_pop(ctx);

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
	static void construct(Context &ctx, Shared<T> value)
	{
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
	static void push(Context &ctx, Shared<T> value)
	{
		js::StackAssert sa{ctx, 1};

		duk_push_object(ctx);
		value.object->prototype(ctx);
		duk_set_prototype(ctx, -2);
		apply(ctx, value.object);
	}

	/**
	 * Get a managed shared_ptr from the stack.
	 *
	 * @param ctx the context
	 * @param index the object index
	 * @return the shared_ptr
	 */
	static std::shared_ptr<T> get(Context &ctx, int index)
	{
		/* Verify that it is the correct type */
		duk_get_prop_string(ctx, index, T::name());

		if (duk_get_type(ctx, -1) == DUK_TYPE_UNDEFINED) {
			duk_pop(ctx);
			ctx.raise(ReferenceError("invalid this binding"));
		} else {
			duk_pop(ctx);
		}

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
class TypeInfo<Pointer<T>> {
private:
	static void apply(Context &ctx, T *value)
	{
		duk_push_boolean(ctx, false);
		duk_put_prop_string(ctx, -2, "\xff""\xff""js-deleted");
		duk_push_pointer(ctx, value);
		duk_put_prop_string(ctx, -2, "\xff""\xff""js-ptr");
		duk_push_c_function(ctx, [] (duk_context *ctx) -> duk_ret_t {
			duk_get_prop_string(ctx, 0, "\xff""\xff""js-deleted");

			if (!duk_to_boolean(ctx, -1)) {
				duk_push_boolean(ctx, true);
				duk_put_prop_string(ctx, 0, "\xff""\xff""js-deleted");
				duk_get_prop_string(ctx, 0, "\xff""\xff""js-ptr");
				delete static_cast<T *>(duk_to_pointer(ctx, -1));
				duk_pop(ctx);
			}

			duk_pop(ctx);

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
	static void construct(Context &ctx, Pointer<T> value)
	{
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
	static void push(Context &ctx, Pointer<T> value)
	{
		js::StackAssert sa{ctx, 1};

		duk_push_object(ctx);
		apply(ctx, value.object);
		value.object->prototype(ctx);
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
	static T *get(Context &ctx, int index)
	{
		/* Verify that it is the correct type */
		duk_get_prop_string(ctx, index, T::name());

		if (duk_get_type(ctx, -1) == DUK_TYPE_UNDEFINED) {
			duk_pop(ctx);
			ctx.raise(ReferenceError("invalid this binding"));
		} else {
			duk_pop(ctx);
		}

		duk_get_prop_string(ctx, index, "\xff""\xff""js-ptr");
		T *value = static_cast<T *>(duk_to_pointer(ctx, -1));
		duk_pop(ctx);

		return value;
	}
};

} // !js

} // !irccd

#endif // !_JS_H_
