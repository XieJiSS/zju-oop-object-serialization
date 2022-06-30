#pragma once

#include "thirdparty/tinyxml2.h"
#include "thirdparty/base64.h"
#include "type_utils.h"
#include "common.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <charconv>
#include <map>
#include <set>
#include <vector>
#include <list>
#include <variant>
#include <initializer_list>
#include <type_traits>
#include <stdexcept>

using std::vector;
using std::string;
using std::is_same_v;
using std::is_base_of_v;
using std::remove_cv_t;

namespace serializer {
  namespace xml {
    struct XMLSerializable {
      virtual vector<string> serializeToXML() const = 0;
      virtual void deserializeFromXML(const vector<string>& strings) = 0;
    };

    // anonymous namespace for private-like helper functions
    namespace {
      using namespace tinyxml2;

      template<typename T>
      typename std::enable_if_t<is_supported_literal_v<T>, string>
      serialize_to_literal(const T &t);
      template<typename T>
      typename std::enable_if_t<is_supported_literal_v<T>, T>
      deserialize_from_literal(const string &s);
      template<typename T>
      typename std::enable_if_t<is_supported_literal_v<T>, string>
      to_string_value(const T& t);
      template<typename T>
      typename std::enable_if_t<is_supported_literal_v<T>, T>
      from_string_value(const string& str);

      template<typename T>
      typename std::enable_if_t<is_supported_literal_v<T>, string>
      serialize_to_literal(const T &t) {
        _debug("serialize_to_literal(const T& t)");
        if constexpr (is_same_v<remove_cv_t<T>, string>) {
          // due to the limitation of tinyxml2 (it only accpets char*), we'd like to discard
          // contents after the first '\0' for consistency.
          return string(t.c_str());
        } else if constexpr (is_same_v<remove_cv_t<T>, char *>) {
          // C-style string shouldn't contains null character. Any content after the first '\0'
          // will be ignored.
          return string(t);
        } else if constexpr (std::is_arithmetic_v<T>) {
          // std::to_chars can handle chars correctly, so no need to convert them to ints,
          // which is the reason why we use std::to_chars instead of std::stringstream.
          string result;
          if constexpr (std::is_floating_point_v<T>) {
            // Both gcc and clang<=13 do not support std::to_chars for floating point types.
            // A patch to support this is available at:
            // https://www.mail-archive.com/gcc-patches@gcc.gnu.org/msg242323.html
            // but so far, this patch is not accepted due to ODR violation concerns.
            // clang finally added support for std::to_chars for floating point types in
            // clang 14, which is released only several months earlier (Mar. 23rd, 2022).
            // So we have to use std::stringstream for floating point types.
            std::stringstream ss;
            // set the precision to the maximum digits number for this floating point type.
            ss << std::setprecision(std::numeric_limits<T>::max_digits10) << t;
            ASSERT(ss.good());
            result = ss.str();
          } else if constexpr (std::is_integral_v<T>) {
            result.resize(std::numeric_limits<T>::digits10 + 1);
            auto [ptr, ec] = std::to_chars(result.data(), result.data() + result.size(), t);
            if (ec != std::errc()) {
              throw std::runtime_error("failed to convert " + std::to_string(t) + " to string");
            }
            result.resize(ptr - result.data());
          } else {
            constexpr auto x = impossible_error(t, "is_arithmetic but not fp or integral");
          }
          _debug(string("serialize_to_literal(const T& t) result: ") + result);
          return result;
        } else {
          constexpr auto x = impossible_error(t, "Unsupported type for serialization to literal.");
        }
      }

