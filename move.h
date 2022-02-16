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

#ifndef MOVE_H
#define MOVE_H

#include <vector>
#include <array>

#include "types.h"
#include "utils.h"

class position;

enum Dir { N, S, NN, SS, NW, NE, SW, SE, no_dir };




class Movegen {
  int last;
  Move list[218]{}; // max moves in any chess position
  Color us, them;
  U64 rank2{}, rank7{};
  U64 empty{}, pawns{}, pawns2{}, pawns7{};
  std::vector<U64> bishop_mvs, rook_mvs, queen_mvs;  
  Square * knights{};
  Square * bishops{};
  Square * rooks{};
  Square * queens{};
  Square * kings{};
  U64 enemies{}, all_pieces{}, qtarget{}, ctarget{}, check_target{}, evasion_target{};
  Square eps;
  bool can_castle_ks{}, can_castle_qs{};
  
  // utilities  
  inline void initialize(const position& p);
  inline void pawn_caps(U64& left, U64& right, U64& ep_left, U64& ep_right) const;
  
  template<Movetype mt>
  void encode(U64& b, const int& f);

  template<Movetype mt>
  void encode_pawn_pushes(U64& b, const int& dir);

  inline void pawn_quiets(U64& single, U64& dbl) const;
  inline void encode_promotions(U64& b, const int& dir);
  inline void capture_promotions(U64& right_caps, U64& left_caps) const;
  inline void quiet_promotions(U64& quiets) const;
  inline void encode_capture_promotions(U64& b, const int& dir);
  
 public:
  Movegen() : last(0), list{}, us(), them(), rank2(0), rank7(0), empty(0), pawns(0), pawns2(0), pawns7(0), knights(nullptr), bishops(nullptr), rooks(nullptr), queens(nullptr), kings(nullptr), enemies(0), all_pieces(0), qtarget(0), ctarget(0), check_target(0), evasion_target(0), eps(), can_castle_ks(false), can_castle_qs(false)
  {
  }

  Movegen(const position& pos) : last(0) { initialize(pos); }
  Movegen(const Movegen& o) = delete;
  Movegen(const Movegen&& o) = delete;
  ~Movegen() = default;

  Movegen& operator=(const Movegen& o) = delete;
  Movegen& operator=(const Movegen&& o) = delete;
  Move& operator[](const int& idx) { return list[idx]; }
  
  template<Movetype mt, Piece p>
  void generate();

  template<Piece p>
  void generate();
  
  // utilities
  int size() const { return last; }    
  inline void print() const;
  inline void print_legal(position& p) const;
  void reset() { last = 0; }
};

#include "move.hpp"

#endif
