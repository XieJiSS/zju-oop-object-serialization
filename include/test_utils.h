#pragma once

#include <iostream>

static bool has_failed = false;

#define EXPECT_EQ(a, b, name) \
  do { \
    if (a != b) { \
      has_failed = true; \
      std::cerr << "FAILED " << name << ": " << a << " != " << b << std::endl; \
    } else { \
      std::cout << "PASSED " << name << ": " << a << " == " << b << std::endl; \
    } \
  } while(0);

#define SHOW_TEST_RESULTS() \
  if (has_failed) { \
    std::cout << "Some tests have failed!" << std::endl; \
  } else { \
    std::cout << "All tests passed!" << std::endl; \
  }
