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

#ifndef POSITION_H
#define POSITION_H

#include <thread>
#include <algorithm>
#include <vector>
#include <iostream>
#include <array>
#include <string>
#include <sstream>
#include <cstring>
#include <memory>

#include "types.h"
#include "utils.h"
#include "bitboards.h"
#include "magics.h"
#include "zobrist.h"
#include "order.h"
#include "parameter.h" // just for parameter reference (todo: refactor)

struct Move;


struct info {
  U64 checkers;
  U64 pinned[2];
  U64 key;
  U64 mkey;
  U64 pawnkey;
  U64 repkey;
  U16 hmvs;
  U16 cmask;
  U8 move50;
  Color stm;
  Square eps;
  Square ks[2];
  bool has_castled[2];
  Piece captured;    
  bool incheck;
};


struct piece_data {  
  
  std::array<U64, 2> bycolor;
  std::array<Square, 2> king_sq;
  std::array<Color, squares> color_on;
  std::array<Piece, squares> piece_on;
  std::array<std::array<int, pieces>, 2> number_of;
  std::array<std::array<U64, squares>, colors> bitmap;
  std::array<std::array<std::array<int, squares>, pieces>, 2> piece_idx;
  std::array<std::array<std::array<Square, 11>, pieces>, 2> square_of;

  piece_data(): bycolor(), king_sq(), color_on(), piece_on(), number_of(), bitmap(), piece_idx(), square_of()
  {
  } ;
  piece_data(const piece_data& pd);
  piece_data& operator=(const piece_data& pd);
  
  // utility methods for moving pieces
  void clear();

  void set(const Color& c, const Piece& p, const Square& s, info& ifo);
  
  inline void do_quiet(const Color& c, const Piece& p, const Square& f, const Square& t, info& ifo);

  template<Color c>
  void do_castle(const bool& kingside);
  
  template<Color c>
  void undo_castle(const bool& kingside);

  inline void do_cap(const Color& c, const Piece& p, const Square& f, const Square& t, info& ifo);
  
  inline void do_ep(const Color& c, const Square& f, const Square& t, info& ifo);

  inline void do_promotion(const Color& c, const Piece& p,
			   const Square& f, const Square& t, info& ifo);
	     
  inline void do_promotion_cap(const Color& c,
			       const Piece& p, const Square& f, const Square& t, info& ifo);

  inline void do_castle_ks(const Color& c, const Square& f, const Square& t, info& ifo);
  
  inline void do_castle_qs(const Color& c, const Square& f, const Square& t, info& ifo);
  
  inline void remove_piece(const Color& c, const Piece& p, const Square& s, info& ifo);
  
  inline void add_piece(const Color& c, const Piece& p, const Square& s, info& ifo);
};


class position {  
  U16 thread_id{};
  info history[512]{};
  move_history stats;
  info ifo{};
  piece_data pcs;
  U64 hidx{};
  U64 nodes_searched{};
  U64 qnodes_searched{};
  
 public:
  position(): thread_id(0), history{}, ifo(), hidx(0), nodes_searched(0), qnodes_searched(0), elapsed_ms(0)
  {
  }

  position(std::istringstream& fen);
  position(const std::string& fen);
  position(const position& p, const std::thread& t);
  position(const position& p);
  position(const position&& p);
  position& operator=(const position& p);
  position& operator=(const position&&);
  ~position() = default;

  double elapsed_ms{};
  std::string bestmove;
  parameters params; // reference to our tuneable parameters
  bool debug_search = false;

  // setup/clear a position
  void setup(std::istringstream& fen);
  std::string to_fen();
  void clear();
  void set_piece(const char& p, const Square& s);
  void print() const;
  void do_move(const Move& m);
  void undo_move(const Move& m);
  void do_null_move();
  void undo_null_move();
  int see_move(const Move& m);
  int see(const Move& m);

  void stats_update(const Move& m,
                    const Move& previous, 
                    const int16& depth,
                    const Score& score,
                    const std::vector<Move>& quiets,
                    Move * killers) {
    stats.update(*this, m, previous, depth, score, quiets, killers);
  }
  move_history& history_stats() { return stats; }

  // utilities
  bool is_attacked(const Square& s, const Color& us, const Color& them, U64 m = 0ULL);
  U64 attackers_of2(const Square& s, const Color& c) const;
  U64 attackers_of(const Square& s, const U64& bb);
  U64 checkers() const { return ifo.checkers; }
  bool in_check() const;
  bool is_legal(const Move& m);
  bool is_legal_hashmove(const Move& m);
  U64 pinned(Color us);
  bool is_draw() const;

  bool can_castle_ks() const {
    return (ifo.cmask & (ifo.stm == white ? wks : bks)) == (ifo.stm == white ? wks : bks);
  }

  bool can_castle_qs() const {
    return (ifo.cmask & (ifo.stm == white ? wqs : bqs)) == (ifo.stm == white ? wqs : bqs);
  }

