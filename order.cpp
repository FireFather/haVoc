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
#include "order.h"
#include "move.h"
#include "position.h"

move_history& move_history::operator=(const move_history& mh) {  
  std::copy(std::begin(mh.history), std::end(mh.history), std::begin(history));
  std::copy(std::begin(mh.counters), std::end(mh.counters), std::begin(counters));
  return (*this);
}

void move_history::update(const position& p,
			  const Move& m,
			  const Move& previous,
			  const int16& depth,
			  const Score& eval,
			  const std::vector<Move>& quiets,
			  Move * killers) {
  
  const Color c = p.to_move();
  int score = pow(depth, 2);
  
  if (m.type == quiet) {

    history[c][m.f][m.t] += score;
    counters[previous.f][previous.t] = m;

    if (eval >= mate_max_ply &&
      m != killers[0] &&
      m != killers[1] &&
      m != killers[2]) {
      killers[3] = killers[2];
      killers[2] = m;
    }
    else if (eval < mate_max_ply &&
      m != killers[2] &&
      m != killers[3] &&
      m != killers[0]) {
      killers[1] = killers[0];
      killers[0] = m;
    }
    
  }

  if (previous.type == quiet) {
    history[c ^ 1][previous.f][previous.t] -= score;
  }  
  //
  //for (auto& q : quiets) {
  //  if (m.f == q.f) continue;
  //  history[c][q.f][q.t] -= score;
  //}
  
}



void move_history::clear() { 
  for (auto& v : history) { for (auto& w : v) { std::fill(w.begin(), w.end(), 0); } }

  Move empty{}; empty.set(0, 0, no_type);
  for (auto& v : counters) { std::fill(v.begin(), v.end(), empty); }
}


template<>
int move_history::score<white>(const Move& m, const Move& previous, const Move& followup, const Move& threat) {
  int score = history[white][m.f][m.t];
  if (counters[previous.f][previous.t] == m) score += counter_move_bonus;
  if (followup.t == m.f) score -= counter_move_bonus; // moving the same piece multiple times..
  if (m.f == threat.t) score += threat_evasion_bonus;
  if (m.type == promotion_q) score += 81; // ordering material values - pawn value
  if (m.type == promotion_r) score += 38;
  if (m.type == promotion_b) score += 25;
  if (m.type == promotion_n) score += 20;
  return score;
}

template<>
int move_history::score<black>(const Move& m, const Move& previous, const Move& followup, const Move& threat) {
  int score = history[black][m.f][m.t];
  if (counters[previous.f][previous.t] == m) score += counter_move_bonus;
  if (followup.t == m.f) score -= counter_move_bonus; // moving the same piece multiple times..
  if (m.f == threat.t) score += threat_evasion_bonus;
  if (m.type == promotion_q) score += 81; // ordering material values - pawn value
  if (m.type == promotion_r) score += 38;
  if (m.type == promotion_b) score += 25;
  if (m.type == promotion_n) score += 20;
  return score;
}



scored_move& scored_move::operator=(const scored_move& o) = default;


move_order::move_order(position& p,
                       Move& hashmv,
                       Move * kill) :
  phase(hash_move), hashmove(&hashmv),
  killers(kill), to_move(p.to_move()), incheck(p.in_check()) {
  
  if (!p.is_legal_hashmove(hashmv)) { hashmove->f = static_cast<Square>(0); hashmove->t = static_cast<Square>(0); hashmove->type = no_type; }
  
  stats = &(p.history_stats());
  stats->counter_move_bonus = p.params.counter_move_bonus;
  stats->threat_evasion_bonus = p.params.threat_evasion_bonus;
  moves = new Movegen(p);
  list.clear();
}

move_order::~move_order() {
  if (moves) { delete moves; moves = nullptr; }
}


bool move_order::skip(const Move& m) const
{
  if (m.type == no_type) return true;
  
  if (m == *hashmove) return true;

  
  if ((m == killers[0] || m == killers[1] ||
       m == killers[2] || m == killers[3]))
    return true;
  

  return false;
}

