#include "libxml.h"
#include "test_utils.h"

#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <vector>
#include <list>
#include <map>
#include <unordered_map>
#include <set>
#include <tuple>

using std::string;
using std::vector;
using std::list;
using std::map;
using std::unordered_map;
using std::set;
using std::pair;
using std::tuple;
using std::cout;
using std::endl;

using namespace serializer::xml;

// a nested struct
struct _SimpleStruct : XMLSerializable {
  _SimpleStruct() {}
  _SimpleStruct(int a, int b) : a(a), b(b) {}
  int a;
  int b;
  vector<string> serializeXML() const override;
  void deserializeXML(const vector<string>&) override;
};

vector<string> _SimpleStruct::serializeXML() const {
  vector<string> result;
  result.push_back(serialize_to_string_xml(this->a, "s.a"));
  result.push_back(serialize_to_string_xml(this->b, "s.b"));
  return result;
}
void _SimpleStruct::deserializeXML(const vector<string>& v) {
  deserialize_from_string_xml(this->a, "s.a", v[0]);
  deserialize_from_string_xml(this->b, "s.b", v[1]);
}

struct UserDefinedType : XMLSerializable {
  UserDefinedType() {}
  UserDefinedType(int idx, string name, vector<double> data, _SimpleStruct simpleObj) :
    idx(idx),
    name(name),
    data(data),
    simpleObj(simpleObj) {}
  int idx;
  std::string name;
  std::vector<double> data;
  _SimpleStruct simpleObj;
  vector<string> serializeXML() const override;
  void deserializeXML(const vector<string>&) override;
};

vector<string> UserDefinedType::serializeXML() const {
  vector<string> result;
  result.push_back(serialize_to_string_xml(this->idx, "_0"));
  result.push_back(serialize_to_string_xml(this->name, "_1"));
  result.push_back(serialize_to_string_xml(this->data, "_2"));
  result.push_back(serialize_to_string_xml(this->simpleObj, "_3"));
  return result;
}

void UserDefinedType::deserializeXML(const vector<string>& v) {
  deserialize_from_string_xml(this->idx, "_0", v[0]);
  deserialize_from_string_xml(this->name, "_1", v[1]);
  deserialize_from_string_xml(this->data, "_2", v[2]);
  deserialize_from_string_xml(this->simpleObj, "_3", v[3]);
}