  template<Color c>
  bool can_castle_ks() const {
    return (ifo.cmask & (c == white ? wks : bks) == ( c == white ? wks : bks) );
  }

  template<Color c>
  bool can_castle_qs() const {
    return (ifo.cmask & (c == white ? wqs : bqs) == (c == white ? wks : bks) );
  }

  template<Color c>
  bool can_castle() const {
    return can_castle_ks<c>() || can_castle_qs<c>();
  }

  template<Color c>
  bool has_castled() const { return ifo.has_castled[c]; }

  template<Color c>
  U64 pinned() const { return ifo.pinned[c];  }

  // returns true if there are pawns on the 7th 
  // rank for either side (hint to search not to aggressively reduce search depth)
  bool pawns_near_promotion() const {
    return 
      (get_pieces<white, pawn>() & bitboards::row[r7] != 0ULL) ||
      (get_pieces<black, pawn>() & bitboards::row[r2] != 0ULL);
  }

  // returns true if there are pawns on the 7th rank
  // for the side to move
  bool pawns_on_7th() const
  {
    return ifo.stm == white ? (get_pieces<white, pawn>() & bitboards::row[r7] != 0ULL) :
      (get_pieces<black, pawn>() & bitboards::row[r2] != 0ULL);
  }

  template<Color c>
  bool non_pawn_material() const {
    return ((get_pieces<c, knight>() |
      get_pieces<c, bishop>() |
      get_pieces<c, rook>() |
      get_pieces<c, queen>()) != 0ULL);
  }


  // position info access wrappers
  Square eps() const { return ifo.eps; }
  Color to_move() const { return ifo.stm; }
  U64 key() const { return ifo.key; }
  U64 repkey() const { return ifo.repkey; }
  U64 pawnkey() const { return ifo.pawnkey; }
  U64 material_key() const { return ifo.mkey; }
  // piece access wrappers
  U64 all_pieces() const { return pcs.bycolor[white] | pcs.bycolor[black]; }

  unsigned number_of(const Color& c, const Piece& p) const { return pcs.number_of[c][p]; }

  Piece piece_on(const Square& s) const { return pcs.piece_on[s]; }

  Square king_square(const Color& c) const { return ifo.ks[c]; }

  Square king_square() const { return ifo.ks[ifo.stm]; }

  Color color_on(const Square& s) { return pcs.color_on[s]; }

  U16 id() const { return thread_id; }

  void set_id(U16 id) { thread_id = id; }
  void set_nodes_searched(U64 n) { nodes_searched = n;  }

  void set_qnodes_searched(U64 qn) { qnodes_searched = qn; }

  static bool is_promotion(const U8& mt);

  bool is_master() const { return thread_id == 0; }

  U64 nodes() const { return nodes_searched; }
  U64 qnodes() const { return qnodes_searched; }
  void adjust_nodes(const U64& dn) { nodes_searched += dn; }
  void adjust_qnodes(const U64& dn) { qnodes_searched += dn; }
  
  template<Color c, Piece p>
  U64 get_pieces() const { return pcs.bitmap[c][p]; }

  template<Color c>
  U64 get_pieces() const { return pcs.bycolor[c]; }

  template<Color c, Piece p>
  Square * squares_of() const {
    return const_cast<Square*>(pcs.square_of[c][p].data()+1);
  }
  
};


inline void piece_data::clear() {
  std::fill(bycolor.begin(), bycolor.end(), 0);
  std::fill(king_sq.begin(), king_sq.end(), no_square);
  std::fill(color_on.begin(), color_on.end(), no_color);
  std::fill(piece_on.begin(), piece_on.end(), no_piece);

  for (auto& v: number_of) std::fill(v.begin(), v.end(), 0);
  for (auto& v : bitmap) std::fill(v.begin(), v.end(), 0ULL);
  for (auto& v: piece_idx) { for (auto& w : v) { std::fill(w.begin(), w.end(), 0); } }
  for (auto& v: square_of) { for (auto& w : v) { std::fill(w.begin(), w.end(), no_square); } }
}

inline void piece_data::do_quiet(const Color& c, const Piece& p,
				 const Square& f, const Square& t, info& ifo) {

  // bitmaps
  U64 fto = bitboards::squares[f] | bitboards::squares[t];

  int idx = piece_idx[c][p][f];
  piece_idx[c][p][f] = 0;
  piece_idx[c][p][t] = idx;
  
  bycolor[c] ^= fto;
  bitmap[c][p] ^= fto;
  
  square_of[c][p][idx] = t;
  color_on[f] = no_color;
  color_on[t] = c;

  piece_on[t] = p;
  piece_on[f] = no_piece;
  
  ifo.key = ifo.key ^ zobrist::piece(f, c, p);
  ifo.key = ifo.key ^ zobrist::piece(t, c, p);

  ifo.repkey = ifo.repkey ^ zobrist::piece(f, c, p);
  ifo.repkey = ifo.repkey ^ zobrist::piece(t, c, p);

  if (p == pawn) {
    ifo.pawnkey = ifo.pawnkey ^ zobrist::piece(f, c, p);
    ifo.pawnkey = ifo.pawnkey ^ zobrist::piece(t, c, p);
  }
}