void move_order::sort() {
  unsigned N = list.size();  
  scored_move key({}, static_cast<Score>(0));

  for (unsigned i = 1; i < N; ++i) {
    key = list[i]; 
    int j = i - 1;  
    while (j >= 0 && list[j] > key) {
      list[j+1] = list[j];
      --j;
    }
    list[j+1] = key; 
  }

}



template<>
bool move_order::next_move<main0>(position& pos, Move& m, const Move& previous, const Move& followup, const Move& threat) {

  m = {};
  m.type = no_type;
  std::vector<int> mvals { 10, 30, 35, 48, 91, 200 };
  
  switch (phase) {
    
  case hash_move: {
    if (hashmove->type != no_type) {
      m = *hashmove;
    }
    ++phase;
    break;
  }

    
  case mate_killer1 : {
    if (pos.is_legal_hashmove(killers[2]) &&
      killers[2] != *hashmove) {
      m = killers[2];
    }
    ++phase; break;
  }
                      
    
  case mate_killer2: {
    if (pos.is_legal_hashmove(killers[3])) {
      if (killers[3] != *hashmove && killers[3] != killers[2])
        m = killers[3];
    }
    ++phase; break;
  }

    
  case good_captures: {
    if (list.empty()) {
      
      moves->generate<capture, pieces>();
      for (int i = 0; i < moves->size(); ++i) {
        
        if (skip((*moves)[i])) continue;
        
        Piece pt = pos.piece_on(static_cast<Square>((*moves)[i].t));
        Piece pf = pos.piece_on(static_cast<Square>((*moves)[i].f));
        
        Score sc = ((*moves)[i].type == ep) ? static_cast<Score>(0) :
          //Score(pos.see((*moves)[i]));
          static_cast<Score>(mvals[pt] - mvals[pf]);

        // promotions
        if (m.type == capture_promotion_q) sc = static_cast<Score>(sc + 81); // ordering material values - pawn value
        if (m.type == capture_promotion_r) sc = static_cast<Score>(sc + 38);
        if (m.type == capture_promotion_b) sc = static_cast<Score>(sc + 25);
        if (m.type == capture_promotion_n) sc = static_cast<Score>(sc + 20);
        
        list.emplace_back(scored_move((*moves)[i], sc));
      }
      
      sort();
      
      if (list.empty()) { ++phase; }
    }
    else {
      if (list.back().s >= static_cast<Score>(0)) {
        m = list.back().m;
        list.pop_back();
      }

      if (!list.empty() && list.back().s < static_cast<Score>(0)) { ++phase; }
      if (list.empty()) { moves->reset(); ++phase; }      
    }    
    break;
  }


  case killer1 : {
    if (pos.is_legal_hashmove(killers[0])) {
      if (killers[0] != *hashmove)
        m = killers[0];
    }
    ++phase; break;
  }
                 
                 
  case killer2: {
    if (pos.is_legal_hashmove(killers[1])) {
      if ((killers[1] != *hashmove) && killers[1] != killers[0])
        m = killers[1];
    }
    ++phase; break;
  }
    
    
  case bad_captures: {
    if (list.empty()) { moves->reset(); ++phase; return true; }
    m = list.back().m;
    list.pop_back();
    break;
  }
    

  case quiets : {
    if (list.empty()) {
      
      auto ss = [this](const Move& m, const Move& previous, const Move& followup, const Move& threat) {
        return static_cast<Score>(to_move == white ? stats->score<white>(m, previous, followup, threat) : stats->score<black>(m, previous, followup, threat)); };
      
      moves->generate<quiet, pieces>();
      for (int i = 0; i < moves->size(); ++i) {

        if (skip((*moves)[i])) { continue; }
        
        Score sc = ss((*moves)[i], previous, followup, threat);
        

        list.emplace_back(scored_move((*moves)[i], sc));
        
      }
      sort();
    }

    if (list.empty()) { moves->reset(); ++phase; }
    else {
      m = list.back().m;
      list.pop_back();
      if (list.empty()) { moves->reset(); ++phase; return true; } 
    }
    break;
  }
    
  case end : { return false; }
    
  } // end switch
  
  return phase != end;
}