int main() {
  cout << std::setprecision(std::numeric_limits<long double>::max_digits10);

  // test arithmetic types
  unsigned char u_char1 = 0xFF, u_char2;
  signed char s_char1 = -1, s_char2;
  unsigned short u_short1 = 0xFFFF, u_short2;
  signed short s_short1 = -0x80, s_short2;
  unsigned int u_int1 = 123, u_int2;
  signed int s_int1 = -456, s_int2;
  unsigned long long u_longlong1 = 12300, u_longlong2;
  signed long long s_longlong1 = -45600, s_longlong2;
  float float1 = 123.456, float2;
  double double1 = 123.4567, double2;
  long double long_double1 = 123.456789, long_double2;

  serialize_to_b64file_xml(u_char1, "u_char", "result/u_char.xml.b64");
  serialize_to_b64file_xml(s_char1, "s_char", "result/s_char.xml.b64");
  serialize_to_b64file_xml(u_short1, "u_short", "result/u_short.xml.b64");
  serialize_to_b64file_xml(s_short1, "s_short", "result/s_short.xml.b64");
  serialize_to_b64file_xml(u_int1, "u_int", "result/u_int.xml.b64");
  serialize_to_b64file_xml(s_int1, "s_int", "result/s_int.xml.b64");
  serialize_to_b64file_xml(u_longlong1, "u_longlong", "result/u_longlong.xml.b64");
  serialize_to_b64file_xml(s_longlong1, "s_longlong", "result/s_longlong.xml.b64");
  serialize_to_b64file_xml(float1, "float", "result/float.xml.b64");
  serialize_to_b64file_xml(double1, "double", "result/double.xml.b64");
  serialize_to_b64file_xml(long_double1, "long_double", "result/long_double.xml.b64");
  deserialize_from_b64file_xml(u_char2, "u_char", "result/u_char.xml.b64");
  deserialize_from_b64file_xml(s_char2, "s_char", "result/s_char.xml.b64");
  deserialize_from_b64file_xml(u_short2, "u_short", "result/u_short.xml.b64");
  deserialize_from_b64file_xml(s_short2, "s_short", "result/s_short.xml.b64");
  deserialize_from_b64file_xml(u_int2, "u_int", "result/u_int.xml.b64");
  deserialize_from_b64file_xml(s_int2, "s_int", "result/s_int.xml.b64");
  deserialize_from_b64file_xml(u_longlong2, "u_longlong", "result/u_longlong.xml.b64");
  deserialize_from_b64file_xml(s_longlong2, "s_longlong", "result/s_longlong.xml.b64");
  deserialize_from_b64file_xml(float2, "float", "result/float.xml.b64");
  deserialize_from_b64file_xml(double2, "double", "result/double.xml.b64");
  deserialize_from_b64file_xml(long_double2, "long_double", "result/long_double.xml.b64");
  EXPECT_EQ((int)u_char1, (int)u_char2, "unsigned char");
  EXPECT_EQ((int)s_char1, (int)s_char2, "signed char");
  EXPECT_EQ((int)u_short1, (int)u_short2, "unsigned short");
  EXPECT_EQ((int)s_short1, (int)s_short2, "signed short");
  EXPECT_EQ((int)u_int1, (int)u_int2, "unsigned int");
  EXPECT_EQ((int)s_int1, (int)s_int2, "signed int");
  EXPECT_EQ((int)u_longlong1, (int)u_longlong2, "unsigned long long");
  EXPECT_EQ((int)s_longlong1, (int)s_longlong2, "signed long long");
  EXPECT_EQ(float1, float2, "float");
  EXPECT_EQ(double1, double2, "double");
  EXPECT_EQ(long_double1, long_double2, "long double");

  // test string
  string str1 = "Hello World!", str2;
  serialize_to_b64file_xml(str1, "str", "result/str.xml.b64");
  deserialize_from_b64file_xml(str2, "str", "result/str.xml.b64");
  EXPECT_EQ(str1, str2, "string");

  // test vector
  vector<int> vec1 = {1, 2, 3, 4, 5};
  serialize_to_b64file_xml(vec1, "std_vector", "result/vector.xml.b64");
  vector<int> vec2;
  deserialize_from_b64file_xml(vec2, "std_vector", "result/vector.xml.b64");
  EXPECT_EQ(vec1.size(), vec2.size(), "vector.size()");
  for(auto i = 0; i < vec1.size(); i++) {
    EXPECT_EQ(vec1[i], vec2[i], "vector[i]");
  }

  // test list
  list<int> list1 = {1, 2, 3, 4};
  serialize_to_b64file_xml(list1, "std_list", "result/list.xml.b64");
  list<int> list2;
  deserialize_from_b64file_xml(list2, "std_list", "result/list.xml.b64");
  EXPECT_EQ(list1.size(), list2.size(), "list.size()");
  cout << "std::list: ";
  for (auto i : list1) {
    cout << i << " ";
  }
  cout << "should be the same as ";
  for (auto i : list2) {
    cout << i << " ";
  }
  cout << endl;

  // test map
  map<int, int> map1 = {{1, 1}, {2, 2}, {3, 3}};
  serialize_to_b64file_xml(map1, "std_map", "result/map.xml.b64");
  map<int, int> map2;
  deserialize_from_b64file_xml(map2, "std_map", "result/map.xml.b64");
  EXPECT_EQ(map1.size(), map2.size(), "map.size()");
  cout << "std::map: ";
  for (auto i : map1) {
    cout << i.first << "->" << i.second << " ";
  }
  cout << "should be the same as ";
  for (auto i : map2) {
    cout << i.first << "->" << i.second << " ";
  }
  cout << endl;

  // test set
  set<int> set1 = {1, 2, 1};
  serialize_to_b64file_xml(set1, "std_set", "result/set.xml.b64");
  set<int> set2;
  deserialize_from_b64file_xml(set2, "std_set", "result/set.xml.b64");
  EXPECT_EQ(set1.size(), set2.size(), "set.size()");
  cout << "std::set: ";
  for (auto i : set1) {
    cout << i << " ";
  }
  cout << "should contain the same numbers as ";
  for (auto i : set2) {
    cout << i << " ";
  }
  cout << endl;

  // test map generalization (unordered_map as an example)
  unordered_map<int, int> unordered_map1 = {{1, 1}, {2, 2}, {3, 3}};
  serialize_to_b64file_xml(unordered_map1, "std_unordered_map", "result/unordered_map.xml.b64");
  unordered_map<int, int> unordered_map2;
  deserialize_from_b64file_xml(unordered_map2, "std_unordered_map", "result/unordered_map.xml.b64");
  EXPECT_EQ(unordered_map1.size(), unordered_map2.size(), "unordered_map.size()");
  cout << "std::unordered_map: ";
  for (auto i : unordered_map1) {
    cout << i.first << "->" << i.second << " ";
  }
  cout << "should contain the same kv pairs as ";
  for (auto i : unordered_map2) {
    cout << i.first << "->" << i.second << " ";
  }
  cout << endl;

  // test std::pair
  pair<int, int> pair1 = {1, 2};
  serialize_to_b64file_xml(pair1, "std_pair", "result/pair.xml.b64");
  pair<int, int> pair2;
  deserialize_from_b64file_xml(pair2, "std_pair", "result/pair.xml.b64");
  EXPECT_EQ(pair1.first, pair2.first, "pair.first");
  EXPECT_EQ(pair1.second, pair2.second, "pair.second");

  // test std::tuple
  tuple<int, int, int> tuple1 = {1, 2, 3};
  serialize_to_b64file_xml(tuple1, "std_tuple", "result/tuple.xml.b64");
  tuple<int, int, int> tuple2;
  deserialize_from_b64file_xml(tuple2, "std_tuple", "result/tuple.xml.b64");
  EXPECT_EQ(std::get<0>(tuple1), std::get<0>(tuple2), "std::get<0>(tuple)");
  EXPECT_EQ(std::get<1>(tuple1), std::get<1>(tuple2), "std::get<1>(tuple)");
  EXPECT_EQ(std::get<2>(tuple1), std::get<2>(tuple2), "std::get<2>(tuple)");

  // test user-defined types
  UserDefinedType udt1 = {1, "MyName", {4.1, 5.2, 6.3}, _SimpleStruct{1, 2}};
  serialize_to_b64file_xml(udt1, "udt", "result/udt.xml.b64");
  UserDefinedType udt2;
  deserialize_from_b64file_xml(udt2, "udt", "result/udt.xml.b64");
  EXPECT_EQ(udt1.idx, udt2.idx, "utd.idx");
  EXPECT_EQ(udt1.name, udt2.name, "utd.name");
  for(size_t i = 0; i < udt1.data.size(); i++) {
    EXPECT_EQ(udt1.data[i], udt2.data[i], "utd.data[" + std::to_string(i) + "]");
  }
  EXPECT_EQ(udt1.simpleObj.a, udt2.simpleObj.a, "utd.simpleObj.a");
  EXPECT_EQ(udt1.simpleObj.b, udt2.simpleObj.b, "utd.simpleObj.b");
  try {
    deserialize_from_b64file_xml(udt1, "udt", "result/non_existing_file.xml.b64");
    EXPECT_EQ(1, 0, "deserialize from non-existing file should throw an exception");
  } catch (const std::exception& e) {
    cout << "PASSED (XFAIL) deserialize_from_b64file_xml(udt1, \"result/non_existing_file.xml.b64\") failed as expected." << endl;
  }

  // test consts
  // const arithmetic
  const int const_int1 = 1;
  serialize_to_b64file_xml(const_int1, "const_int", "result/const_int.xml.b64");
  int const_int2;
  deserialize_from_b64file_xml(const_int2, "const_int", "result/const_int.xml.b64");
  EXPECT_EQ(const_int1, const_int2, "const int");

  // const string
  const string const_str1 = "MyName";
  serialize_to_b64file_xml(const_str1, "const_str", "result/const_str.xml.b64");
  string const_str2;
  deserialize_from_b64file_xml(const_str2, "const_str", "result/const_str.xml.b64");
  EXPECT_EQ(const_str1, const_str2, "const string");

  // const pair (test trivial container)
  const pair<int, int> const_pair1 = {1, 2};
  serialize_to_b64file_xml(const_pair1, "const_std_pair", "result/const_pair.xml.b64");
  pair<int, int> const_pair2;
  deserialize_from_b64file_xml(const_pair2, "const_std_pair", "result/const_pair.xml.b64");
  EXPECT_EQ(const_pair1.first, const_pair2.first, "const pair.first");
  EXPECT_EQ(const_pair1.second, const_pair2.second, "const pair.second");

  // const tuple (test non-trivial container)
  const tuple<int, int, int> const_tuple1 = {1, 2, 3};
  serialize_to_b64file_xml(const_tuple1, "const_std_tuple", "result/const_tuple.xml.b64");
  tuple<int, int, int> const_tuple2;
  deserialize_from_b64file_xml(const_tuple2, "const_std_tuple", "result/const_tuple.xml.b64");
  EXPECT_EQ(std::get<0>(const_tuple1), std::get<0>(const_tuple2), "std::get<0>(const_tuple)");
  EXPECT_EQ(std::get<1>(const_tuple1), std::get<1>(const_tuple2), "std::get<1>(const_tuple)");
  EXPECT_EQ(std::get<2>(const_tuple1), std::get<2>(const_tuple2), "std::get<2>(const_tuple)");

  // const vector (test non-trivial container)
  const vector<int> const_vector1 = {1, 2, 3, 4, 5};
  serialize_to_b64file_xml(const_vector1, "const_std_vector", "result/const_vector.xml.b64");
  vector<int> const_vector2;
  deserialize_from_b64file_xml(const_vector2, "const_std_vector", "result/const_vector.xml.b64");
  EXPECT_EQ(const_vector1.size(), const_vector2.size(), "const vector.size");
  for(auto i = 0; i < const_vector1.size(); i++) {
    EXPECT_EQ(const_vector1[i], const_vector2[i], "const vector[i]");
  }

  // const char*
  const char* const_cstr1 = "MyName";
  serialize_to_b64file_xml(const_cstr1, "const_cstr", "result/const_cstr.xml.b64");
  char* const_cstr2 = (char *)malloc(strlen(const_cstr1) + 1);
  deserialize_from_b64file_xml(const_cstr2, "const_cstr", "result/const_cstr.xml.b64");
  EXPECT_EQ(string(const_cstr1), string(const_cstr2), "const char*");

  SHOW_TEST_RESULT();
  TEST_QUIT();
}
