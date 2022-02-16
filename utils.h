/*
-----------------------------------------------------------------------------
This source file is part of the Havoc chess engine
Copyright (c) 2020 Minniesoft
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/
#pragma once

#ifndef UTILS_H
#define UTILS_H

#include <cstdlib>
#include <random>
#include <climits>
#include <memory>
#include <chrono>
#include <ctime>
#include <sstream>
#include <vector>
#include <string>
#include <iostream>

#include "types.h"

namespace util {  
  // std::make_unique is part of c++14
  template<typename T, typename... Args>
  std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
  }

  inline std::vector<std::string> split(std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(s);
    std::string t;
    while (getline(ss, t, delimiter)) { tokens.push_back(t); }
    return tokens;
  }


  inline int row(const int& r) { return (r >> 3); }
  inline int col(const int& c) { return c & 7; }
  inline int row_dist(const int& s1, const int& s2) { return abs(row(s1) - row(s2)); }
  inline int col_dist(const int& s1, const int& s2) { return abs(col(s1) - col(s2)); }
  inline bool on_board(const int& s1) { return s1 >= 0 && s1 <= 63; }
  inline bool same_row(const int& s1, const int& s2) { return row(s1) == row(s2); }
  inline bool same_col(const int& s1, const int& s2) { return col(s1) == col(s2); }
  inline bool on_diagonal(const int& s1, const int& s2) { return col_dist(s1, s2) == row_dist(s1, s2); }
  inline bool aligned(const int& s1, const int& s2) {
    return on_diagonal(s1, s2) || same_row(s1, s2) || same_col(s1, s2);
  }
  inline bool aligned(const int& s1, const int& s2, const int& s3) {
    return (same_col(s1, s2) && same_col(s1, s3)) ||
      (same_row(s1, s2) && same_row(s1, s3)) ||
      (on_diagonal(s1, s3) && on_diagonal(s1, s2) && on_diagonal(s2, s3));
  }
  
  inline U64 squares_infront(const U64& colbb, const Color& c, const int& s) {
    return (c == white ? colbb << 8 * (row(s) + 1) : colbb >> 8 * (8 - row(s)));
  }


  inline U64 squares_behind(U64& colbb, const Color& c, const int& s) {
    return ~squares_infront(colbb, c, s) & colbb;
  }
    
  template<typename T>
    class rand {
    std::mt19937 gen;
    std::uniform_int_distribution<T> dis;
  public:
  rand() : gen(0), dis(0, UINT_MAX) { }
    
    T next() { return dis(gen); }
  };     
  
  
  class clock {
    std::chrono::time_point<std::chrono::system_clock> _start;
    std::chrono::time_point<std::chrono::system_clock> _end;
  public:
    clock() = default;

    void start() { _start = std::chrono::system_clock::now(); }
    void stop() { _end = std::chrono::system_clock::now(); }
    double elapsed_ms() { stop(); double msecs = ms(); start(); return msecs; }
    double ms() const
    {
      std::chrono::duration<double> sec = _end - _start;
      return sec.count() * 1000.0;
    }

  };
}

#endif
