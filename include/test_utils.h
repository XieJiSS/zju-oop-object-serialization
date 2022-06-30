#pragma once

#include <iostream>

static unsigned int total_tests = 0;
static unsigned int total_failures = 0;

#define EXPECT_EQ(a, b, name) \
  do { \
    total_tests++; \
    if (a != b) { \
      total_failures++; \
      std::cerr << "FAILED " << name << ": " << a << " != " << b << std::endl; \
    } else { \
      std::cout << "PASSED " << name << ": " << a << " == " << b << std::endl; \
    } \
  } while(0);

#define SHOW_TEST_RESULT() \
  do { \
    std::cout << "[test_result] "; \
    if (total_failures > 0) { \
      std::cout << total_failures << " tests within " << total_tests << " have failed!" << std::endl; \
    } else { \
      std::cout << "All " << total_tests << " tests have passed!" << std::endl; \
    } \
  } while(0);

#define TEST_QUIT() \
  do { \
    if (total_failures > 0) { \
      return 1; \
    } else { \
      return 0; \
    } \
  } while(0);
