/*
 * json.cpp -- C++14 JSON manipulation using jansson
 *
 * Copyright (c) 2015-2016 David Demelier <markand@malikania.fr>
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

#include <jansson.h>

#include <sstream>

#include "json.hpp"

namespace irccd {

namespace json {

namespace {

void readObject(Value &parent, json_t *object);
void readArray(Value &parent, json_t *array);

Value readValue(json_t *v)
{
    if (json_is_null(v))
        return Value(nullptr);
    if (json_is_string(v))
        return Value(json_string_value(v));
    if (json_is_real(v))
        return Value(json_number_value(v));
    if (json_is_integer(v))
        return Value(static_cast<int>(json_integer_value(v)));
    if (json_is_boolean(v))
        return Value(json_boolean_value(v));
    if (json_is_object(v)) {
        Value object(Type::Object);

        readObject(object, v);

        return object;
    }
    if (json_is_array(v)) {
        Value array(Type::Array);

        readArray(array, v);

        return array;
    }

    return Value();
}

void readObject(Value &parent, json_t *object)
{
    const char *key;
    json_t *value;

    json_object_foreach(object, key, value)
        parent.insert(key, readValue(value));
}

void readArray(Value &parent, json_t *array)
{
    size_t index;
    json_t *value;

    json_array_foreach(array, index, value)
        parent.append(readValue(value));
}

template <typename Func, typename... Args>
Value convert(Func fn, Args&&... args)
{
    json_error_t error;
    json_t *json = fn(std::forward<Args>(args)..., &error);

    if (json == nullptr)
        throw Error(error.text, error.source, error.line, error.column, error.position);

    Value value;

    if (json_is_object(json)) {
        value = Value(Type::Object);
        readObject(value, json);
    } else {
        value = Value(Type::Array);
        readArray(value, json);
    }

    json_decref(json);

    return value;
}

std::string indent(int param, int level)
{
    std::string str;

    if (param < 0)
        str = std::string(level, '\t');
    else if (param > 0)
        str = std::string(param * level, ' ');

    return str;
}

} // !namespace

void Value::copy(const Value &other)
{
    switch (other.m_type) {
    case Type::Array:
        new (&m_array) std::vector<Value>(other.m_array);
        break;
    case Type::Boolean:
        m_boolean = other.m_boolean;
        break;
    case Type::Int:
        m_integer = other.m_integer;
        break;
    case Type::Object:
        new (&m_object) std::map<std::string, Value>(other.m_object);
        break;
    case Type::Real:
        m_number = other.m_number;
        break;
    case Type::String:
        new (&m_string) std::string(other.m_string);
        break;
    default:
        break;
    }

    m_type = other.m_type;
}

void Value::move(Value &&other)
{
    switch (other.m_type) {
    case Type::Array:
        new (&m_array) std::vector<Value>(std::move(other.m_array));
        break;
    case Type::Boolean:
        m_boolean = other.m_boolean;
        break;
    case Type::Int:
        m_integer = other.m_integer;
        break;
    case Type::Object:
        new (&m_object) std::map<std::string, Value>(std::move(other.m_object));
        break;
    case Type::Real:
        m_number = other.m_number;
        break;
    case Type::String:
        new (&m_string) std::string(std::move(other.m_string));
        break;
    default:
        break;
    }

    m_type = other.m_type;
}

Value::Value(Type type)
    : m_type(type)
{
    switch (m_type) {
    case Type::Array:
        new (&m_array) std::vector<Value>();
        break;
    case Type::Boolean:
        m_boolean = false;
        break;
    case Type::Int:
        m_integer = 0;
        break;
    case Type::Object:
        new (&m_object) std::map<std::string, Value>();
        break;
    case Type::Real:
        m_number = 0;
        break;
    case Type::String:
        new (&m_string) std::string();
        break;
    default:
        break;
    }
}

Value::~Value()
{
    switch (m_type) {
    case Type::Array:
        m_array.~vector<Value>();
        break;
    case Type::Object:
        m_object.~map<std::string, Value>();
        break;
    case Type::String:
        m_string.~basic_string();
        break;
    default:
        break;
    }
}

std::string Value::toString(bool coerce) const
{
    std::string result;

    if (m_type == Type::String)
        result = m_string;
    else if (coerce)
        result = toJson();

    return result;
}

std::string Value::toJson(int level, int current) const
{
    std::ostringstream oss;

    switch (m_type) {
    case Type::Array: {
        oss << '[' << (level != 0 ? "\n" : "");

        unsigned total = m_array.size();
        unsigned i = 0;
        for (const auto &v : m_array) {
            oss << indent(level, current + 1) << v.toJson(level, current + 1);
            oss << (++i < total ? "," : "");
            oss << (level != 0 ? "\n" : "");
        }

        oss << (level != 0 ? indent(level, current) : "") << ']';
        break;
    }
    case Type::Boolean:
        oss << (m_boolean ? "true" : "false");
        break;
    case Type::Int:
        oss << m_integer;
        break;
    case Type::Null:
        oss << "null";
        break;
    case Type::Object: {
        oss << '{' << (level != 0 ? "\n" : "");

        unsigned total = m_object.size();
        unsigned i = 0;
        for (const auto &pair : m_object) {
            oss << indent(level, current + 1);

            // Key and :.
            oss << "\"" << pair.first << "\":" << (level != 0 ? " " : "");

            // Value.
            oss << pair.second.toJson(level, current + 1);

            // Comma, new line if needed.
            oss << (++i < total ? "," : "") << (level != 0 ? "\n" : "");
        }

        oss << (level != 0 ? indent(level, current) : "") << '}';
        break;
    }
    case Type::Real:
        oss << m_number;
        break;
    case Type::String:
        oss << "\"" << escape(m_string) << "\"";
        break;
    default:
        break;
    }

    return oss.str();
}

std::string escape(const std::string &value)
{
    std::string result;

    for (auto it = value.begin(); it != value.end(); ++it) {
        switch (*it) {
        case '\\':
            result += "\\\\";
            break;
        case '/':
            result += "\\/";
            break;
        case '"':
            result += "\\\"";
            break;
        case '\b':
            result += "\\b";
            break;
        case '\f':
            result += "\\f";
            break;
        case '\n':
            result += "\\n";
            break;
        case '\r':
            result += "\\r";
            break;
        case '\t':
            result += "\\t";
            break;
        default:
            result += *it;
            break;
        }
    }

    return result;
}

Value fromString(const std::string &buffer)
{
    return convert(json_loads, buffer.c_str(), 0);
}

Value fromFile(const std::string &path)
{
    return convert(json_load_file, path.c_str(), 0);
}

} // !json

} // !irccd