      template<typename T>
      typename std::enable_if_t<is_supported_literal_v<T>, T>
      deserialize_from_literal(const string &s) {
        _debug("deserialize_from_literal(const string& s)");
        if constexpr (std::is_arithmetic_v<T>) {
          if constexpr (std::is_floating_point_v<T>) {
            // we first interpret the string with highest precision available, then cast it to the
            // desired type. This can help us get rid of writing a bunch of if-else statements.
            return static_cast<T>(std::stold(s));
          } else if constexpr (std::is_integral_v<T>) {
            // note that bools are also integral, treated as unsigned ints.
            if constexpr (std::is_signed_v<T>) {
              // ditto.
              return static_cast<T>(std::stoll(s));
            } else if constexpr (std::is_unsigned_v<T>) {
              // ditto.
              return static_cast<T>(std::stoull(s));
            } else {
              constexpr auto x = impossible_error(0, "T is neither signed nor unsigned.");
            }
          } else {
            constexpr auto x = impossible_error(0, "T is neither fp nor integral.");
          }
        } else if constexpr (is_same_v<remove_cv_t<T>, string>) {
          return s;
        } else if constexpr (is_same_v<remove_cv_t<T>, char *>) {
          return s.c_str();
        } else {
          constexpr auto x = impossible_error(0, "T is not a supported literal.");
        }
      }
      template<typename T>
      typename std::enable_if_t<is_supported_literal_v<T>, string>
      to_string_value(const T& t) {
        return serialize_to_literal(t);
      }

      template<typename T>
      typename std::enable_if_t<is_supported_literal_v<T>, T>
      from_string_value(const string& str) {
        return deserialize_from_literal<T>(str);
      }
    }
    // declarations
    // support user-defined serialize function for custom types
    template<typename T>
    void serialize_xml(const T& t, const string &node_name, XMLPrinter *printer);
    template<typename T>
    void serialize_xml(const T& t, const string &node_name, const string &file_name);
    template<typename T>
    string serialize_to_string_xml(const T& t, const string &node_name);
    template<typename T>
    void serialize_to_b64file_xml(const T& t, const string &node_name, const string &file_name);

    template<typename T>
    void deserialize_xml(T& t, const string &node_name, XMLElement *parent);
    template<typename T>
    void deserialize_xml(T& t, const string &node_name, const string &file_name);
    template<typename T>
    void deserialize_from_string_xml(T& t, const string &node_name, const string &xml_string);
    template<typename T>
    void deserialize_from_b64file_xml(T& t, const string &node_name, const string &file_name);

