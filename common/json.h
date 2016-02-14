/*
 * json.h -- C++14 JSON manipulation using jansson parser
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

#ifndef _JSON_H_
#define _JSON_H_

/**
 * @file json.h
 * @brief Jansson C++14 wrapper
 *
 * These classes can be used to build or parse JSON documents using jansson library. It is designed to be safe
 * and explicit. It does not implement implicit sharing like jansson so when you access (e.g. Value::toObject) values
 * you get real copies, thus when you read big documents it can has a performance cost.
 */

#include <cassert>
#include <exception>
#include <initializer_list>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace irccd {

/**
 * Json namespace.
 */
namespace json {

/**
 * @enum Type
 * @brief Type of Value.
 */
enum class Type {
	Array,		//!< Value is an array []
	Boolean,	//!< Value is boolean
	Int,		//!< Value is integer
	Null,		//!< Value is defined to null
	Object,		//!< Value is object {}
	Real,		//!< Value is float
	String		//!< Value is unicode string
};

/**
 * @class Error
 * @brief Error description.
 */
class Error : public std::exception {
private:
	std::string m_text;
	std::string m_source;
	int m_line;
	int m_column;
	int m_position;

public:
	/**
	 * Create the error.
	 *
	 * @param text the text message
	 * @param source the source (e.g. file name)
	 * @param line the line number
	 * @param column the column number
	 * @param position the position
	 */
	inline Error(std::string text, std::string source, int line, int column, int position) noexcept
		: m_text(std::move(text))
		, m_source(std::move(source))
		, m_line(line)
		, m_column(column)
		, m_position(position)
	{
	}

	/**
	 * Get the error message.
	 *
	 * @return the text
	 */
	inline const std::string &text() const noexcept
	{
		return m_text;
	}

	/**
	 * Get the source (e.g. a file name).
	 *
	 * @return the source
	 */
	inline const std::string &source() const noexcept
	{
		return m_source;
	}

	/**
	 * Get the line.
	 *
	 * @return the line
	 */
	inline int line() const noexcept
	{
		return m_line;
	}

	/**
	 * Get the column.
	 *
	 * @return the column
	 */
	inline int column() const noexcept
	{
		return m_column;
	}

	/**
	 * Get the position.
	 *
	 * @return the position
	 */
	inline int position() const noexcept
	{
		return m_position;
	}

	/**
	 * Get the error message.
	 *
	 * @return the message
	 */
	const char *what() const noexcept override
	{
		return m_text.c_str();
	}
};

/**
 * @class Buffer
 * @brief Open JSON document from text.
 */
class Buffer {
public:
	std::string text;	//!< The JSON text
};

/**
 * @class File
 * @brief Open JSON document from a file.
 */
class File {
public:
	std::string path;	//!< The path to the file
};

/**
 * @class Value
 * @brief Generic JSON value wrapper.
 */
class Value {
private:
	Type m_type{Type::Null};

	union {
		double m_number;
		bool m_boolean;
		int m_integer;
		std::string m_string;
		std::vector<Value> m_array;
		std::map<std::string, Value> m_object;
	};

	void copy(const Value &);
	void move(Value &&);
	std::string toJson(int indent, int current) const;

	/**
	 * @class BaseIterator
	 * @brief This is the base class for iterator and const_iterator
	 *
	 * This iterator works for both arrays and objects. Because of that purpose, it is only available
	 * as forward iterator.
	 *
	 * When iterator comes from an object, you can use key() otherwise you can use index().
	 */
	template <typename ValueType, typename ArrayIteratorType, typename ObjectIteratorType>
	class BaseIterator : public std::iterator<std::forward_iterator_tag, ValueType> {
	private:
		friend class Value;

		ValueType &m_value;
		ArrayIteratorType m_ita;
		ObjectIteratorType m_itm;

		inline void increment()
		{
			if (m_value.isObject())
				m_itm++;
			else
				m_ita++;
		}

		BaseIterator(ValueType &value, ObjectIteratorType it)
			: m_value(value)
			, m_itm(it)
		{
		}

