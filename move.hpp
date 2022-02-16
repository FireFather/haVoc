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
#include "move.h"
#include "magics.h"
#include "bits.h"
#include "bitboards.h"
#include "position.h"


// utilities
namespace {
  template<Dir>
  void shift(U64& b);  
  template<>
  void shift<N>(U64& b)  { b <<= 8; }
  template<>
  void shift<S>(U64& b)  { b >>= 8; }
  template<>
  void shift<NN>(U64& b) { b <<= 16; }
  template<>
  void shift<SS>(U64& b) { b >>= 16; }
  template<>
  void shift<NW>(U64& b) { b <<= 7; }
  template<>
  void shift<NE>(U64& b) { b <<= 9; }
  template<>
  void shift<SW>(U64& b) { b >>= 7; }
  template<>
  void shift<SE>(U64& b) { b >>= 9; }  
}


template<Movetype mt>
void Movegen::encode(U64& b, const int& f) {  
  while (b) list[last++].set(f, bits::pop_lsb(b), mt);
}

template<Movetype mt>
void Movegen::encode_pawn_pushes(U64& b, const int& dir) {
  while (b) {
    int to = bits::pop_lsb(b);
    list[last++].set(to+dir, to, mt);
  }
}

inline void Movegen::encode_promotions(U64& b, const int& dir) {
  while (b) {
	  auto to = static_cast<Square>(bits::pop_lsb(b));
	  auto f = static_cast<Square>(to + dir);
    list[last++].set(f, to, promotion_q);
    list[last++].set(f, to, promotion_r);
    list[last++].set(f, to, promotion_b);
    list[last++].set(f, to, promotion_n);
  }
}

inline void Movegen::encode_capture_promotions(U64& b, const int& dir) {
  while (b) {
	  auto to = static_cast<Square>(bits::pop_lsb(b));
	  auto f = static_cast<Square>(to + dir);
    list[last++].set(f, to, capture_promotion_q);
    list[last++].set(f, to, capture_promotion_r);
    list[last++].set(f, to, capture_promotion_b);
    list[last++].set(f, to, capture_promotion_n);
  }
}


inline void Movegen::print() const
{
  for(int j=0; j<last; ++j) {    
    std::cout << SanSquares[list[j].f] << SanSquares[list[j].t] << " ";
  }
  std::cout << "\n";
}

inline void Movegen::print_legal(position& p) const
{
  for(int j=0; j<last; ++j) {    
    if (p.is_legal(list[j]))  std::cout << SanSquares[list[j].f] << SanSquares[list[j].t] << " ";
  }
  std::cout << "\n";
}


//----------------------------------------------
// movegen utilities
//----------------------------------------------

inline void Movegen::initialize(const position& p) {
  us = p.to_move();
  them = static_cast<Color>(us ^ 1);
  all_pieces = p.all_pieces();
  empty = ~all_pieces;

  can_castle_ks = p.can_castle_ks();
  can_castle_qs = p.can_castle_qs();

  check_target = p.checkers();
  evasion_target = 0ULL;
  
  if (check_target != 0ULL && !bits::more_than_one(check_target)) { // only 1 checker
    U64 tmp = check_target;
    auto frm = static_cast<Square>(bits::pop_lsb(tmp));
    Piece checker = p.piece_on(frm);
    if (checker == bishop || checker == rook || checker == queen) {      
      evasion_target = bitboards::between[frm][p.king_square()] & empty;
    }
  }
  
  if (us == white) {
    rank2 = bitboards::row[r2];
    rank7 = bitboards::row[r7];
    pawns = p.get_pieces<white, pawn>();
    knights = p.squares_of<white, knight>();    
    bishops = p.squares_of<white, bishop>();
    rooks = p.squares_of<white, rook>();
    queens = p.squares_of<white, queen>();
    kings = p.squares_of<white, king>();
    enemies = p.get_pieces<black>();
  }
  else {
    rank2 = bitboards::row[r7];
    rank7 = bitboards::row[r2];
    pawns = p.get_pieces<black, pawn>();
    knights = p.squares_of<black, knight>();
    bishops = p.squares_of<black, bishop>();
    rooks = p.squares_of<black, rook>();
    queens = p.squares_of<black, queen>();
    kings = p.squares_of<black, king>();
    enemies = p.get_pieces<white>();
  }

  qtarget = (evasion_target != 0ULL ? evasion_target : empty);
  ctarget = (check_target == 0ULL ? enemies : check_target);

  eps = p.eps();
  pawns2 = pawns & rank2;
  pawns7 = pawns & rank7;
}


