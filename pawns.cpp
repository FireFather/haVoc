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
#include <vector>

#include "pawns.h"
#include "types.h"
#include "utils.h"
#include "bitboards.h"
#include "squares.h"
#include "evaluate.h"

pawn_table ptable;

template<Color c>
int16 evaluate(const position& p, pawn_entry& e);

inline size_t pow2(size_t x) {
  return x <= 2 ? x : pow2(x >> 1) << 1;
}

pawn_table::pawn_table() : sz_mb(0), count(0) {
  init();
}



void pawn_table::init() {
  sz_mb = 10 * 1024; // todo : input mb parameter
  count = 1024 * sz_mb / sizeof(pawn_entry);
  count = pow2(count);
  count = (count < 1024 ? 1024 : count);
  entries = std::unique_ptr<pawn_entry[]>(new pawn_entry[count]());
  clear();
}



void pawn_table::clear() const
{
  memset(entries.get(), 0, count * sizeof(pawn_entry));
}



pawn_entry * pawn_table::fetch(const position& p) const
{
  U64 k = p.pawnkey();
  unsigned idx = k & (count - 1);
  if (entries[idx].key == k) {
    return &entries[idx];
  }
  std::memset(&entries[idx], 0, sizeof(pawn_entry));
  entries[idx].key = k;
  entries[idx].score = evaluate<white>(p, entries[idx]) - evaluate<black>(p, entries[idx]);
  return &entries[idx];
}


template<Color c>
bool backward_pawn(const int& row, const int& col, const U64& pawns) { 
  int left = col - 1 < A ? -1 : col - 1;
  int right = col + 1 > H ? -1 : col + 1;
  bool left_greater = false;
  bool right_greater = false;

  if (c == white) {
    if (left != -1) {
      int sq = -1;
      U64 left_pawns = bitboards::col[left] & pawns;
      while (left_pawns) {
        int tmp = bits::pop_lsb(left_pawns);
        if (tmp > sq) sq = tmp;
      }
      left_greater = sq > 0 && util::row(sq) > row;
    }

    if (right != -1) {
      int sq = -1;
      U64 right_pawns = bitboards::col[right] & pawns;
      bool no_right_pawns = (right_pawns == 0ULL);
      while (right_pawns) {
        int tmp = bits::pop_lsb(right_pawns);
        if (tmp > sq) sq = tmp;
      }
      right_greater = sq > 0 && util::row(sq) > row || no_right_pawns;
    }
  }
  else {
    if (left != -1) {
      int sq = 100;
      U64 left_pawns = bitboards::col[left] & pawns;
      bool no_left_pawns = (left_pawns == 0ULL);
      while (left_pawns) {
        int tmp = bits::pop_lsb(left_pawns);
        if (tmp < sq) sq = tmp;
      }
      left_greater = sq < 100 && util::row(sq) < row || no_left_pawns;
    }
    if (right != -1) {
      int sq = 100;
      U64 right_pawns = bitboards::col[right] & pawns;
      bool no_right_pawns = (right_pawns == 0ULL);
      while (right_pawns) {
        int tmp = bits::pop_lsb(right_pawns);
        if (tmp < sq) sq = tmp;
      }
      right_greater = sq < 100 && util::row(sq) < row || no_right_pawns;
    }
  }

  return (left == -1 && right_greater) ||
    (right == -1 && left_greater) ||
    (left_greater && right_greater);
}

template<Color c>
int16 evaluate(const position& p, pawn_entry& e) {

  // sq score scale factors by column
  std::vector<float> pawn_scaling { 0.86f, 0.90f, 0.95f, 1.00f, 1.00f, 0.95f, 0.90f, 0.86f };
  std::vector<float> material_vals { 100.0f, 300.0f, 315.0f, 480.0f, 910.0f };

  auto them = static_cast<Color>(c ^ 1);

  U64 pawns = p.get_pieces<c, pawn>(); 
  U64 epawns = them == white ?
    p.get_pieces<white, pawn>() :
    p.get_pieces<black, pawn>();
  
  Square * sqs = p.squares_of<c, pawn>();

  Square ksq = p.king_square(c);

  int16 score = 0;
  U64 locked_bb = 0ULL;
  
  for (Square s = *sqs; s != no_square; s = *++sqs) {

    U64 fbb = bitboards::squares[s];
    int row = util::row(s);
    int col = util::col(s);

    score += p.params.sq_score_scaling[pawn] * square_score<c>(pawn, s);
    score += pawn_scaling[col] * material_vals[pawn];

    
    // pawn attacks
    e.attacks[c] |= bitboards::pattks[c][s];

    // king shelter
    if ((bitboards::kmask[ksq] & fbb)) {
      e.king[c] |= fbb;
    }
    
    // passed pawns
    U64 mask = bitboards::passpawn_mask[c][s] & epawns;  
    if (mask == 0ULL) { 
      e.passed[c] |= fbb; 
      e.score += p.params.passed_pawn_bonus;
    }

    // isolated pawns
    U64 neighbors_bb = bitboards::neighbor_cols[col] & pawns;
    if (neighbors_bb == 0ULL) {
      e.isolated[c] |= fbb;
      score -= p.params.isolated_pawn_penalty;
    }

    // backward
    if (backward_pawn<c>(row, col, pawns)) {
      e.backward[c] |= fbb;
      score -= p.params.backward_pawn_penalty;
    }



    // pawns by square color
    U64 wsq = bitboards::colored_sqs[white] & fbb;
    if (wsq) e.light[c] |= fbb;
    U64 bsq = bitboards::colored_sqs[black] & fbb;
    if (bsq) e.dark[c] |= fbb;

    // doubled pawns
    U64 doubled = bitboards::col[col] & pawns;
    if (bits::more_than_one(doubled)) {
      e.doubled[c] |= doubled;
      if (e.isolated[c] & doubled) {
        score -= 2 * p.params.doubled_pawn_penalty;

      }
      else score -= p.params.doubled_pawn_penalty;

    }


    // semi-open file pawns
    U64 column = bitboards::col[col];
    if ((column & epawns) == 0ULL) {
      e.semiopen[c] |= fbb;

      if ((fbb & e.backward[c])) {
        score -= 2 * p.params.backward_pawn_penalty;
      }

      if ((fbb & e.isolated[c])) {
        score -= 2 * p.params.semi_open_pawn_penalty;
      }
    }

    // track king/queen side pawn configurations
    if (util::col(s) <= D) e.qsidepawns[c] |= fbb;
    else e.ksidepawns[c] |= fbb;

    // .. locked center pawns
    // count nb of center pawns while computing this too
    // e.g. french advanced, caro-kahn advanced, 4-pawns attack in KID etc.
    // favors flank attacks, knights, and small penalties for bishop
    if ((bitboards::squares[s] & bitboards::big_center_mask)) {
	    auto front_sq = static_cast<Square>(c == white ? s + 8 : s - 8);
      if (util::on_board(front_sq)) {
        U64 fbb = bitboards::squares[front_sq];
        e.center_pawn_count++;
        if ((epawns & fbb)) {
          locked_bb |= fbb;
        }

      }
    }

    // pawn islands
    // pawn chain tips
    // pawn chain bases
    // pawn majorities
  }
  
  // note : evaluated 2x's when we only need to evaluate once
  // since it is the same per side - but the performance hit should be small
  if (bits::count(locked_bb) >= 2) {
    e.locked_center = true;
  }

  return score;  
}
