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
#ifndef SQUARES_H
#define SQUARES_H

#include "types.h"
#include "utils.h"


namespace {
	const float sq_scores[6][64] =
    {
     {
      // pawns
      0.000,  0.000,  0.000,  0.000,  0.000,  0.000,  0.000,  0.000, 
      0.880,  0.862,  0.327,  0.017,  0.025,  1.000,  0.907,  0.985, 
      0.301,  0.328,  0.291,  0.131,  0.263,  0.251,  0.564,  0.428, 
      0.257,  0.193,  0.377,  0.432,  0.581,  0.276,  0.169,  0.203, 
      0.080,  0.070,  0.093,  0.246,  0.211,  0.068,  0.075,  0.067, 
      0.015,  0.012,  0.014,  0.023,  0.015,  0.016,  0.007,  0.015, 
      0.003,  0.002,  0.002,  0.003,  0.001,  0.000,  0.000,  0.001, 
      0.000,  0.000,  0.000,  0.000,  0.000,  0.000,  0.000,  0.000,
     },
     
     {
      // knights
      0.001,  0.147,  0.017,  0.027,  0.036,  0.042,  0.058,  0.001, 
      0.010,  0.010,  0.048,  0.307,  0.223,  0.034,  0.016,  0.024, 
      0.043,  0.140,  0.800,  0.090,  0.107,  0.966,  0.127,  0.025, 
      0.067,  0.020,  0.112,  0.235,  0.128,  0.077,  0.027,  0.047, 
      0.024,  0.077,  0.057,  0.100,  0.137,  0.053,  0.069,  0.018, 
      0.005,  0.023,  0.031,  0.042,  0.022,  0.017,  0.008,  0.006, 
      0.005,  0.008,  0.012,  0.008,  0.009,  0.007,  0.003,  0.002, 
      0.002,  0.001,  0.002,  0.002,  0.002,  0.001,  0.000,  0.000
     },
     
     {
      // bishops
      0.017,  0.058,  0.891,  0.036,  0.043,  0.484,  0.015,  0.006, 
      0.038,  0.356,  0.179,  0.310,  0.574,  0.106,  0.500,  0.033, 
      0.072,  0.205,  0.143,  0.623,  0.726,  0.207,  0.128,  0.068, 
      0.031,  0.042,  0.246,  0.131,  0.107,  0.276,  0.037,  0.078, 
      0.024,  0.106,  0.056,  0.086,  0.068,  0.039,  0.223,  0.021, 
      0.022,  0.037,  0.037,  0.045,  0.030,  0.024,  0.018,  0.052, 
      0.011,  0.020,  0.019,  0.011,  0.014,  0.013,  0.008,  0.008, 
      0.002,  0.004,  0.006,  0.006,  0.006,  0.004,  0.003,  0.000
     },

     {
      // rooks
      0.000,  0.053,  0.070,  0.591,  0.591,  0.021,  0.046,  0.000, 
      0.017,  0.013,  0.040,  0.064,  0.044,  0.031,  0.008,  0.004, 
      0.018,  0.015,  0.023,  0.034,  0.032,  0.023,  0.015,  0.012, 
      0.013,  0.010,  0.016,  0.026,  0.017,  0.012,  0.005,  0.006, 
      0.014,  0.012,  0.016,  0.019,  0.015,  0.007,  0.005,  0.005, 
      0.017,  0.014,  0.015,  0.021,  0.009,  0.005,  0.003,  0.005, 
      0.027,  0.024,  0.022,  0.018,  0.011,  0.006,  0.004,  0.007, 
      0.008,  0.006,  0.006,  0.006,  0.003,  0.000,  0.000,  0.001
     },

     {
      // queens
      0.012,  0.027,  0.048,  1.000,  0.050,  0.014,  0.002,  0.000, 
      0.013,  0.043,  0.371,  0.405,  0.368,  0.070,  0.019,  0.004, 
      0.026,  0.154,  0.084,  0.174,  0.118,  0.152,  0.069,  0.030, 
      0.057,  0.026,  0.053,  0.078,  0.061,  0.053,  0.055,  0.043, 
      0.016,  0.021,  0.022,  0.034,  0.029,  0.022,  0.025,  0.044, 
      0.014,  0.012,  0.017,  0.020,  0.017,  0.016,  0.012,  0.026, 
      0.014,  0.018,  0.014,  0.011,  0.012,  0.010,  0.005,  0.010, 
      0.007,  0.006,  0.007,  0.008,  0.007,  0.003,  0.002,  0.005
     },

     {
      // kings
      0.003,  0.030,  0.937,  -0.54,  -0.61,  0.019,  1.00,  0.055, 
      0.003,  0.006,  0.008,  0.013,  0.020,  0.027,  0.053,  0.034, 
      0.001,  0.004,  0.006,  0.011,  0.016,  0.016,  0.013,  0.006, 
      0.001,  0.002,  0.004,  0.006,  0.007,  0.007,  0.005,  0.002, 
      0.001,  0.002,  0.002,  0.003,  0.003,  0.003,  0.003,  0.001, 
      0.001,  0.001,  0.001,  0.001,  0.002,  0.002,  0.001,  0.001, 
      0.000,  0.001,  0.001,  0.001,  0.001,  0.001,  0.001,  0.000, 
      0.000,  0.000,  0.000,  0.000,  0.000,  0.000,  0.000,  0.000
     }
    };


  
  template<Color c>
  float square_score(const Piece& p, const Square& s);

  
  template<> float square_score<black>(const Piece& p, const Square& s) {
    return 5 * sq_scores[p][56 - 8 * util::row(s) + util::col(s)];
  }
  
  
  template<> float square_score<white>(const Piece& p, const Square& s) {   
    return 5 * sq_scores[p][s];
  }  
}

#endif