inline void Movegen::pawn_quiets(U64& single, U64& dbl) const
{

  if (pawns == 0ULL) return;

  single = pawns & bitboards::pawnmask[us]; // filter the promotion candidates  
  dbl = pawns & rank2;

  auto s = (us == white ? shift<N> : shift<S>);
  
  s(single);
  single &= qtarget;
  
  for (int i=0; i<2; ++i) {
    s(dbl);
    dbl &= empty;
  }
  if (evasion_target != 0ULL) dbl &= evasion_target;
}


inline void Movegen::pawn_caps(U64& left, U64& right, U64& ep_left, U64& ep_right) const
{

  if (pawns == 0ULL) return;
  
  // normal captures - non promotions
  left = pawns & bitboards::pawnmaskleft[us];
  right = pawns & bitboards::pawnmaskright[us];   
  
  auto sw = (us == white ? shift<NW> : shift<SE>);
  auto se = (us == white ? shift<NE> : shift<SW>);
  
  if (left != 0ULL) {
    se(left);
    left &= ctarget;
  }
  
  if (right != 0ULL) {
    sw(right);
    right &= ctarget;
  }

  // ep captures - evasions are checked individually
  if (eps == no_square) return;
  auto r = (us == white ? r5 : r4);
  ep_left = pawns & bitboards::pawnmaskleft[us] & bitboards::row[r]; 
  ep_right = pawns & bitboards::pawnmaskright[us] & bitboards::row[r];
  
  if (ep_left != 0ULL) {
    se(ep_left);
    ep_left &= bitboards::squares[eps];
  }
  
  if (ep_right != 0ULL) {
    sw(ep_right);
    ep_right &= bitboards::squares[eps];
  }    
}

inline void Movegen::quiet_promotions(U64& quiets) const
{  
  if (pawns7 == 0ULL) return;
  quiets = pawns7;

  auto s = (us == white ? shift<N> : shift<S>);
  
  s(quiets);

  quiets &= qtarget;
}

inline void Movegen::capture_promotions(U64& right_caps, U64& left_caps) const
{
  if (pawns7 == 0ULL) return;
  
  right_caps = pawns7 & bitboards::pawnmaskright[us ^ 1];
  left_caps = pawns7 & bitboards::pawnmaskleft[us ^ 1];

  auto sw = (us == white ? shift<NW> : shift<SE>);
  auto se = (us == white ? shift<NE> : shift<SW>);

  if (left_caps) {
    se(left_caps);
    left_caps &= ctarget;
  }

  if (right_caps) {
    sw(right_caps);  
    right_caps &= ctarget;
  }
}

//------------------------------
// pawn moves
//------------------------------

template<>
inline void Movegen::generate<quiet, pawn>() {
  
  U64 single_pushes = 0ULL;
  U64 double_pushes = 0ULL;
  int d1 = (us == white ? -8 : 8);
  int d2 = 2 * d1;

  pawn_quiets(single_pushes, double_pushes);

  if (single_pushes != 0ULL) encode_pawn_pushes<quiet>(single_pushes, d1);
  if (double_pushes != 0ULL) encode_pawn_pushes<quiet>(double_pushes, d2);
}