    // definitions
    template<typename T>
    void serialize_xml(const T& t, const string &node_name, XMLPrinter *printer) {
      _debug("serialize_xml(const T& t, const string &node_name, XMLPrinter *printer)");
      printer->OpenElement(node_name.c_str(), true);
      if constexpr (is_supported_container_v<T>) {
        _debug("is_supported_container_v<T>");
        if constexpr (is_pair_v<T>) {
          _debug("serialize_xml: is_pair_v<T>");
          serialize_xml(std::get<0>(t), "first", printer);
          serialize_xml(std::get<1>(t), "second", printer);
        } else if constexpr (is_array_container_v<T>) {
          _debug("serialize_xml: is_array_container_v<T>");
          printer->PushAttribute("size", std::to_string(t.size()).c_str());
          size_t index = 0;
          for (const auto& el : t) {
            serialize_xml(el, string("_") + std::to_string(index++), printer);
          }
        } else if constexpr (is_tuple_v<T>) {
          _debug("serialize_xml: is_tuple_v<T>");
          // Here we use foreach_in_tuple to iterate over the elements of the tuple at
          // compile time, since std::get<i> is constexpr after C++14.
          foreach_in_tuple(t, [&](const auto& el, const size_t i) {
            serialize_xml(el, string("_") + std::to_string(i), printer);
          });
        } else if constexpr (is_map_container_v<T>) {
          _debug("serialize_xml: is_map_container_v<T>");
          printer->PushAttribute("size", std::to_string(t.size()).c_str());
          size_t index = 0;
          for (const auto& el : t) {
            serialize_xml(el.first, string("_") + std::to_string(index) + "_k", printer);
            serialize_xml(el.second, string("_") + std::to_string(index) + "_v", printer);
            index++;
          }
        } else if constexpr (is_set_container_v<T>) {
          _debug("serialize_xml: is_set_container_v<T>");
          printer->PushAttribute("size", std::to_string(t.size()).c_str());
          size_t index = 0;
          for (const auto& el : t) {
            serialize_xml(el, string("_") + std::to_string(index++), printer);
          }
        } else {
          constexpr auto x = impossible_error(t, "T is a supported container type, but it's serializer is missing.");
        }
      } else if constexpr (is_supported_literal_v<T>) {
        if constexpr (is_cstring_v<T>) {
          printer->PushAttribute("val", t);
        } else {
          printer->PushAttribute("val", to_string_value(t).c_str());
        }
      } else if constexpr (is_base_of_v<XMLSerializable, remove_cv_t<T>>) {
        _debug("serialize_xml: is_base_of_v<XMLSerializable, remove_cv_t<T>>");
        vector<string> v = t.serializeToXML();
        serialize_xml(v, "udt", printer);
      } else {
        constexpr auto x = impossible_error(t, "T is not a supported type, you must derive T from XMLSerializable");
      }
      printer->CloseElement(true);
    }
    template<typename T>
    void serialize_xml(const T& t, const string &node_name, const string &file_name) {
      _debug("serialize_xml(const T& t, const string &node_name, const string &file_name)");
      XMLPrinter printer;
      printer.OpenElement("serialization", true);
      serialize_xml(t, node_name, &printer);
      printer.CloseElement(true);
      std::ofstream ofs(file_name);
      ASSERT(ofs.is_open());
      ofs << printer.CStr();
      ASSERT(ofs.good());
      ofs.close();
    }
    template<typename T>
    string serialize_to_string_xml(const T& t, const string &node_name) {
      _debug("serialize_to_string_xml(const T& t, const string &node_name)");
      XMLPrinter printer;
      printer.OpenElement("serialization", true);
      serialize_xml(t, node_name.c_str(), &printer);
      printer.CloseElement(true);
      return string(printer.CStr());
    }
    template<typename T>
    void serialize_to_b64file_xml(const T& t, const string &node_name, const string &file_name) {
      _debug("serialize_to_b64file_xml(const T& t, const string &node_name, const string &file_name)");
      string xml = serialize_to_string_xml(t, node_name);
      string b64_xml = base64_encode_pem(xml);
      std::ofstream ofs(file_name);
      ASSERT(ofs.is_open());
      ofs << b64_xml;
      ASSERT(ofs.good());
      ofs.close();
    }

