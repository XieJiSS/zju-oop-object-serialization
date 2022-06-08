#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <initializer_list>
#include <type_traits>
#include <list>
#include <map>
#include <unordered_map>
#include <set>
#include <tuple>

using std::string;
using std::remove_cv_t;

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

  namespace {
    // Modified from https://stackoverflow.com/a/16397153/8553479

    // We'd like to fail the compilation if i > sizeof...(Tp), because this indicates a
    // coding error, and should not be silently ignored. So we use i == sizeof...(Tp)
    // instead of i >= sizeof...(Tp) for the final step.

    // final step: stop the recursion
    template<size_t i = 0, typename Func, typename... Tp>
    typename std::enable_if_t<i == sizeof...(Tp), void> foreach_in_tuple(std::tuple<Tp...>&, Func) {}
    // recursive step: call the function on each element of the tuple
    template<size_t i = 0, typename Func, typename... Tp>
    typename std::enable_if_t<i < sizeof...(Tp), void> foreach_in_tuple(std::tuple<Tp...>& t, Func f) {
      f(std::get<i>(t), i);
      foreach_in_tuple<i + 1, Func, Tp...>(t, f);
    }
    
    // final step: stop the recursion
    template<size_t i = 0, typename Func, typename... Tp>
    typename std::enable_if_t<i == sizeof...(Tp), void> foreach_in_tuple(const std::tuple<Tp...>&, Func) {}
    // recursive step: call the function on each element of the tuple
    template<size_t i = 0, typename Func, typename... Tp>
    typename std::enable_if_t<i < sizeof...(Tp), void> foreach_in_tuple(const std::tuple<Tp...>& t, Func f) {
      f(std::get<i>(t), i);
      foreach_in_tuple<i + 1, Func, Tp...>(t, f);
    }
  }

  // Check if a type is a std::tuple
  namespace {
    // I learned this method of determining types after implementing several type checkers
    // with the destruct-then-construct method. Obviously the following way is easier to
    // understand, but the idiom it uses is still a proposal of the C++ standard, and this
    // method also does not support *-like containers.
    // Say, we'd like to match std::pair-like types by checking they have first_type and
    // second_type typedefs. But if we use this approach, we can only match std::pair<int, int>.
    // So I'm not really sure if it is a good idea to replace those harder-to-understand
    // type checkers with this one. Leaving them as-is for now.
    // See also: https://stackoverflow.com/questions/12919310/c-detect-templated-class
    template<typename T, template <typename...> class Template>
    struct is_specialization_of_shim : std::false_type {};

    template<template <typename...> class Template, typename... Args>
    struct is_specialization_of_shim<Template<Args...>, Template> : std::true_type {};

    template<typename T>
    struct _is_tuple : is_specialization_of_shim<typename std::decay_t<T>, std::tuple> {};

    template<typename T>
    constexpr bool is_tuple_v = _is_tuple<remove_cv_t<T>>::value;
  }

  // Check if a type is a std::pair-like type, i.e. has a first and second member
  namespace {
    // fallback struct:
    template<typename T, typename U = void>
    struct P {
      static constexpr bool v = false;
    };
    template<typename T>
    struct P<T, std::void_t<
      typename T::first_type,  // first_type
      typename T::second_type  // second_type
    >> {
      // tuples might also have first_type and second_type, but we don't want to
      // treat them as std::pair.
      static constexpr bool v = !is_tuple_v<remove_cv_t<T>>;
    };
  }
  template<typename T>
  constexpr auto is_pair_v = P<remove_cv_t<T>>::v;
  
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
  constexpr auto is_array_container_v = A<remove_cv_t<T>>::v;


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
  constexpr auto is_map_container_v = M<remove_cv_t<T>>::v;


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
  constexpr auto is_set_container_v = S<remove_cv_t<T>>::v;

  
  // Check if a type is a supported container.
  namespace {
    // fallback struct:
    template<class T>
    struct C {
      static constexpr bool v = false;
    };
    // We use C<Container<T>> instead of C<T> to expose the inner type T to the template.
    template<typename T, class Alloc, template<typename CT, typename CAlloc> class Container>
    struct C<Container<T, Alloc>> {
      // Here, we reconstruct the container type.
      using prob_t = Container<T, Alloc>;
      static constexpr bool v = is_pair_v<prob_t>                ||
                                is_array_container_v<prob_t>     ||
                                is_map_container_v<prob_t>       ||
                                is_set_container_v<prob_t>;
    };
  }
  // Here, we have to pick is_tuple_v<T> out of the struct C<Container<T, Alloc>>,
  // because it does not work with the reconstructed container type prob_t.
  template<typename T>
  constexpr auto is_supported_container_v = C<remove_cv_t<T>>::v || is_tuple_v<remove_cv_t<T>>;

  // we don't want to treat char* const as cstring, so we should not wrap T with remove_cv_t.
  template<typename T>
  constexpr auto is_cstring_v = std::is_same_v<T, char*> ||
                                std::is_same_v<T, const char*>;
  
  template<typename T>
  constexpr auto is_string_cstring_v = std::is_same_v<remove_cv_t<T>, string> ||
                                       is_cstring_v<T>;  // ditto, no remove_cv_t.

  template<typename T>
  constexpr auto is_supported_v = is_supported_container_v<remove_cv_t<T>> ||
                                  is_string_cstring_v<remove_cv_t<T>>      ||
                                  std::is_arithmetic_v<T>;  // is_arithmetic_v has remove_cv_t inside.

  template<typename T>
  constexpr auto is_supported_literal_v = is_supported_v<remove_cv_t<T>> &&
                                          !is_supported_container_v<remove_cv_t<T>>;

  // We cannot use static_assert(false), because it is an ill-formed NDR. This is a workaround.
  // The T&& is needed to make the function constexpr.
  // See also: C++ standard [temp.res]/8.
  // See also: https://stackoverflow.com/a/14637372/8553479
  // See also: https://lists.isocpp.org/std-discussion/2021/01/0965.php
  template<typename T>
  constexpr bool impossible_error(T&&, const char* msg) {
    throw msg;
  }
}