		BaseIterator(ValueType &value, ArrayIteratorType it)
			: m_value(value)
			, m_ita(it)
		{
		}

	public:
		/**
		 * Get the iterator key (for objects).
		 *
		 * @pre iterator must be dereferenceable
		 * @pre iterator must come from object
		 * @return the key
		 */
		inline const std::string &key() const noexcept
		{
			assert(m_value.isObject());
			assert(m_itm != m_value.m_object.end());

			return m_itm->first;
		}

		/**
		 * Get the iterator position (for arrays).
		 *
		 * @pre iterator must be dereferenceable
		 * @pre iterator must come from arrays
		 * @return the index
		 */
		inline unsigned index() const noexcept
		{
			assert(m_value.isArray());
			assert(m_ita != m_value.m_array.end());

			return std::distance(m_value.m_array.begin(), m_ita);
		}

		/**
		 * Dereference the iterator.
		 *
		 * @pre iterator be dereferenceable
		 * @return the value
		 */
		inline ValueType &operator*() noexcept
		{
			assert((m_value.isArray()  && m_ita != m_value.m_array.end()) ||
			       (m_value.isObject() && m_itm != m_value.m_object.end()));

			return (m_value.m_type == Type::Object) ? m_itm->second : *m_ita;
		}

		/**
		 * Dereference the iterator as a pointer.
		 *
		 * @pre iterator must be dereferenceable
		 * @return the value
		 */
		inline ValueType *operator->() noexcept
		{
			assert((m_value.isArray()  && m_ita != m_value.m_array.end()) ||
			       (m_value.isObject() && m_itm != m_value.m_object.end()));

			return (m_value.m_type == Type::Object) ? &m_itm->second : &(*m_ita);
		}

		/**
		 * Increment the iterator. (Prefix version).
		 *
		 * @pre iterator must be dereferenceable
		 * @return *this;
		 */
		inline BaseIterator &operator++() noexcept
		{
			assert((m_value.isArray()  && m_ita != m_value.m_array.end()) ||
			       (m_value.isObject() && m_itm != m_value.m_object.end()));

			increment();

			return *this;
		}

		/**
		 * Increment the iterator. (Postfix version).
		 *
		 * @pre iterator must be dereferenceable
		 * @return *this;
		 */
		inline BaseIterator &operator++(int) noexcept
		{
			assert((m_value.isArray()  && m_ita != m_value.m_array.end()) ||
			       (m_value.isObject() && m_itm != m_value.m_object.end()));

			increment();

			return *this;
		}

		/**
		 * Compare two iterators.
		 *
		 * @param it1 the first iterator
		 * @param it2 the second iterator
		 * @return true if they are same
		 */
		bool operator==(const BaseIterator &it) const noexcept
		{
			if (m_value.isObject() && it.m_value.isObject())
				return m_itm == it.m_itm;
			if (m_value.isArray() && it.m_value.isArray())
				return m_ita == it.m_ita;

			return false;
		}

		/**
		 * Test if the iterator is different.
		 *
		 * @param it the iterator
		 * @return true if they are different
		 */
		inline bool operator!=(const BaseIterator &it) const noexcept
		{
			return !(*this == it);
		}
	};

public:
	/**
	 * Forward iterator.
	 */
	using iterator = BaseIterator<Value, typename std::vector<Value>::iterator, typename std::map<std::string, Value>::iterator>;

	/**
	 * Const forward iterator.
	 */
	using const_iterator = BaseIterator<const Value, typename std::vector<Value>::const_iterator, typename std::map<std::string, Value>::const_iterator>;

	/**
	 * Construct a null value.
	 */
	inline Value()
	{
	}

	/**
	 * Create a value with a specified type, this is usually only needed when you want to create an object or
	 * an array.
	 *
	 * For any other types, initialize with sane default value.
	 *
	 * @param type the type
	 */
	Value(Type type);

	/**
	 * Construct a null value.
	 */
	inline Value(std::nullptr_t) noexcept
		: m_type(Type::Null)
	{
	}

