#pragma once

#include <string>
#include <vector>
#include <list>
#include <variant>
#include <initializer_list>
#include <type_traits>

namespace serializer {
  // anonymous namespace for private-like helper functions, i.e. _debug
  namespace {
    static const bool is_debug = false;
    template<typename T>
    void _debug(T t, bool newline = true) {
      if (is_debug) {
        std::cout << t;
        if (newline) {
          std::cout << std::endl;
        } else {
          std::cout << " ";
        }
      }
    }
  }
  
  // Check if a type is an array-like container (namely, std::vector or std::list).
  namespace {
    // fallback struct:
    template<class T>
    struct A {
      static constexpr bool v = 0;
    };
    // This struct will be matched if T is an array-like container.
    template<typename T, class Alloc, template<typename CT, typename CAlloc> class Container>
    struct A<Container<T, Alloc>> {
      using prob_t = Container<T, Alloc>;
      static constexpr bool v = std::is_same_v<prob_t, std::list<T, Alloc>> || std::is_same_v<prob_t, std::vector<T, Alloc>>;
    };
  }
  template<typename T>
  constexpr auto is_array_container_v = A<T>::v;


  // Check if a type is a map-like container. That is, any container with key_type and mapped_type
  // inferable from std::pair and supports operator[] is accepted.
  // See also: https://en.cppreference.com/w/cpp/container/map
  namespace {
    // fallback struct:
    template<typename T, typename U = void>
    struct M {
      static constexpr bool v = false;
    };
    template<typename T>
    struct M<T, std::void_t<
      typename T::key_type,     // key_type
      typename T::mapped_type,  // mapped_type
      decltype(std::declval<T&>()[std::declval<const typename T::key_type&>()])  // operator[]
    >> {
      static constexpr bool v = true;
    };
  }
  template<typename T>
  constexpr auto is_map_container_v = M<T>::v;


  // Check if a type is a set container.
  namespace {
    // fallback struct:
    template<class T>
    struct S {
      static constexpr bool v = 0;
    };
    // This struct will be matched if T is a set container.
    template<typename T, class Alloc, template<typename CT, typename CAlloc> class Container>
    struct S<Container<T, Alloc>> {
      using prob_t = Container<T, Alloc>;
      static constexpr bool v = std::is_same_v<prob_t, std::set<T, Alloc>>;
    };
  }
  template<typename T>
  constexpr auto is_set_container_v = S<T>::v;

  
  // Check if a type is a supported container (namely, is_array_containver_v or map or set).
  namespace {
    // fallback struct:
    template<class T>
    struct C {
      static constexpr bool v = false;
    };
    // We have to use C<Container<T>> instead of C<T> to expose the inner type T to the template.
    template<typename T, class Alloc, template<typename CT, typename CAlloc> class Container>
    struct C<Container<T, Alloc>> {
      // Here, we reconstruct the container type.
      using prob_t = Container<T, Alloc>;
      static constexpr bool v = is_array_container_v<prob_t> || is_map_container_v<prob_t> || is_set_container_v<prob_t>;
    };
  }
  template<typename T>
  constexpr auto is_supported_container_v = C<T>::v;

  template<typename T>
  constexpr auto is_supported_v = is_supported_container_v<T>    ||
                                  std::is_same_v<T, std::string> ||
                                  std::is_same_v<T, const char*> ||
                                  std::is_arithmetic_v<T>;
}
