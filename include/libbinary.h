#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <map>
#include <set>
#include <vector>
#include <list>
#include <variant>
#include <initializer_list>
#include <type_traits>
#include <stdexcept>

#include "type_utils.h"

#define ASSERT(a) \
  do { \
    if (!(a)) { \
      throw std::runtime_error("Assertion failed: " #a); \
    } \
  } while(0);

using std::string;
using std::is_same_v;

// Here, we're writing the declaration and the definition together, in a header file,
// so that we can make sure the definition is always in the same translation unit with
// the actual code that uses it.
// By doing so, we can make use of template function's implicit instantiation mechanism,
// without having to explicitly instantiate every type that we want to support.
// See also: https://en.cppreference.com/w/cpp/language/function_template#Implicit_instantiation

namespace serializer {
  namespace binary {
    // anonymous namespace for private-like functions that should not be exposed to the user
    namespace {
      // declarations
      template<typename T>
      void _write(std::ostream& os, const T& t, size_t size);
      // Note that we could have declare this as std::enable_if_t<std::is_arithmetic_v<T>, void>
      // to protect this generic function from being called with container types that can't be
      // casted to const char * directly, but since we need to handle const char * itself, the
      // is_arithmetic_v constraint is not suitable. Instead, we use static_assert to check
      // incorrectly matched function calls at compile time to ensure that the final _write call
      // won't produce useless nonsense data.
      template<typename T>
      void _write(std::ostream& os, const T& t);
      void _write(std::ostream& os, const std::string& s);

      template<typename T>
      void _read(std::istream& is, T& t);
      // No need to pass in the str's size, since we've stored the size of the string we're reading.
      void _read(std::istream& is, std::string& str);

      // definitions
      template<typename T>
      void _write(std::ostream& os, const T& t, size_t size) {
        _debug("_write(std::ostream& os, const T& t, size_t size)");
        static_assert(!is_array_container_v<T>, "T must not be a container");
        // strings are handled separately
        static_assert(!is_same_v<T, std::string>, "T must not be a string");
        os.write(reinterpret_cast<const char*>(&size), sizeof(size));
        if constexpr (std::is_pointer_v<T>) {
          os.write(reinterpret_cast<const char*>(t), size);
        } else {
          os.write(reinterpret_cast<const char*>(&t), size);
        }
        // Check that the write succeeded.
        ASSERT(os.good());
      }
      template<typename T>
      void _write(std::ostream& os, const T& t) {
        _debug("_write(std::ostream& os, const T& t)");
        static_assert(!is_array_container_v<T>, "T must not be a container");
        static_assert(!is_same_v<T, std::string>, "T must not be a string");
        const size_t size = sizeof(t);
        os.write(reinterpret_cast<const char*>(&size), sizeof(size));
        os.write(reinterpret_cast<const char*>(&t), size);
      }
      void _write(std::ostream& os, const std::string& s) {
        _debug("_write(std::ostream& os, const std::string& s)");
        // we can't use sizeof(s.c_str()), because the string might contain null characters
        size_t size = s.size();
        _write(os, s.c_str(), size);
      }

      // If T is a pointer type, then t should be pre-allocated.
      template<typename T>
      void _read(std::istream& is, T& t) {
        _debug("_read(std::istream& is, T& t)");
        static_assert(!is_array_container_v<T>, "T must not be a container");
        static_assert(!is_same_v<T, std::string>, "T must not be a string");
        size_t size;
        is.read(reinterpret_cast<char*>(&size), sizeof(size));
        if constexpr (std::is_pointer_v<T>) {
          is.read(reinterpret_cast<char *>(t), size);
        } else {
          is.read(reinterpret_cast<char *>(&t), size);
        }
      }
      void _read(std::istream& is, std::string& str) {
        _debug("_read(std::istream& is, std::string& str)");
        size_t size;
        is.read(reinterpret_cast<char*>(&size), sizeof(size));
        char* c_str = new char[size];  // +1 for null terminator
        is.read(c_str, size);
        ASSERT(is.good());
        str = string(c_str, size);
        delete[] c_str;
      }
    }
    
    // declarations
    template<typename T>
    void serialize(const T& t, std::ostream& os);
    // support user-defined serialize function for custom types
    template<typename T>
    void serialize(const T& t, std::ostream& os, string (*f)(const T&));
    template<typename T>
    void serialize(const T& t, const string &file_name);
    template<typename T>
    void serialize(const T& t, const string &file_name, string (*f)(const T&));
    
    template<typename T>
    void deserialize(T& t, std::istream& is);
    // support user-defined deserialize function for custom types
    template<typename T>
    void deserialize(T& t, std::istream& is, void (*f)(T&, const string&));
    template<typename T>
    void deserialize(T& t, const string &file_name);
    template<typename T>
    void deserialize(T& t, const string &file_name, void (*f)(T&, const string&));

    // definitions
    template<typename T>
    void serialize(const T& t, std::ostream& os) {
      _debug("serialize(const T& t, std::ostream& os)");
      if constexpr (is_supported_container_v<T>) {
        _debug("is_supported_container_v<T>");
        size_t size = t.size();
        _write(os, size, sizeof(size));
        if constexpr (is_array_container_v<T>) {
          _debug("serialize: is_array_container_v<T>");
          for (const auto& elem : t) {
            serialize(elem, os);
          }
        } else if constexpr (is_map_container_v<T>) {
          _debug("serialize: is_map_container_v<T>");
          for (const auto& elem : t) {
            serialize(elem.first, os);
            serialize(elem.second, os);
          }
        } else if constexpr (is_set_container_v<T>) {
          _debug("serialize: is_set_container_v<T>");
          for (const auto& elem : t) {
            serialize(elem, os);
          }
        } else {
          std::cerr << "serialize: unreachable code reached" << std::endl;
        }
      } else if constexpr (is_same_v<T, string> || is_same_v<T, const char*> || std::is_arithmetic_v<T>) {
        _write(os, t);
      } else {
        static_assert(is_supported_v<T>, "T is not a supported type, you must provide a serialize function");
      }
    }
    template<typename T>
    void serialize(const T& t, std::ostream& os, string (*f)(const T&)) {
      _debug("serialize(const T& t, std::ostream& os, string (*f)(const T&))");
      _write(os, f(t));
    }
    template<typename T>
    void serialize(const T& t, const string &file_name) {
      _debug("serialize(const T& t, const string &file_name)");
      std::ofstream os(file_name, std::ios::binary);
      // Check if file is opened successfully
      ASSERT(os.good());
      serialize(t, os);
      os.close();
    }
    template<typename T>
    void serialize(const T& t, const string &file_name, string (*f)(const T&)) {
      _debug("serialize(const T& t, const string &file_name, string (*f)(const T&))");
      std::ofstream os(file_name, std::ios::binary);
      // Check if file is opened successfully
      ASSERT(os.good());
      serialize(t, os, f);
      os.close();
    }

    template<typename T>
    void deserialize(T& t, std::istream& is) {
      _debug("deserialize(T& t, std::istream& is)");
      if constexpr (is_supported_container_v<T>) {
        _debug("is_supported_container_v<T>");
        size_t size;
        _read(is, size);
        if constexpr (is_array_container_v<T>) {
          _debug("deserialize: is_array_container_v<T>");
          t.resize(size);
          _debug("deserialize: resizing to " + std::to_string(size));
          for (auto& elem : t) {
            // here we use the reference to the element in the container
            // because std::list does not support operator[]
            deserialize(elem, is);
          }
        } else if constexpr (is_map_container_v<T>) {
          _debug("deserialize: is_map_container_v<T>");
          for (size_t i = 0; i < size; ++i) {
            typename T::key_type key;
            typename T::mapped_type value;
            deserialize(key, is);
            deserialize(value, is);
            t.insert(std::make_pair(key, value));
          }
        } else if constexpr (is_set_container_v<T>) {
          _debug("deserialize: is_set_container_v<T>");
          for (size_t i = 0; i < size; ++i) {
            typename T::value_type value;
            deserialize(value, is);
            t.insert(value);
          }
        } else {
          static_assert(is_array_container_v<T> || is_map_container_v<T> || is_set_container_v<T>, "unreachable code reached");
        }
      } else if constexpr (is_same_v<T, string> || is_same_v<T, const char*> || std::is_arithmetic_v<T>) {
        _read(is, t);
      } else {
        static_assert(is_supported_v<T>, "T is not a supported type, you must provide a deserialize function");
      }
    }
    template<typename T>
    void deserialize(T& t, std::istream& is, void (*f)(T&, const string&)) {
      _debug("deserialize(T& t, std::istream& is, void (*f)(T&, const string&))");
      string str;
      _read(is, str);
      f(t, str);
    }
    template<typename T>
    void deserialize(T& t, const string &file_name) {
      _debug("deserialize(T& t, const string &file_name)");
      std::ifstream is(file_name, std::ios::binary);
      // Check if file exists
      ASSERT(is.good());
      deserialize(t, is);
      is.close();
    }
    template<typename T>
    void deserialize(T& t, const string &file_name, void (*f)(T&, const string&)) {
      _debug("deserialize(T& t, const string &file_name, void (*f)(T&, const string&))");
      std::ifstream is(file_name, std::ios::binary);
      // Check if file exists
      ASSERT(is.good());
      deserialize(t, is, f);
      is.close();
    }
  } // namespace binary
} // namespace serialize