template<>
bool move_order::next_move<qsearch>(position& pos, Move& m, const Move& previous, const Move& followup, const Move& threat) {
  
  m = {};
  m.type = no_type;
  std::vector<int> mvals { 10, 30, 35, 48, 91, 200 };
  
  switch (phase) {
    
  case hash_move: {
    if (incheck ||
      hashmove->type == capture ||
      hashmove->type == ep ||
      hashmove->type == capture_promotion_q ||
      hashmove->type == capture_promotion_r ||
      hashmove->type == capture_promotion_b ||
      hashmove->type == capture_promotion_n) {
      m = *hashmove;
    }
    ++phase;
    break;
  }

                  
  case mate_killer1 : {    
    if (pos.is_legal_hashmove(killers[2]) &&
      killers[2] != *hashmove) {
      m = killers[2];
    }    
    ++phase; break;
  }
    

  case mate_killer2 : {    
    if (pos.is_legal_hashmove(killers[3])) {
      if (killers[3] != *hashmove && killers[3] != killers[2])
        m = killers[3];
    }    
    ++phase; break;
  }
  
    
  case good_captures : {
    if (list.empty()) {
      
      moves->generate<capture, pieces>();
      for (int i = 0; i < moves->size(); ++i) {
	
        if (skip((*moves)[i])) continue;
	
        Piece pt = pos.piece_on(static_cast<Square>((*moves)[i].t));
        Piece pf = pos.piece_on(static_cast<Square>((*moves)[i].f));

        //Score sc = Score(pos.see((*moves)[i]));
		
        Score sc = ((*moves)[i].type == ep) ? static_cast<Score>(0) :
          static_cast<Score>(mvals[pt] - mvals[pf]);
        
        // promotions
        if (m.type == capture_promotion_q) sc = static_cast<Score>(sc + 81); // ordering material values - pawn value
        if (m.type == capture_promotion_r) sc = static_cast<Score>(sc + 38);
        if (m.type == capture_promotion_b) sc = static_cast<Score>(sc + 25);
        if (m.type == capture_promotion_n) sc = static_cast<Score>(sc + 20);

        list.emplace_back(scored_move((*moves)[i], sc));
      }
      
      sort();
      
      
      if (list.empty()) { ++phase; }
    }
    else {
      if (list.back().s >= static_cast<Score>(0)) {
        m = list.back().m;
        list.pop_back();
      }
      
      if (!list.empty() && list.back().s < static_cast<Score>(0)) { ++phase; }
      if (list.empty()) { moves->reset(); ++phase; }
    }
    
    break;
  }

    
  case killer1 : {   
    if (incheck && pos.is_legal_hashmove(killers[0])) {
      if (killers[0].f != hashmove->f || killers[0].t != hashmove->t)
        m = killers[0];
    }
    ++phase;
    break;    
  }
    
    
  case killer2 : {
    if (incheck && pos.is_legal_hashmove(killers[1])) {
      if ((killers[1].f != hashmove->f || killers[1].t != hashmove->t) &&
        (killers[1].f != killers[0].f || killers[1].t != killers[0].t))
        m = killers[1];
    }
    ++phase;
    break;
  }   

    
  case bad_captures: {
    if (list.empty()) { moves->reset(); ++phase; return true; }
    m = list.back().m;
    list.pop_back();
    break;
  }

    
  case quiets : {
    if (list.empty() && incheck) {
      
      auto ss = [this](const Move& m, const Move& previous, const Move& followup, const Move& threat) {
		  return static_cast<Score>(to_move == white ? stats->score<white>(m, previous, followup, threat) : stats->score<black>(m, previous, followup, threat)); };
      
      moves->generate<quiet, pieces>();
      for (int i = 0; i < moves->size(); ++i) {
        
        if (skip((*moves)[i])) continue;
        
        Score sc = ss((*moves)[i], previous, followup, threat);
        
        list.emplace_back(scored_move((*moves)[i], sc));

      }
      sort();
    }

    if (list.empty()) { moves->reset(); ++phase; }
    else {
      m = list.back().m;
      list.pop_back();
      if (list.empty()) { moves->reset(); ++phase; return true; } 
    }
    break;
  }

  case end : { return false; }
    
  } // end switch
  
  return phase != end;
}
