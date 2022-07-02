# 8-1 Object Serialization

This is a library for serializing and deserializing objects written in C++.

The reason of implementing this library in a header-files-only manner will be explained later.


## Compile & Run

```bash
$ ./test.sh
```

Or you can compile it manually:

```bash
$ cmake .
$ make -j
```

Use `make clean` to clean previously built files.

Note that this library requires a compiler that supports (at least) C++17 to compile.


## Highlights

- Of course, basic requirements are fulfilled
- Bonus: XML serialization with `base64` encoding
- Anonymous `namespace`s to hide internally used classes & functions from users
- Using various type gymnastics (类型体操) to implement polymorphism and strong compile-time type checks


### Polymorphism

With polymorphism, we can write less codes and support more types.

- For xml, `struct XMLSerializable`: base class for all serializable objects, with pure virtual functions to be overridden
- For binary, provide additional {de,}serializer to `serializer::binary::serialize` to support user-defined types
- `std::map`-like objects are supported, such as `std::unordered_map` or user-implemented maps
  - handled by same codes as `std::map`
  - _`std::map`-like_ is defined as: `T::key_type`, `T::mapped_type`, as well as `operator[]` that accepts `T::key_type` and returns `T&`
- `std::pair`-like objects are supported in a similar way.
- `std::vector` and `std::list`'s {de,}serialization are handled by same codes.
- Additional container supports for `std::tuple`. Type checks for `std::tuple` are done by recursively iterating over the tuple at compile time.


### Compile-Time Type Checks

With compile-time type checks, we can discover bugs at compile time. Usually, compile logs provide more helpful information to find the cause of the error. For instance:

```
./include/libbinary.h:197:16:   required from ‘void serializer::binary::serialize(const T&, const string&) [with T = main()::Example; std::string = std::__cxx11::basic_string<char>]’
./tests/test_binary.cpp:107:26:   required from here
./include/libbinary.h:183:44: error: ‘constexpr bool serializer::impossible_error(T&&, const char*) [with T = const main()::Example&]’ called in a constant expression
  183 |         constexpr auto x = impossible_error(t, "T is not a supported type, you must provide a serialize function");
```

By reading this log, we know the input type `T`, the reason of failing (`T is not a supported type`), and the location of the error. Without proper type checks, we can only discover this error at runtime, not knowing the type `T`, and some valuable information might be lost due to inlining & other optimizations. What's more, if the tests failed to cover all the cases, this runtime error might not be triggered, leaving a flaw inside the library.

Here is a list of type traits & techniques that I've used to implement compile-time type checks:

- `std::enable_if_t`, `std::is_arithmetic_v`, `std::is_base_of`, `std::is_same_t`
- `std::remove_cv_t`, `std::remove_volatile_t`, `std::void_t`, `std::true_type`, `std::false_type`
- `std::declval`, `decltype`
- `std::decay_t`, `std::is_specialization_of` (this is way toooooo new, it's still a proposal, so I have to implement a shim)
- `if constexpr`
  - `static_assert`
  - A manually implemented `constexpr bool impossible_error` to replace `static_assert(false)` in the `else` branch of `if constexpr`-s, which will also be explained later

For implementation details, see `include/type_utils.h`.


### Run-Time Checks

Some runtime checks are not avoidable, acting as the last layer of safe guards to make sure that the program does not act in undefined behaviors. For instance, we need to check that files are correctly opened and are succesfully read into the memory, and that the input is valid. We also want to check that strings are successfully written into files, etc.

However, due to the nature of the language, unexpected errors will definitely exist. Like, you can modify the XML files directly, and if you are deserializing some values into raw pointers, you might get a segfault caused by out-of-bound memory accesses.


## Additional Notes

- Just don't seprate template functions' declaration and definition. Keep them inside one Translation Unit (TU), or you'll have to explicitly instantiate them. That's too annoying.
- `remove_cv_t` will not change `const char*` to `char *`. It will actually change `char const*` to `char *`.
- `static_assert(false)`, even provided inside the `else` branch of `if constexpr`, causes ill-formed NDR.
- gcc's implementation of `std::to_chars` is partial. It only works for integral types. clang also doesn't support `std::to_chars` of floating-point types until LLVM 14.0. So we have to use `std::stringstream`.
- Set `std::stringstream`'s precision to `std::numeric_limits<T>::max_digits10` instead of `std::numeric_limits<T>::digits10`. The latter causes rounding errors for some input.
- `std::variant` is tricky. `std::variant<std::vector<T>, std::list<T>>` as a function parameter's type will compile, but this won't work. It matches neither `std::vector<T>` nor `std::list<T>`. Actually, `std::variant` is not designed to be used this way.
- `std::span` does not support `std::list`, because `std::list`'s data is not contiguous.
- We can match `std::pair` with `T::first_type` and `T::second_type`. Although it is said that `std::pair` is a specialization of `std::tuple`, `std::tuple` does not have `first_type` and `second_type` members.
- `std::make_index_sequence` can also be used to iterate through a tuple at compile time, but I failed to make it work.
- It's too hard to serialize user-defined structs into XML with the original structure preserved (_i.e._ directly mapped to XML's tree structure). Currently, nested structs are flattened when serializing to XML, result in many `&lt;` and `&gt;` in the generated XML. Escaping of inner structs' serialized XML can be avoided if the user is able to provide a iterator for the input struct (and its inner structs), but that's not considered as _a convenient mechanism_.