	/**
	 * Construct a boolean value.
	 *
	 * @param value the boolean value
	 */
	inline Value(bool value) noexcept
		: m_type(Type::Boolean)
		, m_boolean(value)
	{
	}

	/**
	 * Create value from integer.
	 *
	 * @param value the value
	 */
	inline Value(int value) noexcept
		: m_type(Type::Int)
		, m_integer(value)
	{
	}

	/**
	 * Construct a value from a C-string.
	 *
	 * @param value the C-string
	 */
	inline Value(const char *value)
		: m_type(Type::String)
	{
		new (&m_string) std::string(value ? value : "");
	}

	/**
	 * Construct a number value.
	 *
	 * @param value the real value
	 */
	inline Value(double value) noexcept
		: m_type(Type::Real)
		, m_number(value)
	{
	}

	/**
	 * Construct a string value.
	 *
	 * @param value the string
	 */
	inline Value(std::string value) noexcept
		: m_type(Type::String)
	{
		new (&m_string) std::string(std::move(value));
	}

	/**
	 * Create an object from a map.
	 *
	 * @param values the values
	 * @see fromObject
	 */
	inline Value(std::map<std::string, Value> values)
		: Value(Type::Object)
	{
		for (const auto &pair : values)
			insert(pair.first, pair.second);
	}

	/**
	 * Create an array from a vector.
	 *
	 * @param values the values
	 * @see fromArray
	 */
	inline Value(std::vector<Value> values)
		: Value(Type::Array)
	{
		for (Value value : values)
			append(std::move(value));
	}

	/**
	 * Construct a value from a buffer.
	 *
	 * @param buffer the text
	 * @throw Error on errors
	 */
	Value(const Buffer &buffer);

	/**
	 * Construct a value from a file.
	 *
	 * @param file the file
	 * @throw Error on errors
	 */
	Value(const File &file);

	/**
	 * Move constructor.
	 *
	 * @param other the value to move from
	 */
	inline Value(Value &&other)
	{
		move(std::move(other));
	}

	/**
	 * Copy constructor.
	 *
	 * @param other the value to copy from
	 */
	inline Value(const Value &other)
	{
		copy(other);
	}

	/**
	 * Copy operator.
	 *
	 * @param other the value to copy from
	 * @return *this
	 */
	inline Value &operator=(const Value &other)
	{
		copy(other);

		return *this;
	}

	/**
	 * Move operator.
	 *
	 * @param other the value to move from
	 */
	inline Value &operator=(Value &&other)
	{
		move(std::move(other));

		return *this;
	}

	/**
	 * Destructor.
	 */
	~Value();

	/**
	 * Get an iterator to the beginning.
	 *
	 * @pre must be an array or object
	 * @return the iterator
	 */
	inline iterator begin() noexcept
	{
		assert(isArray() || isObject());

		return m_type == Type::Object ? iterator(*this, m_object.begin()) : iterator(*this, m_array.begin());
	}

	/**
	 * Overloaded function.
	 *
	 * @pre must be an array or object
	 * @return the iterator
	 */
	inline const_iterator begin() const noexcept
	{
		assert(isArray() || isObject());

		return m_type == Type::Object ? const_iterator(*this, m_object.begin()) : const_iterator(*this, m_array.begin());
	}

	/**
	 * Overloaded function.
	 *
	 * @pre must be an array or object
	 * @return the iterator
	 */
	inline const_iterator cbegin() const noexcept
	{
		assert(isArray() || isObject());

		return m_type == Type::Object ? const_iterator(*this, m_object.cbegin()) : const_iterator(*this, m_array.cbegin());
	}

	/**
	 * Get an iterator to the end.
	 *
	 * @pre must be an array or object
	 * @return the iterator
	 */
	inline iterator end() noexcept
	{
		assert(isArray() || isObject());

		return m_type == Type::Object ? iterator(*this, m_object.end()) : iterator(*this, m_array.end());
	}