    template<typename T>
    void deserialize_xml(T& t, const string &node_name, XMLElement *parent) {
      _debug("deserialize_xml(T& t, const string &node_name, XMLElement *parent)");
      XMLElement *elem = parent->FirstChildElement(node_name.c_str());
      ASSERT(elem != nullptr);
      if constexpr (is_supported_container_v<T>) {
        _debug("is_supported_container_v<T>");
        if constexpr (is_pair_v<T>) {
          _debug("deserialize_xml: is_pair_v<T>");
          deserialize_xml(std::get<0>(t), "first", elem);
          deserialize_xml(std::get<1>(t), "second", elem);
        } else if constexpr (is_array_container_v<T>) {
          _debug("deserialize_xml: is_array_container_v<T>");
          // child count
          ASSERT(elem->Attribute("size") != nullptr);
          const size_t size = std::stoul(elem->Attribute("size"));
          // resize
          t.resize(size);
          size_t index = 0;
          for (auto& el : t) {
            deserialize_xml(el, string("_") + std::to_string(index++), elem);
          }
        } else if constexpr (is_tuple_v<T>) {
          _debug("deserialize_xml: is_tuple_v<T>");
          // Here we use foreach_in_tuple to iterate over the elements of the tuple at
          // compile time, since std::get<i> is constexpr after C++14.
          foreach_in_tuple(t, [&](auto& el, const auto i) {
            deserialize_xml(el, string("_") + std::to_string(i), elem);
          });
        } else if constexpr (is_map_container_v<T>) {
          _debug("deserialize_xml: is_map_container_v<T>");
          ASSERT(elem->Attribute("size") != nullptr);
          const size_t size = std::stoul(elem->Attribute("size"));
          for (size_t i = 0; i < size; i++) {
            XMLElement *k = elem->FirstChildElement((string("_") + std::to_string(i) + "_k").c_str());
            XMLElement *v = elem->FirstChildElement((string("_") + std::to_string(i) + "_v").c_str());
            ASSERT(k != nullptr);
            ASSERT(v != nullptr);
            typename T::key_type key;
            typename T::mapped_type value;
            deserialize_xml(key, string("_") + std::to_string(i) + "_k", elem);
            deserialize_xml(value, string("_") + std::to_string(i) + "_v", elem);
            t.insert(std::make_pair(key, value));
          }
        } else if constexpr (is_set_container_v<T>) {
          _debug("deserialize_xml: is_set_container_v<T>");
          ASSERT(elem->Attribute("size") != nullptr);
          const size_t size = std::stoul(elem->Attribute("size"));
          for (size_t i = 0; i < size; i++) {
            typename T::value_type value;
            deserialize_xml(value, string("_") + std::to_string(i), elem);
            t.insert(value);
          }
        } else {
          constexpr auto x = impossible_error(t, "T is a supported container type, but it's serializer is missing.");
        }
      } else if constexpr (is_supported_literal_v<T>) {
        ASSERT(elem->Attribute("val") != nullptr);
        if constexpr (is_cstring_v<T>) {
          // Reading a string in this case so that we don't need to worry about memory allocation.
          string s = from_string_value<string>(elem->Attribute("val"));
          // The user should preallocate enough spaces for C-style strings.
          memcpy(t, s.c_str(), s.size());
        } else {
          t = from_string_value<T>(elem->Attribute("val"));
        }
      } else if constexpr (std::is_base_of_v<XMLSerializable, remove_cv_t<T>>) {
        _debug("deserialize_xml: is_base_of_v<XMLSerializable, remove_cv_t<T>>");
        vector<string> args;
        deserialize_xml(args, "udt", elem);
        t.deserializeFromXML(args);
      } else {
        constexpr auto x = impossible_error(t, "T is not a supported type, you must derive T from XMLSerializable");
      }
    }
    template<typename T>
    void deserialize_xml(T& t, const string &node_name, const string &file_name) {
      _debug("deserialize_xml(T& t, const string &node_name, const string &file_name)");
      XMLDocument doc;
      doc.LoadFile(file_name.c_str());
      ASSERT(doc.ErrorID() == 0);
      XMLElement* root = doc.FirstChildElement("serialization");
      ASSERT(root != nullptr);
      deserialize_xml(t, node_name, root);
      doc.Clear();
    }
    template<typename T>
    void deserialize_from_string_xml(T& t, const string &node_name, const string &xml_string) {
      _debug("deserialize_from_string_xml(T& t, const string &node_name, const string &xml_string)");
      XMLDocument doc;
      doc.Parse(xml_string.c_str());
      ASSERT(doc.ErrorID() == 0);
      XMLElement* root = doc.FirstChildElement("serialization");
      ASSERT(root != nullptr);
      deserialize_xml(t, node_name, root);
      doc.Clear();
    }
    template<typename T>
    void deserialize_from_b64file_xml(T& t, const string &node_name, const string &file_name) {
      _debug("deserialize_from_b64file_xml(T& t, const string &node_name, const string &file_name)");
      std::ifstream ifs(file_name);
      ASSERT(ifs.is_open());
      std::stringstream b64_xml_ss;
      b64_xml_ss << ifs.rdbuf();
      string xml = base64_decode(b64_xml_ss.str(), true);
      deserialize_from_string_xml(t, node_name, xml);
    }
  } // namespace xml
} // namespace serializer
