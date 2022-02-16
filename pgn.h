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
#ifndef PGN_H
#define PGN_H

#include <fstream>
#include <algorithm>
#include <sstream>
#include <string>
#include <iostream>
#include <cmath>

#include "move.h"
#include "position.h"
#include "types.h"
#include "utils.h"
#include "uci.h"
enum Result { pgn_draw, pgn_wwin, pgn_bwin, pgn_none };

//const std::string START_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

struct game {
  Result result;
  unsigned white_elo{}, black_elo{};
  std::vector<Move> moves;
  
  game() { clear(); }
  bool finished() const { return result != pgn_none; }
  void clear() { moves.clear(); result = pgn_none; white_elo = 0; black_elo = 0; }
  unsigned rating_diff() const { return fabs(white_elo - black_elo); }
};


class pgn {
	std::vector<game> games;
  std::vector<std::string> pgn_files;
  
  bool parse_files();
  bool parse_moves(position& p, const std::string& line, game& g) const;

	static inline bool is_header(const std::string& line);
	static inline bool is_elo(const std::string& line);
	static inline bool is_empty(const std::string& line);
  inline void parse_elo(game& g, const std::string& line) const;
	static inline void strip(std::string& token);
	static inline Square get_square(const std::string& s, int start);

  template<Piece piece>
  static bool find_move(position& p, const Square& to, Move& m, int row = -1, int col = -1);
  
 public:
  pgn() = default;
	pgn(const std::vector<std::string>& files);
  ~pgn() = default;
	pgn(const pgn& o)=delete;
  pgn(const pgn&& o)=delete;
  pgn& operator=(const pgn& o)=delete;
  pgn& operator=(const pgn&& o)=delete;

  
  std::vector<game>& parsed_games() { return games; }
  bool move_from_san(position& p, std::string& s, Move& m) const;

};

inline bool pgn::is_elo(const std::string& line) {
  return (line.find("[WhiteElo") != std::string::npos ||
	  line.find("[BlackElo") != std::string::npos);
}


inline void pgn::parse_elo(game& g, const std::string& line) const
{
  // assume this is a valid elo-tag from a pgn file (!!)
  bool white = line.find("[WhiteElo") != std::string::npos;
  
  std::string segment;
  std::stringstream tmp(line);
  std::getline(tmp, segment, ' ');
  std::getline(tmp, segment, ' ');
  strip(segment);
  
  if (white) { g.white_elo = std::stoi(segment); }
  else { g.black_elo = std::stoi(segment); }
}


inline bool pgn::is_header(const std::string& line) {
  return (line.find('[') != std::string::npos ||
	  line.find(']') != std::string::npos);
}


inline bool pgn::is_empty(const std::string& line) {
  return (line.empty() || line == "\n");
}

inline void pgn::strip(std::string& token) {
  std::string skip = "!?+#[]\"{}";
  std::string result;
  for (const auto& c : token) {
    if (skip.find(c) != std::string::npos) { continue; }
    result += c;
  }
  token = result;
}


template<Piece piece>
bool pgn::find_move(position& p, const Square& to, Move& m, int row, int col) {
  
  Movegen mvs(p);
  mvs.generate<piece>();
  bool promotion = (m.type != no_type);
  
  
  for (int j=0; j<mvs.size(); ++j) {
    
    if (!p.is_legal(mvs[j])) continue;

    if (promotion && m.type == mvs[j].type && mvs[j].t == to) {
      
      if (row >= 0 && row == util::row(mvs[j].f)) { m = mvs[j]; return true; }
      if (col >= 0 && col == util::col(mvs[j].f)) { m = mvs[j]; return true; }
      if (col < 0 && row < 0) { m = mvs[j]; return true; }
    }
    
    else if (!promotion && mvs[j].t == to) {
      if (row >= 0 && row == util::row(mvs[j].f)) { m = mvs[j]; return true; }
      if (col >=0 && col == util::col(mvs[j].f)) { m = mvs[j]; return true; }
      if (col < 0 && row < 0) { m = mvs[j]; return true; }
    }
    
  }
  return false;
}


inline Square pgn::get_square(const std::string& s, int start) {
  
  std::string str = s.substr(start - 1, 2);

  int i = 0;
  for (const auto& sq : SanSquares) {
    if (sq == str) break;
    ++i;
  }
  return static_cast<Square>(i);
}

#endif