	/**
	 * Get an iterator to the end.
	 *
	 * @pre must be an array or object
	 * @return the iterator
	 */
	inline const_iterator end() const noexcept
	{
		assert(isArray() || isObject());

		return m_type == Type::Object ? const_iterator(*this, m_object.end()) : const_iterator(*this, m_array.end());
	}

	/**
	 * Get an iterator to the end.
	 *
	 * @pre must be an array or object
	 * @return the iterator
	 */
	inline const_iterator cend() const noexcept
	{
		assert(isArray() || isObject());

		return m_type == Type::Object ? const_iterator(*this, m_object.cend()) : const_iterator(*this, m_array.cend());
	}

	/**
	 * Get the value type.
	 *
	 * @return the type
	 */
	inline Type typeOf() const noexcept
	{
		return m_type;
	}

	/**
	 * Get the value as boolean.
	 *
	 * @return the value or false if not a boolean
	 */
	bool toBool() const noexcept;

	/**
	 * Get the value as integer.
	 *
	 * @return the value or 0 if not a integer
	 */
	int toInt() const noexcept;

	/**
	 * Get the value as real.
	 *
	 * @return the value or 0 if not a real
	 */
	double toReal() const noexcept;

	/**
	 * Get the value as string.
	 *
	 * @param coerce set to true to coerce the value if not a string
	 * @return the value or empty string if not a string
	 */
	std::string toString(bool coerce = false) const noexcept;

	/**
	 * Check if the value is boolean type.
	 *
	 * @return true if boolean
	 */
	inline bool isBool() const noexcept
	{
		return m_type == Type::Boolean;
	}

	/**
	 * Check if the value is integer type.
	 *
	 * @return true if integer
	 */
	inline bool isInt() const noexcept
	{
		return m_type == Type::Int;
	}

	/**
	 * Check if the value is object type.
	 *
	 * @return true if object
	 */
	inline bool isObject() const noexcept
	{
		return m_type == Type::Object;
	}

	/**
	 * Check if the value is array type.
	 *
	 * @return true if array
	 */
	inline bool isArray() const noexcept
	{
		return m_type == Type::Array;
	}

	/**
	 * Check if the value is integer or real type.
	 *
	 * @return true if integer or real
	 * @see toInt
	 * @see toReal
	 */
	inline bool isNumber() const noexcept
	{
		return m_type == Type::Real || m_type == Type::Int;
	}

	/**
	 * Check if the value is real type.
	 *
	 * @return true if real
	 */
	inline bool isReal() const noexcept
	{
		return m_type == Type::Real;
	}

	/**
	 * Check if the value is null type.
	 *
	 * @return true if null
	 */
	inline bool isNull() const noexcept
	{
		return m_type == Type::Null;
	}

	/**
	 * Check if the value is string type.
	 *
	 * @return true if string
	 */
	inline bool isString() const noexcept
	{
		return m_type == Type::String;
	}

	/**
	 * Get the array or object size.
	 *
	 * @pre must be an array or object
	 * @return the size
	 */
	inline unsigned size() const noexcept
	{
		assert(isArray() || isObject());

		if (m_type == Type::Object)
			return m_object.size();

		return m_array.size();
	}

	/**
	 * Remove all the values.
	 *
	 * @pre must be an array or an object
	 */
	inline void clear() noexcept
	{
		assert(isArray() || isObject());

		if (m_type == Type::Array)
			m_array.clear();
		else
			m_object.clear();
	}

	/*
	 * Array functions
	 * ----------------------------------------------------------
	 */

	/**
	 * Get the value at the specified position or the defaultValue if position is out of bounds.
	 *
	 * @param position the position
	 * @param defaultValue the value replacement
	 * @return the value or defaultValue
	 */
	template <typename DefaultValue>
	inline Value valueOr(unsigned position, DefaultValue &&defaultValue) const
	{
		if (m_type != Type::Array || position >= m_array.size())
			return defaultValue;

		return m_array[position];
	}