template<>
inline void Movegen::generate<capture, pawn>() {
  U64 left = 0ULL;
  U64 right = 0ULL;
  U64 ep_left = 0ULL;
  U64 ep_right = 0ULL;
  
  int d1 = (us == white ? -7 : 9);
  int d2 = (us == white ? -9 : 7);

  pawn_caps(left, right, ep_left, ep_right);
  
  if (left != 0ULL) encode_pawn_pushes<capture>(left, d2);
  if (right != 0ULL) encode_pawn_pushes<capture>(right, d1);
  
  if (ep_left != 0ULL) encode_pawn_pushes<ep>(ep_left, d2);  
  if (ep_right != 0ULL) encode_pawn_pushes<ep>(ep_right, d1);  
}

template<>
inline void Movegen::generate<promotion, pawn>() {
  U64 mvs = 0;
  quiet_promotions(mvs);
  if (mvs) encode_promotions(mvs, us == white ? -8 : 8);
}

template<>
inline void Movegen::generate<capture_promotion, pawn>() {
  U64 caps_l = 0;
  U64 caps_r = 0;
  int d1 = (us == white ? -7 : 9);
  int d2 = (us == white ? -9 : 7);
  capture_promotions(caps_r, caps_l);
  if (caps_r != 0ULL) encode_capture_promotions(caps_r, d1);
  if (caps_l != 0ULL) encode_capture_promotions(caps_l, d2);
}


//------------------------------
// knight moves
//------------------------------
template<>
inline void Movegen::generate<quiet, knight>() {
  for (Square * s = &knights[0]; *s != no_square; ++s) {
    U64 mvs = bitboards::nmask[*s] & qtarget;    
    if (mvs != 0ULL) encode<quiet>(mvs, *s);
  }
}

template<>
inline void Movegen::generate<capture, knight>() {
  for (Square * s = &knights[0]; *s != no_square; ++s) {
    U64 mvs = bitboards::nmask[*s] & ctarget;     
    if ( mvs != 0ULL) encode<capture>(mvs, *s);
  }
}

template<>
inline void Movegen::generate<pseudo_legal, knight>() {  
  for (Square * s = &knights[0]; *s != no_square; ++s) {
    U64 qmvs = bitboards::nmask[*s] & qtarget;     
    if (qmvs != 0ULL) encode<quiet>(qmvs, *s);
    
    U64 cmvs = bitboards::nmask[*s] & ctarget;     
    if (cmvs != 0ULL) encode<capture>(cmvs, *s);
  }
}

//------------------------------
// bishop moves
//------------------------------
template<>
inline void Movegen::generate<quiet, bishop>() {
  for (Square * s = &bishops[0]; *s != no_square; ++s) {
    U64 mvs = magics::attacks<bishop>(all_pieces, *s) & qtarget;
    if (mvs != 0ULL) encode<quiet>(mvs, *s);
  }  
}

template<>
inline void Movegen::generate<capture, bishop>() {
  for (Square * s = &bishops[0]; *s != no_square; ++s) {
    U64 mvs = magics::attacks<bishop>(all_pieces, *s) & ctarget;
    if (mvs != 0ULL) encode<capture>(mvs, *s);
  }
}

template<>
inline void Movegen::generate<pseudo_legal, bishop>() {
  for (Square * s = &bishops[0]; *s != no_square; ++s){ 
    U64 mvs = magics::attacks<bishop>(all_pieces, *s);

    U64 q = mvs & qtarget;
    if (q != 0ULL) encode<quiet>(q, *s);
    
    U64 c = mvs & ctarget;
    if (c != 0ULL) encode<capture>(c, *s);
  }
}

//------------------------------
// rook moves
//------------------------------
template<>
inline void Movegen::generate<quiet, rook>() {
  for (Square * s = &rooks[0]; *s != no_square; ++s) {
    U64 mvs = magics::attacks<rook>(all_pieces, *s) & qtarget;
    if (mvs != 0ULL) encode<quiet>(mvs, *s);
  }   
}

