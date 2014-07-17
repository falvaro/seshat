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

#ifndef _INCLUDED_Matrix_h
#define _INCLUDED_Matrix_h

#define OP_TRACKING

#ifdef OP_TRACKING
static uint64_t matrixOps = 0;
#endif

// M += a * b
static void outer(
    const real_t *aBegin, const real_t *aEnd, real_t *M, const real_t *b,
    const real_t *bEnd) {
#ifdef OP_TRACKING
  const real_t* mStart = M;
#endif
  for (; b != bEnd; ++b) {
    real_t input = *b;
    for (const real_t *a = aBegin; a != aEnd; ++a, ++M) {
      *M += *a * input;
    }
  }
#ifdef OP_TRACKING
  matrixOps += M - mStart;
#endif
}

// out += M in
static void dot(
    const real_t *inBegin, const real_t *inEnd, const real_t *M, real_t *out,
    real_t *outEnd) {
#ifdef OP_TRACKING
  const real_t* mStart = M;
#endif
  for (; out != outEnd; ++out) {
    real_t sum = 0;
    for (const real_t *in = inBegin; in != inEnd; ++in, ++M) {
      sum += *M * (*in);
    }
    *out += sum;
  }
#ifdef OP_TRACKING
  matrixOps += M - mStart;
#endif
}

// out += transpose(M) in
static void dot_transpose(
    const real_t *in, const real_t *inEnd, const real_t *M, real_t *outBegin,
    real_t *outEnd) {
#ifdef OP_TRACKING
  const real_t* mStart = M;
#endif
  for (; in != inEnd; ++in) {
    real_t input = *in;
    for (real_t *out = outBegin; out != outEnd; ++out, ++M) {
      *out += *M * input;
    }
  }
#ifdef OP_TRACKING
  matrixOps += M - mStart;
#endif
}

// out += transpose(M^2) in
static void dot_transpose_m_squared(
    const real_t *in, const real_t *inEnd, const real_t *M, real_t *outBegin,
    real_t *outEnd) {
#ifdef OP_TRACKING
  const real_t* mStart = M;
#endif
  for (; in != inEnd; ++in) {
    real_t input = *in;
    for (real_t *out = outBegin; out != outEnd; ++out, ++M) {
      *out += squared(*M) * input;
    }
  }
#ifdef OP_TRACKING
  matrixOps += M - mStart;
#endif
}

// M += a^2 * b
static void outer_a_squared(
    const real_t *aBegin, const real_t *aEnd, real_t *M, const real_t *b,
    const real_t *bEnd) {
#ifdef OP_TRACKING
  const real_t* mStart = M;
#endif
  for (; b != bEnd; ++b) {
    real_t input = *b;
    for (const real_t *a = aBegin; a != aEnd; ++a, ++M) {
      *M += squared(*a) * input;
    }
  }
#ifdef OP_TRACKING
  matrixOps += M - mStart;
#endif
}

template<class R> static void outer(const R& a, real_t *M, const R&b) {
  outer(boost::begin(a), boost::end(a), M, boost::begin(b), boost::end(b));
}

template<class R> static void dot(const R& a, const real_t *M, const R& b) {
  dot(boost::begin(a), boost::end(a), M, boost::begin(b), boost::end(b));
}

template<class R> static void dot_transpose(
    const R& a, const real_t *M, const R& b) {
  dot_transpose(
      boost::begin(a), boost::end(a), M, boost::begin(b), boost::end(b));
}

template<class R> static void outer_a_squared(
    const R& a, real_t *M, const R&b) {
  outer_a_squared(
      boost::begin(a), boost::end(a), M, boost::begin(b), boost::end(b));
}

template<class R> static void dot_transpose_m_squared(
    const R& a, const real_t *M, const R& b) {
  dot_transpose_m_squared(
      boost::begin(a), boost::end(a), M, boost::begin(b), boost::end(b));
}

static real_t& elt(View<real_t> M, int x, int y, int width) {
  return M[(y*width) + x];
}
#endif