	/**
	 * Overloaded function with type check.
	 *
	 * @param position the position
	 * @param type the requested type
	 * @param defaultValue the value replacement
	 * @return the value or defaultValue
	 */
	template <typename DefaultValue>
	inline Value valueOr(unsigned position, Type type, DefaultValue &&defaultValue) const
	{
		if (m_type != Type::Array || position >= m_array.size() || m_array[position].typeOf() != type)
			return defaultValue;

		return m_array[position];
	}

	/**
	 * Get a value at the specified index.
	 *
	 * @pre must be an array
	 * @param position the position
	 * @return the value
	 * @throw std::out_of_range if out of bounds
	 */
	inline const Value &at(unsigned position) const
	{
		assert(isArray());

		return m_array.at(position);
	}

	/**
	 * Overloaded function.
	 *
	 * @pre must be an array
	 * @param position the position
	 * @return the value
	 * @throw std::out_of_range if out of bounds
	 */
	inline Value &at(unsigned position)
	{
		assert(isArray());

		return m_array.at(position);
	}

	/**
	 * Get a value at the specified index.
	 *
	 * @pre must be an array
	 * @pre position must be valid
	 * @param position the position
	 * @return the value
	 */
	inline const Value &operator[](unsigned position) const
	{
		assert(isArray());
		assert(position < m_array.size());

		return m_array[position];
	}

	/**
	 * Overloaded function.
	 *
	 * @pre must be an array
	 * @pre position must be valid
	 * @param position the position
	 * @return the value
	 */
	inline Value &operator[](unsigned position)
	{
		assert(isArray());
		assert(position < m_array.size());

		return m_array[position];
	}

	/**
	 * Push a value to the beginning of the array.
	 *
	 * @pre must be an array
	 * @param value the value to push
	 */
	inline void push(const Value &value)
	{
		assert(isArray());

		m_array.insert(m_array.begin(), value);
	}

	/**
	 * Overloaded function.
	 *
	 * @pre must be an array
	 * @param value the value to push
	 */
	inline void push(Value &&value)
	{
		assert(isArray());

		m_array.insert(m_array.begin(), std::move(value));
	}

	/**
	 * Insert a value at the specified position.
	 *
	 * @pre must be an array
	 * @pre position must be valid
	 * @param position the position
	 * @param value the value to push
	 */
	inline void insert(unsigned position, const Value &value)
	{
		assert(isArray());
		assert(position <= m_array.size());

		m_array.insert(m_array.begin() + position, value);
	}

	/**
	 * Overloaded function.
	 *
	 * @pre must be an array
	 * @pre position must be valid
	 * @param position the position
	 * @param value the value to push
	 */
	inline void insert(unsigned position, Value &&value)
	{
		assert(isArray());
		assert(position <= m_array.size());

		m_array.insert(m_array.begin() + position, std::move(value));
	}

	/**
	 * Add a new value to the end.
	 *
	 * @pre must be an array
	 * @param value the value to append
	 */
	inline void append(const Value &value)
	{
		assert(isArray());

		m_array.push_back(value);
	}

	/**
	 * Overloaded function.
	 *
	 * @pre must be an array
	 * @param value the value to append
	 */
	inline void append(Value &&value)
	{
		assert(isArray());

		m_array.push_back(std::move(value));
	}

	/**
	 * Remove a value at the specified position.
	 *
	 * @pre must be an array
	 * @pre position must be valid
	 * @param position the position
	 */
	inline void erase(unsigned position)
	{
		assert(isArray());
		assert(position < m_array.size());

		m_array.erase(m_array.begin() + position);
	}

	/*
	 * Object functions
	 * ----------------------------------------------------------
	 */

	/**
	 * Get the value at the specified key or the defaultValue if key is absent.
	 *
	 * @param name the name
	 * @param defaultValue the value replacement
	 * @return the value or defaultValue
	 */
	template <typename DefaultValue>
	Value valueOr(const std::string &name, DefaultValue &&defaultValue) const
	{
		if (m_type != Type::Object)
			return defaultValue;

		auto it = m_object.find(name);

		if (it == m_object.end())
			return defaultValue;

		return it->second;
	}

