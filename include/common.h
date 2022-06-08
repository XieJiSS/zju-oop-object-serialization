#pragma once

#define ASSERT(a) \
  do { \
    if (!(a)) { \
      throw std::runtime_error("Assertion failed: " #a); \
    } \
  } while(0);