template<>
inline void Movegen::generate<capture, rook>() {  
  for (Square * s = &rooks[0]; *s != no_square; ++s) {
    U64 mvs = magics::attacks<rook>(all_pieces, *s) & ctarget;
    if (mvs != 0ULL) encode<capture>(mvs, *s);
  }
}

template<>
inline void Movegen::generate<pseudo_legal, rook>() {  
  for (Square * s = &rooks[0]; *s != no_square; ++s) {
    U64 mvs = magics::attacks<rook>(all_pieces, *s);

    U64 q = mvs & qtarget;
    if (q != 0ULL) encode<quiet>(q, *s);

    U64 c = mvs & ctarget;
    if (c != 0ULL) encode<capture>(c, *s);
  }
}

//------------------------------
// queen moves
//------------------------------
template<>
inline void Movegen::generate<quiet, queen>() {
  for (Square * s = &queens[0]; *s != no_square; ++s) {
    U64 mvs = (magics::attacks<bishop>(all_pieces, *s) |
	       magics::attacks<rook>(all_pieces, *s)) & qtarget;
    if (mvs != 0ULL) encode<quiet>(mvs, *s);
  }   
}

template<>
inline void Movegen::generate<capture, queen>() {

  for (Square * s = &queens[0]; *s != no_square; ++s) {
    U64 mvs = (magics::attacks<bishop>(all_pieces, *s) |
	       magics::attacks<rook>(all_pieces, *s)) & ctarget;
    if (mvs != 0ULL) encode<capture>(mvs, *s);
  }
}

template<>
inline void Movegen::generate<pseudo_legal, queen>() {

  for (Square * s = &queens[0]; *s != no_square; ++s) {
    U64 mvs = (magics::attacks<bishop>(all_pieces, *s) |
	       magics::attacks<rook>(all_pieces, *s));

    U64 q = mvs & qtarget;
    if (q != 0ULL) encode<quiet>(q, *s);

    U64 c = mvs & ctarget;
    if (c != 0ULL) encode<capture>(c, *s);
  }
}

//------------------------------
// king moves
//------------------------------
template<>
inline void Movegen::generate<quiet, king>() {
  U64 mvs = (bitboards::kmask[kings[0]] & empty);
  if (mvs != 0ULL) encode<quiet>(mvs, kings[0]);
}


template<>
inline void Movegen::generate<castles, king>() {
  if (can_castle_ks) list[last++].set(kings[0], (us == white ? G1 : G8), castle_ks);
  if (can_castle_qs) list[last++].set(kings[0], (us == white ? C1 : C8), castle_qs);
}


template<>
inline void Movegen::generate<capture, king>() {  
  U64 mvs = (bitboards::kmask[kings[0]] & enemies);
  if (mvs != 0ULL) encode<capture>(mvs, kings[0]);
}

//------------------------------
// moves by piece type
//------------------------------
template<>
inline void Movegen::generate<king>() {
  generate<capture, king>();
  generate<quiet, king>();
  generate<castles, king>();
}

template<>
inline void Movegen::generate<queen>() {
  generate<pseudo_legal, queen>();
}

template<>
inline void Movegen::generate<rook>() {
  generate<pseudo_legal, rook>();
}

template<>
inline void Movegen::generate<bishop>() {
  generate<pseudo_legal, bishop>();
}

template<>
inline void Movegen::generate<knight>() {
  generate<pseudo_legal, knight>();
}

template<>
inline void Movegen::generate<pawn>() {
  generate<capture_promotion, pawn>();
  generate<promotion, pawn>();
  generate<capture, pawn>();
  generate<quiet, pawn>();
}