	/**
	 * Overloaded function with type check.
	 *
	 * @param name the name
	 * @param type the requested type
	 * @param defaultValue the value replacement
	 * @return the value or defaultValue
	 */
	template <typename DefaultValue>
	Value valueOr(const std::string &name, Type type, DefaultValue &&defaultValue) const
	{
		if (m_type != Type::Object)
			return defaultValue;

		auto it = m_object.find(name);

		if (it == m_object.end() || it->second.typeOf() != type)
			return defaultValue;

		return it->second;
	}

	/**
	 * Get a value from the object.
	 *
	 * @pre must be an object
	 * @param name the value key
	 * @return the value
	 * @throw std::out_of_range if not found
	 */
	inline const Value &at(const std::string &name) const
	{
		assert(isObject());

		return m_object.at(name);
	}

	/**
	 * Overloaded function.
	 *
	 * @pre must be an object
	 * @param name the value key
	 * @return the value
	 * @throw std::out_of_range if not found
	 */
	inline Value &at(const std::string &name)
	{
		assert(isObject());

		return m_object.at(name);
	}

	/**
	 * Get a value from the object.
	 *
	 * @pre must be an object
	 * @param name the value key
	 * @return the value
	 */
	inline Value &operator[](const std::string &name)
	{
		assert(isObject());

		return m_object[name];
	}

	/**
	 * Find a value by key.
	 *
	 * @pre must be an object
	 * @param key the property key
	 * @return the iterator or past the end if not found
	 */
	inline iterator find(const std::string &key)
	{
		assert(isObject());

		return iterator(*this, m_object.find(key));
	}

	/**
	 * Overloaded function.
	 *
	 * @pre must be an object
	 * @param key the property key
	 * @return the iterator or past the end if not found
	 */
	inline const_iterator find(const std::string &key) const
	{
		assert(isObject());

		return const_iterator(*this, m_object.find(key));
	}

	/**
	 * Insert a new value.
	 *
	 * @pre must be an object
	 * @param name the key
	 * @param value the value
	 */
	inline void insert(std::string name, const Value &value)
	{
		assert(isObject());

		m_object.insert({std::move(name), value});
	}

	/**
	 * Overloaded function.
	 *
	 * @pre must be an object
	 * @param name the key
	 * @param value the value
	 */
	inline void insert(std::string name, Value &&value)
	{
		assert(isObject());

		m_object.insert({std::move(name), std::move(value)});
	}

	/**
	 * Check if a value exists.
	 *
	 * @pre must be an object
	 * @param key the key value
	 * @return true if exists
	 */
	inline bool contains(const std::string &key) const noexcept
	{
		assert(isObject());

		return m_object.find(key) != m_object.end();
	}

	/**
	 * Remove a value of the specified key.
	 *
	 * @pre must be an object
	 * @param key the value key
	 */
	inline void erase(const std::string &key)
	{
		assert(isObject());

		m_object.erase(key);
	}

	/**
	 * Return this value as JSon representation.
	 *
	 * @param indent, the indentation to use (0 == compact, < 0 == tabs, > 0 == number of spaces)
	 * @param tabs, use tabs or not
	 * @return the string
	 */
	inline std::string toJson(int indent = 2) const
	{
		return toJson(indent, 0);
	}
};

/**
 * Escape the input.
 *
 * @param input the input
 * @return the escaped string
 */
std::string escape(const std::string &input);

/**
 * Convenient function for creating array from initializer list.
 *
 * @param values the values
 * @return the array
 */
inline Value array(std::initializer_list<Value> values)
{
	return Value(std::vector<Value>(values.begin(), values.end()));
}

/**
 * Convenient function for creating object from initializer list.
 *
 * @param values the values
 * @return the object
 */
inline Value object(std::initializer_list<std::pair<std::string, Value>> values)
{
	return Value(std::map<std::string, Value>(values.begin(), values.end()));
}

} // !json

} // !irccd

#endif // !_JSON_H_