inline void piece_data::do_cap(const Color& c, const Piece& p,
			       const Square& f, const Square& t, info& ifo) {
	auto them = static_cast<Color>(c ^ 1);
  Piece cap = piece_on[t];
  remove_piece(them, cap, t, ifo);
  do_quiet(c, p, f, t, ifo);
}

inline void piece_data::do_promotion(const Color& c, const Piece& p,
				     const Square& f, const Square& t, info& ifo) {
  remove_piece(c, pawn, f, ifo);
  add_piece(c, p, t, ifo);
}
  
inline void piece_data::do_ep(const Color& c, const Square& f, const Square& t, info& ifo) {
	auto them = static_cast<Color>(c ^ 1);
	auto cs = static_cast<Square>(them == white ? t + 8 : t - 8);
  remove_piece(them, pawn, cs, ifo);
  do_quiet(c, pawn, f, t, ifo);
}

inline void piece_data::do_promotion_cap(const Color& c, const Piece& p,
					 const Square& f, const Square& t, info& ifo) {
	auto them = static_cast<Color>(c ^ 1);
  Piece cap = piece_on[t];
  remove_piece(them, cap, t, ifo);
  remove_piece(c, pawn, f, ifo);
  add_piece(c, p, t, ifo);
}

inline void piece_data::do_castle_ks(const Color& c, const Square& f, const Square& t, info& ifo) {
  Square rf = (c == white ? H1 : H8);
  Square rt = (c == white ? F1 : F8);
  
  do_quiet(c, king, f, t, ifo);
  do_quiet(c, rook, rf, rt, ifo);
}

inline void piece_data::do_castle_qs(const Color& c, const Square& f, const Square& t, info& ifo) {
  Square rf = (c == white ? A1 : A8);
  Square rt = (c == white ? D1 : D8);
  
  do_quiet(c, king, f, t, ifo);
  do_quiet(c, rook, rf, rt, ifo);    
}

inline void piece_data::remove_piece(const Color& c, const Piece& p, const Square& s, info& ifo) {
  U64 sq = bitboards::squares[s];
  bycolor[c] ^= sq;
  bitmap[c][p] ^= sq;

  // carefully remove this piece so when we add it back in undo, we
  // do not overwrite an existing piece index
  int tmp_idx = piece_idx[c][p][s];
  int max_idx = number_of[c][p];
  Square tmp_sq = square_of[c][p][max_idx];
  square_of[c][p][tmp_idx] = square_of[c][p][max_idx];
  square_of[c][p][max_idx] = no_square;
  piece_idx[c][p][tmp_sq] = tmp_idx;
  number_of[c][p] -= 1;
  piece_idx[c][p][s] = 0;
  color_on[s] = no_color;
  piece_on[s] = no_piece;
  ifo.key ^= zobrist::piece(s, c, p);
  ifo.mkey ^= zobrist::piece(s, c, p);
  ifo.repkey ^= zobrist::piece(s, c, p);
  if (p == pawn) ifo.pawnkey ^= zobrist::piece(s, c, p);
}

inline void piece_data::add_piece(const Color& c, const Piece& p, const Square& s, info& ifo) {  
  U64 sq = bitboards::squares[s];
  bycolor[c] |= sq;
  bitmap[c][p] |= sq;
  
  number_of[c][p] += 1;
  square_of[c][p][number_of[c][p]] = s;
  piece_on[s] = p;
  piece_idx[c][p][s] = number_of[c][p];
  color_on[s] = c;
  ifo.key ^= zobrist::piece(s, c, p);
  ifo.mkey ^= zobrist::piece(s, c, p);
  ifo.repkey ^= zobrist::piece(s, c, p);
  if (p == pawn) ifo.pawnkey ^= zobrist::piece(s, c, p);
}

inline void piece_data::set(const Color& c, const Piece& p, const Square& s, info& ifo) {
  bitmap[c][p] |= bitboards::squares[s];
  bycolor[c] |= bitboards::squares[s];
  color_on[s] = c;
  number_of[c][p] += 1;
  piece_idx[c][p][s] = number_of[c][p];
  square_of[c][p][number_of[c][p]] = s;
  piece_on[s] = p;
  if (p == king) king_sq[c] = s;

  ifo.key ^= zobrist::piece(s, c, p);
  ifo.mkey ^= zobrist::piece(s, c, p);
  ifo.repkey ^= zobrist::piece(s, c, p);
  if (p == pawn) ifo.pawnkey ^= zobrist::piece(s, c, p);
}

#endif
