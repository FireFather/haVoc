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

#ifndef MATERIAL_H
#define MATERIAL_H

#include <memory>

#include "position.h"
#include "types.h"

struct material_entry {
  U64 key;
  int16 score;
  double endgame_coeff; // interpolation between middle and endgame
  EndgameType endgame;
  U8 number[5]; // knight, bishop, rook, queen
  bool is_endgame() const { return endgame != none;  }
};


class material_table {
	size_t sz_mb;
    size_t count;
    std::unique_ptr<material_entry[]> entries;

    void init();

  public:
    material_table();
    material_table(const material_table& o) = delete;
    material_table(const material_table&& o) = delete;
    material_table& operator=(const material_table& o) = delete;
    material_table& operator=(const material_table&& o) = delete;

    ~material_table() = default;

	void clear() const;
    material_entry * fetch(const position& p) const;
	  
};


extern material_table mtable;

#endif