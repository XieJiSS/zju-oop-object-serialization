#include "libbinary.h"
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <vector>
#include <list>
#include <map>
#include <unordered_map>
#include <set>

bool has_failed = false;

#define EXPECT_EQ(a, b, name) \
  do { \
    if (a != b) { \
      has_failed = true; \
      std::cerr << "FAILED " << name << ": " << a << " != " << b << std::endl; \
    } else { \
      std::cout << "PASSED " << name << ": " << a << " == " << b << std::endl; \
    } \
  } while(0);

using std::string;
using std::vector;
using std::list;
using std::map;
using std::unordered_map;
using std::set;
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
  cout << "std::vector: ";
  for (auto i : vec1) {
    cout << i << " ";
  }
  cout << "should be the same as ";
  for (auto i : vec2) {
    cout << i << " ";
  }
  cout << endl;

  // test list
  list<int> list1 = {1, 2, 3, 4};
  serialize(list1, "result/list.bin");
  list<int> list2;
  deserialize(list2, "result/list.bin");
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
  cout << "std::unordered_map: ";
  for (auto i : unordered_map1) {
    cout << i.first << "->" << i.second << " ";
  }
  cout << "should contain the same kv pairs as ";
  for (auto i : unordered_map2) {
    cout << i.first << "->" << i.second << " ";
  }
  cout << endl;

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

  if(has_failed) {
    cout << "Some tests have failed!" << endl;
  } else {
    cout << "All tests passed!" << endl;
  }
}