//------------------------------
// pseudo-legal quiet
//------------------------------
template<>
inline void Movegen::generate<quiet, pieces>() {
  // todo: order move generation by game phase
  // data-driven approach for this?
  if (check_target == 0ULL) {
    generate<promotion, pawn>();
    generate<quiet, knight>();
    generate<quiet, bishop>();
    generate<quiet, rook>();
    generate<quiet, queen>();
    generate<quiet, pawn>();
    generate<quiet, king>();
    generate<castles, king>();
  }
  else if (check_target != 0ULL && evasion_target == 0ULL) {
    generate<quiet, king>();
  }
  else if (check_target != 0ULL && evasion_target != 0ULL) {
    if (!bits::more_than_one(check_target)) {
      generate<quiet, knight>();
      generate<quiet, bishop>();
      generate<quiet, rook>();
      generate<quiet, queen>();
      generate<promotion, pawn>();
      generate<quiet, pawn>();
    }
    generate<quiet, king>();
  }
  
}

//------------------------------
// pseudo-legal capture
//------------------------------
template<>
inline void Movegen::generate<capture, pieces>() {
  // todo: order move generation by game phase
  // data-driven approach for this?
  if (check_target == 0ULL) {
    generate<capture_promotion, pawn>();
    generate<capture, pawn>();
    generate<capture, knight>();
    generate<capture, bishop>();
    generate<capture, rook>();
    generate<capture, queen>();
    generate<capture, king>();
  }
  else if (check_target != 0ULL && evasion_target == 0ULL) {
    if (!bits::more_than_one(check_target)) {
      generate<capture_promotion, pawn>();
      generate<capture, king>();
      generate<capture, pawn>();
      generate<capture, knight>();
      generate<capture, bishop>();
      generate<capture, rook>();
      generate<capture, queen>();
    }
    else {
      generate<capture, king>(); 
    }
  }
  else if (check_target != 0ULL && evasion_target != 0ULL) {
    if (!bits::more_than_one(check_target)) {
      generate<capture_promotion, pawn>();
      generate<capture, pawn>();
      generate<capture, king>();
      generate<capture, knight>();
      generate<capture, bishop>();
      generate<capture, rook>();
      generate<capture, queen>();
    }
    else {
      generate<capture, king>(); 
    }
  }  
}

//------------------------------
// pseudo-legal all
//------------------------------

template<>
inline void Movegen::generate<pseudo_legal, pieces>() {
  // todo: order move generation by game phase
  // data-driven approach for this?
  if (check_target == 0ULL) { // all moves
    generate<capture_promotion, pawn>();
    generate<promotion, pawn>();
    generate<pseudo_legal, knight>();
    generate<pseudo_legal, bishop>();
    generate<pseudo_legal, rook>();
    generate<pseudo_legal, queen>();
    generate<capture, pawn>();
    generate<quiet, pawn>();
    generate<quiet, king>();
    generate<castles, king>();
    generate<capture, king>();
  }
  else if (check_target != 0ULL && evasion_target == 0ULL) {
    if (!bits::more_than_one(check_target)) {
      generate<capture_promotion, pawn>();
      generate<capture, king>();
      generate<capture, pawn>();
      generate<capture, knight>();
      generate<capture, bishop>();
      generate<capture, rook>();
      generate<capture, queen>();
      generate<quiet, king>();
    }
    else {
      generate<capture, king>(); 
      generate<quiet, king>(); // more than one checker
    }
  }
  else if (check_target != 0ULL && evasion_target != 0ULL) {
    if (!bits::more_than_one(check_target)) {
      // capture moves
      generate<capture_promotion, pawn>();
      generate<capture, king>();
      generate<capture, pawn>();
      generate<pseudo_legal, knight>();
      generate<pseudo_legal, bishop>();
      generate<pseudo_legal, rook>();
      generate<pseudo_legal, queen>();
      
      // blocking moves
      generate<promotion, pawn>();
      generate<quiet, king>();
      generate<quiet, pawn>();
    }
    else {
      generate<capture, king>(); 
      generate<quiet, king>(); // more than one checker
    }
  }
}
