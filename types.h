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
#ifndef TYPES_H
#define TYPES_H

#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <iostream>

#ifdef _MSC_VER
#include <cstdint>
typedef __int16 int16_t;
typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;

typedef int16_t int16;
typedef const uint64_t C64;
typedef const uint32_t C32;
typedef uint64_t U64;
typedef uint32_t U32;
typedef uint16_t U16;
typedef uint8_t U8;

#else
#include <stdint.h>
typedef int16_t int16;
typedef const uint64_t C64;
typedef const uint32_t C32;
typedef uint64_t U64;
typedef uint32_t U32;
typedef uint16_t U16;
typedef uint8_t U8;
#endif

#include "pragma.h"
enum Movetype {
  promotion_q,
  promotion_r,
  promotion_b,
  promotion_n,
  capture_promotion_q,
  capture_promotion_r,
  capture_promotion_b,
  capture_promotion_n,
  castle_ks,
  castle_qs,
  quiet,
  capture,
  ep,
  castles,
  pseudo_legal,
  promotion,
  capture_promotion,
  no_type
};

struct Move {
  U8 f;
  U8 t;
  U8 type;

  Move& operator=(const Move& m) = default;

  bool operator==(const Move& m) const {
    return (f == m.f && t == m.t && type == m.type);
  }

  bool operator!=(const Move& m) const {
    return (f != m.f || t != m.t || type != m.type);
  }

  void set(const U8& frm, const U8& to, const Movetype& mt) {
    f = frm; t = to; type = mt;
  }
};

// enums
enum Piece { pawn, knight, bishop, rook, queen, king, pieces, no_piece };
enum Color { white, black, colors, no_color };
enum Square {
  A1, B1, C1, D1, E1, F1, G1, H1,
  A2, B2, C2, D2, E2, F2, G2, H2,
  A3, B3, C3, D3, E3, F3, G3, H3,
  A4, B4, C4, D4, E4, F4, G4, H4,
  A5, B5, C5, D5, E5, F5, G5, H5,
  A6, B6, C6, D6, E6, F6, G6, H6,
  A7, B7, C7, D7, E7, F7, G7, H7,
  A8, B8, C8, D8, E8, F8, G8, H8,
  squares, no_square
};

enum Row { r1, r2, r3, r4, r5, r6, r7, r8, rows, no_row };
enum Col { A, B, C, D, E, F, G, H, cols, no_col };
enum Score { inf = 10000, ninf = -10000, mate = inf - 1, mated = ninf + 1, mate_max_ply = mate - 64, mated_max_ply = mated + 64, draw = 0 };
enum Nodetype { root, pv, non_pv, searching = 128 };
enum OrderPhase { hash_move, mate_killer1, mate_killer2, good_captures,
		  killer1, killer2, bad_captures, quiets, end };

const std::string SanSquares[64] =
  {
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8"
  };

/*
endgame material encoding reference : kxxk where xx is given by the following encodings
NBRQ  nbrq
0000  0000 (kk)  0
0001  0001 (nn)  17
0001  0010 (nb)  19
0001  0100 (nr)  20
0001  1000 (nq)  24
0010  0001 (bn)  33  
0010  0010 (bb)  35
0010  0100 (br)  39
0010  1000 (bq)  40
0100  0001 (rn)  65
0100  0010 (rb)  66
0100  0100 (rr)  68
0100  1000 (rq)  72
1000  0001 (qn)  129
1000  0010 (qb)  130
1000  0100 (qr)  132
1000  1000 (qq)  136
*/
enum EndgameType {
  none = -1,
  KpK = 0,
  KnnK = 17,
  KnbK = 19,
  KnrK = 20,
  KnqK = 24,
  KbnK = 33,
  KbbK = 35,
  KbrK = 39,
  KbqK = 40,
  KrnK = 65,
  KrbK = 66,
  KrrK = 68,
  KrqK = 72,
  KqnK = 129,
  KqbK = 130,
  KqrK = 132,
  KqqK = 136
};

// enum type enabled iterators
template<typename T> struct is_enum : std::false_type {};
template<> struct is_enum<Piece> : std::true_type {};
template<> struct is_enum<Color> : std::true_type {};
template<> struct is_enum<Square> : std::true_type {};
template<> struct is_enum<Row> : std::true_type {};
template<> struct is_enum<Col> : std::true_type {};
template<> struct is_enum<OrderPhase> : std::true_type{};

template<typename T, 
  typename = typename std::enable_if<is_enum<T>::value>::type >
  int operator++(T& e) {
  e = T(static_cast<int>(e) + 1); return static_cast<int>(e);
}

template<typename T, 
  typename = typename std::enable_if<is_enum<T>::value>::type >
  int operator--(T& e) {
  e = T(static_cast<int>(e) - 1); return static_cast<int>(e);
}  


template<typename T1, typename T2,
  typename = typename std::enable_if<is_enum<T1>::value>::type >
  T1& operator-=(T1& lhs, const T2& rhs) {
  lhs = T1(lhs - rhs); return lhs;
}

template<typename T1, typename T2,
  typename = typename std::enable_if<is_enum<T1>::value>::type >
  T1& operator+=(T1& lhs, const T2& rhs) {
  lhs = T1(lhs + rhs); return lhs;
}
  
  
// arrays for iteration
const std::vector<Piece> Pieces {pawn, knight, bishop, rook, queen, king, pieces, no_piece};
const std::vector<Color> Colors {white, black, colors, no_color};
const std::vector<char> SanPiece{'P','N','B','R','Q','K','p','n','b','r','q','k'};
const std::vector<char> SanCols{'a','b','c','d','e','f','g','h'};
enum CastleRight { wks = 1, wqs = 2, bks = 4, bqs = 8, cr_none = 0, clearbqs = 7, clearbks = 11, clearwqs = 13, clearwks = 14, clearb = 3, clearw = 12 };
const std::map<char, U16> CastleRights {{'K', wks}, {'Q', wqs}, {'k', bks}, {'q', bqs}, {'-', 0}, {'\0', 0}, {' ', 0}};
#endif
