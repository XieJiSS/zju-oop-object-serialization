#pragma once

#include <cstring>
#include <fstream>
#include <initializer_list>
#include <iostream>
#include <list>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

#include "common.h"
#include "type_utils.h"

using std::is_base_of_v;
using std::is_same_v;
using std::remove_cv_t;
using std::string;

// Here, we're writing the declaration and the definition together, in a header file,
// so that we can make sure the definition is always in the same translation unit with
// the actual code that uses it.
// By doing so, we can make use of template function's implicit instantiation mechanism,
// without having to explicitly instantiate every type that we want to support.
// See also: https://en.cppreference.com/w/cpp/language/function_template#Implicit_instantiation

namespace serializer {
  namespace binary {
    struct BinSerializable {
      virtual string serializeToString() const = 0;
      virtual void deserializeFromString(const string &str) = 0;
    };

    // anonymous namespace for private-like functions that should not be exposed to the user
    namespace {
      // declarations
      template <typename T>
      void _write(std::ostream &os, const T &t, size_t size);
      // Note that we could have declare this as std::enable_if_t<std::is_arithmetic_v<T>, void>
      // to protect this generic function from being called with container types that can't be
      // casted to const char * directly, but since we need to handle const char * itself, the
      // is_arithmetic_v constraint is not suitable. Instead, we use static_assert to check
      // incorrectly matched function calls at compile time to ensure that the final _write call
      // won't produce useless nonsense data.
      template <typename T>
      void _write(std::ostream &os, const T &t);
      void _write(std::ostream &os, const std::string &s);

      template <typename T>
      void _read(std::istream &is, T &t);
      // No need to pass in the str's size, since we've stored the size of the string we're reading.
      void _read(std::istream &is, std::string &str);

      // definitions
      template <typename T>
      void _write(std::ostream &os, const T &t, size_t size) {
        _debug("_write(std::ostream& os, const T& t, size_t size)");
        static_assert(!is_array_container_v<T>, "T must not be a container");
        // strings are handled separately
        static_assert(!is_same_v<remove_cv_t<T>, std::string>, "T must not be a string");
        os.write(reinterpret_cast<const char *>(&size), sizeof(size));
        if constexpr (std::is_pointer_v<T>) {
          os.write(reinterpret_cast<const char *>(t), size);
        } else {
          os.write(reinterpret_cast<const char *>(&t), size);
        }
        // Check that the write succeeded.
        ASSERT(os.good());
      }
      template <typename T>
      void _write(std::ostream &os, const T &t) {
        _debug("_write(std::ostream& os, const T& t)");
        static_assert(!is_array_container_v<T>, "T must not be a container");
        static_assert(!is_same_v<remove_cv_t<T>, std::string>, "T must not be a string");
        const size_t size = sizeof(t);
        os.write(reinterpret_cast<const char *>(&size), sizeof(size));
        os.write(reinterpret_cast<const char *>(&t), size);
      }
      inline void _write(std::ostream &os, const std::string &s) {
        _debug("_write(std::ostream& os, const std::string& s)");
        // we can't use sizeof(s.c_str()), because the string might contain null characters
        size_t size = s.size();
        _write(os, s.c_str(), size);
      }

      // If T is a pointer type, then t should be pre-allocated.
      template <typename T>
      void _read(std::istream &is, T &t) {
        _debug("_read(std::istream& is, T& t)");
        static_assert(!is_array_container_v<T>, "T must not be a container");
        static_assert(!is_same_v<remove_cv_t<T>, std::string>, "T must not be a string");
        size_t size;
        is.read(reinterpret_cast<char *>(&size), sizeof(size));
        if constexpr (std::is_pointer_v<T>) {
          is.read(reinterpret_cast<char *>(t), size);
        } else {
          is.read(reinterpret_cast<char *>(&t), size);
        }
      }
      inline void _read(std::istream &is, std::string &str) {
        _debug("_read(std::istream& is, std::string& str)");
        size_t size;
        is.read(reinterpret_cast<char *>(&size), sizeof(size));
        char *c_str = new char[size]; // +1 for null terminator
        is.read(c_str, size);
        ASSERT(is.good());
        str = string(c_str, size);
        delete[] c_str;
      }
    } // namespace

    // declarations
    template <typename T>
    void serialize(const T &t, std::ostream &os);
    template <typename T>
    void serialize(const T &t, const string &file_name);

    template <typename T>
    void deserialize(T &t, std::istream &is);
    template <typename T>
    void deserialize(T &t, const string &file_name);

