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
#include <algorithm>
#include <bitset>

inline void pbil::educate() {		   
  std::uniform_real_distribution<double> dist(0, 1);
  for (auto& row : samples) {
    unsigned b = 0;
    for (auto& c : row) {
      c = static_cast<int>(dist(rng) < probabilities[b++]);
    }
  }
}

template<typename T>
T get_value(const std::vector<int>& gene) {
  std::bitset<sizeof(T) * CHAR_BIT> b;
  int bidx = 0;
  std::for_each(gene.begin(), gene.begin() + gene.size() - 1, [&](int val) { b[bidx++] = val; });
  const auto val = b.to_ulong();
  T result = 0;
  memcpy(&result, &val, sizeof(T));
  return result;
}


inline void pbil::educate(float min, float max) {
  std::uniform_real_distribution<double> dist(0, 1);

  for (auto& row : samples) {

    unsigned word = 32;

    std::bitset<sizeof(float) * CHAR_BIT> bits;

    for (unsigned start = 0; start < row.size(); start += word) {
      float value = min - 1;

      while (value < min || value > max) {

        for (unsigned i = start, idx = 0; i < start + word; ++i, ++idx) {
          bits[idx] = dist(rng) < probabilities[i];
        }

        const auto val = bits.to_ulong();
        memcpy(&value, &val, sizeof(float));
      }

      //std::cout << "dbg val : " << value << std::endl;
      // got a valid value!
      for (unsigned i = start, idx = 0; i < start + word; ++i, ++idx) {
        row[i] = bits[idx];
      }

    }
  }
}

// warm up pbil (localize search around initial best guess)
inline void pbil::initialize_probabilities() {
  double eps = 1e-2;
  std::uniform_real_distribution<double> dist(0, eps);
  std::vector<int> tmp(initial_guess);

  for (auto& g : tmp) g += dist(rng);


  for (unsigned b = 0; b < probabilities.size(); ++b) {

    double lr = learn_rate + neg_learn_rate;

    probabilities[b] = probabilities[b] * (1 - lr) + tmp[b] * lr;
  }
}


inline void pbil::update_probabilities(const std::vector<int>& min_gene,
                                       const std::vector<int>& max_gene) {
  
  for (unsigned b = 0; b < probabilities.size(); ++b) {

    double lr = (min_gene[b] == max_gene[b] ?
      learn_rate : learn_rate + neg_learn_rate);

    probabilities[b] = probabilities[b] * (1 - lr) + min_gene[b] * lr;
  }
}


inline void pbil::mutate() {

  std::uniform_real_distribution<double> dist(0, 1);

  for (auto& p : probabilities) {
    
    if (dist(rng) < mutate_prob) {
      
      p = p * (1.0 - mutate_shift)
        + (dist(rng) < 0.5 ? 1.0 : 0) * mutate_shift;
      
    }    
  }
}


inline void pbil::init() {
  
  std::for_each(samples.begin(), samples.end(),
		[](std::vector<int>& e) { std::fill(e.begin(), e.end(), 0); });
  
  std::fill(probabilities.begin(), probabilities.end(), 0.5);

}


struct thread_data {
  unsigned start;
  unsigned end;
  std::vector<double> errors;
};

template<class T, typename... Args>
void pbil::optimize(T&& residual, Args&&... args) {
  using namespace std::placeholders;
  

  init();
  
  bool start = true;

  for (size_t i = 0; i < 6; ++i) initialize_probabilities();

  auto rf =
    std::bind(std::forward<T>(residual),
      _1,
      std::ref(std::forward<Args>(args))...);
  /*
  Threadpool workers;
  std::vector<thread_data> T;

  for (size_t j = 0, chunk = (samples.size() + 1) / workers.size(); j < workers.size(); j += chunk) {
    thread_data td;
    td.start = j;
    td.end = std::min(td.start + chunk, workers.size() - td.start);
    td.errors(td.end - td.start);
    T.push_back(td);
  }
  */

  while (best_err >= etol) {
    
    //educate();
    educate(-100, 100);
            
    if (start) {
      start = false;
      if (!initial_guess.empty())
        for (int j = 0; j < initial_guess.size(); ++j) samples[0][j] = initial_guess[j];
    }

    int i = 0;
    std::vector<double> errors(samples.size());
    
    for (auto& e : errors) { e = rf(samples[i++]); }
    
    
    double min_err = 1e10;
    double max_err = -1e10;
    std::vector<int> min_sample, max_sample;
    
    i = 0;
    for (const auto& e : errors) {
      if (min_err > e) {
        min_err = e;
        min_sample = samples[i];
      }
      if (max_err < e) {
        max_err = e;
        max_sample = samples[i];
      }
      ++i;
    }
    
    if (best_err > min_err) {
      best_err = min_err;
      std::vector<int> best = min_sample;
      
      for (auto& b : best) {
        std::cout << b;
      }
      std::cout << std::endl;
    }
    
    update_probabilities(min_sample, max_sample);
    mutate();
    
  }
}
