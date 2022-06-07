#include "libbinary.h"
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

using namespace serializer::binary;

struct UserDefinedType {
  int idx;
  std::string name;
  std::vector<double> data;
};

string serializeMyStruct(const UserDefinedType& udt) {
  std::stringstream ss;
  serialize(udt.idx, ss);
  serialize(udt.name, ss);
  serialize(udt.data, ss);
  return ss.str();
}

void deserializeMyStruct(UserDefinedType& udt, const string& str) {
  std::stringstream ss(str);
  deserialize(udt.idx, ss);
  deserialize(udt.name, ss);
  deserialize(udt.data, ss);
}

int main() {
  cout << std::setprecision(10);

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
  long double long_double1 = 123.45678, long_double2;
  serialize(u_char1, "result/uchar.bin");
  serialize(s_char1, "result/schar.bin");
  serialize(u_short1, "result/ushort.bin");
  serialize(s_short1, "result/sshort.bin");
  serialize(u_int1, "result/uint.bin");
  serialize(s_int1, "result/sint.bin");
  serialize(u_longlong1, "result/ulong_long.bin");
  serialize(s_longlong1, "result/slong_long.bin");
  serialize(float1, "result/float.bin");
  serialize(double1, "result/double.bin");
  serialize(long_double1, "result/long_double.bin");
  deserialize(u_char2, "result/uchar.bin");
  deserialize(s_char2, "result/schar.bin");
  deserialize(u_short2, "result/ushort.bin");
  deserialize(s_short2, "result/sshort.bin");
  deserialize(u_int2, "result/uint.bin");
  deserialize(s_int2, "result/sint.bin");
  deserialize(u_longlong2, "result/ulong_long.bin");
  deserialize(s_longlong2, "result/slong_long.bin");
  deserialize(float2, "result/float.bin");
  deserialize(double2, "result/double.bin");
  deserialize(long_double2, "result/long_double.bin");
  EXPECT_EQ((int)u_char1, (int)u_char2, "unsigned char");
  EXPECT_EQ((int)s_char1, (int)s_char2, "signed char");
  EXPECT_EQ(u_short1, u_short2, "unsigned short");
  EXPECT_EQ(s_short1, s_short2, "signed short");
  EXPECT_EQ(u_int1, u_int2, "unsigned int");
  EXPECT_EQ(s_int1, s_int2, "signed int");
  EXPECT_EQ(u_longlong1, u_longlong2, "unsigned long long");
  EXPECT_EQ(s_longlong1, s_longlong2, "signed long long");
  EXPECT_EQ(float1, float2, "float");
  EXPECT_EQ(double1, double2, "double");
  EXPECT_EQ(long_double1, long_double2, "long double");
  
  // test string
  string str1 = "test 123";
  serialize(str1, "result/str.bin");
  string str2;
  deserialize(str2, "result/str.bin");
  EXPECT_EQ(str1, str2, "std::string");

  // test vector
  vector<int> vec1 = {1, 2, 3, 4, 5};
  serialize(vec1, "result/vector.bin");
  vector<int> vec2;
  deserialize(vec2, "result/vector.bin");
  EXPECT_EQ(vec1.size(), vec2.size(), "vector.size()");
  for(auto i = 0; i < vec1.size(); i++) {
    EXPECT_EQ(vec1[i], vec2[i], "vector[i]");
  }

  // test list
  list<int> list1 = {1, 2, 3, 4};
  serialize(list1, "result/list.bin");
  list<int> list2;
  deserialize(list2, "result/list.bin");
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
  serialize(map1, "result/map.bin");
  map<int, int> map2;
  deserialize(map2, "result/map.bin");
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
  serialize(set1, "result/set.bin");
  set<int> set2;
  deserialize(set2, "result/set.bin");
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
  serialize(unordered_map1, "result/unordered_map.bin");
  unordered_map<int, int> unordered_map2;
  deserialize(unordered_map2, "result/unordered_map.bin");
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
  serialize(pair1, "result/pair.bin");
  pair<int, int> pair2;
  deserialize(pair2, "result/pair.bin");
  EXPECT_EQ(pair1.first, pair2.first, "pair.first");
  EXPECT_EQ(pair1.second, pair2.second, "pair.second");

  // test std::tuple
  tuple<int, int, int> tuple1 = {1, 2, 3};
  serialize(tuple1, "result/tuple.bin");
  tuple<int, int, int> tuple2;
  deserialize(tuple2, "result/tuple.bin");
  EXPECT_EQ(std::get<0>(tuple1), std::get<0>(tuple2), "std::get<0>(tuple)");
  EXPECT_EQ(std::get<1>(tuple1), std::get<1>(tuple2), "std::get<1>(tuple)");
  EXPECT_EQ(std::get<2>(tuple1), std::get<2>(tuple2), "std::get<2>(tuple)");

  // test user-defined types
  UserDefinedType udt1 = {1, "MyName", {4.1, 5.2, 6.3}};
  serialize(udt1, "result/udt.bin", serializeMyStruct);
  UserDefinedType udt2;
  deserialize(udt2, "result/udt.bin", deserializeMyStruct);
  EXPECT_EQ(udt1.idx, udt2.idx, "utd.idx");
  EXPECT_EQ(udt1.name, udt2.name, "utd.name");
  for(size_t i = 0; i < udt1.data.size(); i++) {
    EXPECT_EQ(udt1.data[i], udt2.data[i], "utd.data[" + std::to_string(i) + "]");
  }
  try {
    deserialize(udt1, "result/non_existing_file.bin", deserializeMyStruct);
    EXPECT_EQ(1, 0, "deserialize from non-existing file should throw an exception");
  } catch (const std::exception& e) {
    cout << "PASSED (XFAIL) deserialize(udt1, \"result/non_existing_file.bin\") failed as expected." << endl;
  }

  // test consts
  // const arithmetic
  const int const_int1 = 1;
  serialize(const_int1, "result/const_int.bin");
  int const_int2;
  deserialize(const_int2, "result/const_int.bin");
  EXPECT_EQ(const_int1, const_int2, "const int");

  // const string
  const string const_str1 = "MyName";
  serialize(const_str1, "result/const_str.bin");
  string const_str2;
  deserialize(const_str2, "result/const_str.bin");
  EXPECT_EQ(const_str1, const_str2, "const string");

  // const pair (test trivial container)
  const pair<int, int> const_pair1 = {1, 2};
  serialize(const_pair1, "result/const_pair.bin");
  pair<int, int> const_pair2;
  deserialize(const_pair2, "result/const_pair.bin");
  EXPECT_EQ(const_pair1.first, const_pair2.first, "const pair.first");
  EXPECT_EQ(const_pair1.second, const_pair2.second, "const pair.second");

  // const tuple (test non-trivial container)
  const tuple<int, int, int> const_tuple1 = {1, 2, 3};
  serialize(const_tuple1, "result/const_tuple.bin");
  tuple<int, int, int> const_tuple2;
  deserialize(const_tuple2, "result/const_tuple.bin");
  EXPECT_EQ(std::get<0>(const_tuple1), std::get<0>(const_tuple2), "std::get<0>(const_tuple)");
  EXPECT_EQ(std::get<1>(const_tuple1), std::get<1>(const_tuple2), "std::get<1>(const_tuple)");
  EXPECT_EQ(std::get<2>(const_tuple1), std::get<2>(const_tuple2), "std::get<2>(const_tuple)");

  // const vector (test non-trivial container)
  const vector<int> const_vector1 = {1, 2, 3, 4, 5};
  serialize(const_vector1, "result/const_vector.bin");
  vector<int> const_vector2;
  deserialize(const_vector2, "result/const_vector.bin");
  EXPECT_EQ(const_vector1.size(), const_vector2.size(), "const vector.size");
  for(auto i = 0; i < const_vector1.size(); i++) {
    EXPECT_EQ(const_vector1[i], const_vector2[i], "const vector[i]");
  }

  SHOW_TEST_RESULTS();
}