    // definitions
    template <typename T>
    void serialize(const T &t, std::ostream &os) {
      _debug("serialize(const T& t, std::ostream& os)");
      if constexpr (is_supported_container_v<T>) {
        _debug("is_supported_container_v<T>");
        if constexpr (is_pair_v<T>) {
          _debug("serialize: is_pair_v<T>");
          serialize(t.first, os);
          serialize(t.second, os);
        } else if constexpr (is_array_container_v<T>) {
          _debug("serialize: is_array_container_v<T>");
          size_t size = t.size();
          _write(os, size, sizeof(size));
          for (const auto &elem : t) {
            serialize(elem, os);
          }
        } else if constexpr (is_tuple_v<T>) {
          _debug("serialize: is_tuple_v<T>");
          constexpr size_t size = std::tuple_size_v<T>;
          _write(os, size, sizeof(size));
          // Here we use foreach_in_tuple to iterate over the elements of the tuple at
          // compile time, since std::get<i> is constexpr after C++14.
          foreach_in_tuple(t, [&](const auto &elem, auto) { serialize(elem, os); });
        } else if constexpr (is_map_container_v<T>) {
          _debug("serialize: is_map_container_v<T>");
          size_t size = t.size();
          _write(os, size, sizeof(size));
          for (const auto &elem : t) {
            serialize(elem.first, os);
            serialize(elem.second, os);
          }
        } else if constexpr (is_set_container_v<T>) {
          _debug("serialize: is_set_container_v<T>");
          size_t size = t.size();
          _write(os, size, sizeof(size));
          for (const auto &elem : t) {
            serialize(elem, os);
          }
        } else {
          static_assert(always_false<T>, "T is a supported container type, but it's serializer is missing.");
        }
      } else if constexpr (is_supported_literal_v<T>) {
        if constexpr (is_cstring_v<T>) {
          // Storing char* as string to make life easier.
          serialize(string(t), os);
        } else {
          _write(os, t);
        }
      } else if constexpr (is_base_of_v<BinSerializable, remove_cv_t<T>>) {
        _debug("serialize: is_base_of_v<BinSerializable, remove_cv_t<T>>");
        string s = t.serializeToString();
        serialize(s, os);
      } else {
        static_assert(always_false<T>, "T is not a supported type, you must provide a serialize function");
      }
    }
    template <typename T>
    void serialize(const T &t, const string &file_name) {
      _debug("serialize(const T& t, const string &file_name)");
      std::ofstream os(file_name, std::ios::binary);
      // Check if file is opened successfully
      ASSERT(os.good());
      serialize(t, os);
      os.close();
    }

    template <typename T>
    void deserialize(T &t, std::istream &is) {
      _debug("deserialize(T& t, std::istream& is)");
      if constexpr (is_supported_container_v<T>) {
        _debug("is_supported_container_v<T>");
        if constexpr (is_pair_v<T>) {
          _debug("deserialize: is_pair_v<T>");
          typename T::first_type first;
          typename T::second_type second;
          deserialize(first, is);
          deserialize(second, is);
          t = std::make_pair(first, second);
        } else if constexpr (is_array_container_v<T>) {
          _debug("deserialize: is_array_container_v<T>");
          size_t size;
          _read(is, size);
          _debug("deserialize: resizing to " + std::to_string(size));
          t.resize(size);
          for (auto &elem : t) {
            // here we use the reference to the element in the container
            // because std::list does not support operator[]
            deserialize(elem, is);
          }
        } else if constexpr (is_tuple_v<T>) {
          _debug("deserialize: is_tuple_v<T>");
          size_t size = std::tuple_size_v<T>;
          _read(is, size);
          ASSERT(size == std::tuple_size_v<T>);
          // Here we use foreach_in_tuple to iterate over the elements of the tuple at
          // compile time, since std::get<i> is constexpr after C++14.
          foreach_in_tuple(t, [&](auto &elem, auto) { deserialize(elem, is); });
        } else if constexpr (is_map_container_v<T>) {
          _debug("deserialize: is_map_container_v<T>");
          size_t size;
          _read(is, size);
          for (size_t i = 0; i < size; ++i) {
            typename T::key_type key;
            typename T::mapped_type value;
            deserialize(key, is);
            deserialize(value, is);
            t.insert(std::make_pair(key, value));
          }
        } else if constexpr (is_set_container_v<T>) {
          _debug("deserialize: is_set_container_v<T>");
          size_t size;
          _read(is, size);
          for (size_t i = 0; i < size; ++i) {
            typename T::value_type value;
            deserialize(value, is);
            t.insert(value);
          }
        } else {
          static_assert(always_false<T>, "T is a supported container type, but it's serializer is missing.");
        }
      } else if constexpr (is_supported_literal_v<T>) {
        if constexpr (is_cstring_v<T>) {
          // Reading a string in this case, because we are storing char* as strings.
          string s;
          deserialize(s, is);
          // The user should preallocate enough spaces for C-style strings.
          memcpy(t, s.c_str(), s.size());
        } else {
          _read(is, t);
        }
      } else if constexpr (is_base_of_v<BinSerializable, remove_cv_t<T>>) {
        _debug("deserialize: is_base_of_v<BinSerializable, remove_cv_t<T>>");
        string s;
        deserialize(s, is);
        t.deserializeFromString(s);
      } else {
        static_assert(always_false<T>, "T is not a supported type, you must provide a deserialize function");
      }
    }
    template <typename T>
    void deserialize(T &t, const string &file_name) {
      _debug("deserialize(T& t, const string &file_name)");
      std::ifstream is(file_name, std::ios::binary);
      // Check if file exists
      ASSERT(is.good());
      deserialize(t, is);
      is.close();
    }
  } // namespace binary
} // namespace serializer
