/*Copyright 2009,2010 Alex Graves

This file is part of RNNLIB.

RNNLIB is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

RNNLIB is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RNNLIB.  If not, see <http://www.gnu.org/licenses/>.*/

#ifndef _INCLUDED_ActivationFunctions_h
#define _INCLUDED_ActivationFunctions_h

#include <boost/array.hpp>
#include <numeric>
#include <limits>
#include "Log.hpp"

// Implement logistic function
// f(x) = 1 / (1 + exp(-x))
struct Logistic {
  static real_t fn(real_t x) {
    if (x < Log<real_t>::expLimit) {
      if (x > -Log<real_t>::expLimit) {
        return 1.0 / (1.0 + exp(-x));
      }
      return 0;
    }
    return 1;
  }
  static real_t deriv(real_t y) {
    return y*(1.0-y);
  }
  static real_t second_deriv(real_t y) {
    return deriv(y) * (1 - (2 * y));
  }
};
// Implements a soft version of the sign function with
// first and second derivatives.
// f(x) = x / (1 + |x|)
struct Softsign {
  static real_t fn(real_t x) {
    if (x < realMax) {
      if (x > -realMax) {
        return (x/(1 + fabs(x)));
      }
      return -1;
    }
    return 1;
  }
  static real_t deriv(real_t y) {
    return squared(1 - fabs(y));
  }
  static real_t second_deriv(real_t y) {
    return -2 * sign(y) * pow((1 - fabs(y)), 3);
  }
};
// Identity activation function
// f(n)   = x
// f'(n)  = 1
// f''(n) = 0
struct Identity {
  static real_t fn(real_t x) {
    return x;
  }
  static real_t deriv(real_t y) {
    return 1;
  }
  static real_t second_deriv(real_t y) {
    return 0;
  }
};
// Logistic unit in the range [-2, 2]
struct Maxmin2 {
  static real_t fn(real_t x) {
    return (4 * Logistic::fn(x)) - 2;
  }
  static real_t deriv(real_t y) {
    if (y == -2 || y == 2) {
      return 0;
    }
    return (4 - (y * y)) / 4.0;
  }
  static real_t second_deriv(real_t y) {
    return -deriv(y) * 0.5 * y;
  }
};
// Logistic unit in the range [-1, 1]
struct Maxmin1 {
  static real_t fn(real_t x) {
    return (2 * Logistic::fn(x)) - 1;
  }
  static real_t deriv(real_t y) {
    if (y == -1 || y == 1) {
      return 0;
    }
    return (1.0 - (y * y)) / 2.0;
  }
  static real_t second_deriv(real_t y) {
    return -deriv(y) * y;
  }
};
// Logistic unit in the range [0, 2]
struct Max2min0 {
  static real_t fn(real_t x) {
    return (2 * Logistic::fn(x));
  }
  static real_t deriv(real_t y) {
    if (y == -1 || y == 1) {
      return 0;
    }
    return y * (1 - (y / 2.0));
  }
  static real_t second_deriv(real_t y) {
    return deriv(y) * (1 - y);
  }
};
struct Tanh {
  static real_t fn(real_t x) {
    return Maxmin1::fn(2*x);
  }
  static real_t deriv(real_t y) {
    return 1.0 - (y *  y);
  }
  static real_t second_deriv(real_t y) {
    return -2 * deriv(y) * y;
  }
};

#endif
