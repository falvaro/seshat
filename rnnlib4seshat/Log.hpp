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

#ifndef _INCLUDED_Log_h
#define _INCLUDED_Log_h

#include <math.h>
#include <iostream>

using namespace std;

template<class T> class Log
{
  //data
  T expVal;
  T logVal;

public:

  //static data
  static const T expMax;
  static const T expMin;
  static const T expLimit;
  static const T logZero;
  static const T logInfinity;

  //static functions
  static T safe_exp(T x)
  {
    if (x == logZero)
      {
	return 0;
      }
    if (x >= expLimit)
      {
	return expMax;
      }
    return std::exp(x);
  }
  static T safe_log(T x)
  {
    if (x <= expMin)
      {
	return logZero;
      }
    return std::log(x);
  }
  static T log_add(T x, T y)
  {
    if (x == logZero)
      {
	return y;
      }
    if (y == logZero)
      {
	return x;
      }
    if (x < y)
      {
	swap(x, y);
      }
    return x + std::log(1.0 + safe_exp(y - x));
  }
  static T log_subtract(T x, T y)
  {
    if (y == logZero)
      {
	return x;
      }
    if (y >= x)
      {
	return logZero;
      }
    return x + std::log(1.0 - safe_exp(y - x));
  }
  static T log_multiply(T x, T y)
  {
    if (x == logZero || y == logZero)
      {
			return logZero;
      }
    return x + y;
  }
  static T log_divide(T x, T y)
  {
    if (x == logZero)
      {
	return logZero;
      }
    if (y == logZero)
      {
	return logInfinity;
      }
    return x - y;
  }

  //functions
  Log(T v = 0, bool logScale = false):
    expVal(logScale ? -1 : v),
		logVal(logScale ? v : safe_log(v))
  {}
  Log<T>& operator =(const Log<T>& l)
  {
    logVal = l.logVal;
    expVal = l.expVal;
    return *this;
  }
  Log<T>& operator +=(const Log<T>& l)
  {
    logVal = log_add(logVal, l.logVal);
    expVal = -1;
    return *this;
  }
  Log<T>& operator -=(const Log<T>& l)
  {
    logVal = log_subtract(logVal, l.logVal);
    expVal = -1;
    return *this;
  }
	Log<T>& operator *=(const Log<T>& l)
  {
    logVal = log_multiply(logVal, l.logVal);
    expVal = -1;
    return *this;
  }
  Log<T>& operator /=(const Log<T>& l)
  {
    logVal = log_divide(logVal, l.logVal);
    expVal = -1;
    return *this;
  }
  T exp()
  {
    if (expVal < 0)
      {
	expVal = safe_exp(logVal);
      }
    return expVal;
  }
  T log() const
  {
    return logVal;
  }
};

//helper functions
template<class T> Log<T> operator +(Log<T> log1, Log<T> log2)
{
  return Log<T>(Log<T>::log_add(log1.log(), log2.log()), true);
}
template<class T> Log<T> operator -(Log<T> log1, Log<T> log2)
{
  return Log<T>(Log<T>::log_subtract(log1.log(), log2.log()), true);
}
template<class T> Log<T> operator *(Log<T> log1, Log<T> log2)
{
  return Log<T>(Log<T>::log_multiply(log1.log(), log2.log()), true);
}
template<class T> Log<T> operator /(Log<T> log1, Log<T> log2)
{
  return Log<T>(Log<T>::log_divide(log1.log(), log2.log()), true);
}
template<class T> bool operator >(Log<T> log1, Log<T> log2)
{
  return (log1.log() > log2.log());
}
template<class T> bool operator <(Log<T> log1, Log<T> log2)
{
  return (log1.log() < log2.log());
}
template<class T> bool operator ==(Log<T> log1, Log<T> log2)
{
  return (log1.log() == log2.log());
}
template<class T> bool operator <=(Log<T> log1, Log<T> log2)
{
  return (log1.log() <= log2.log());
}
template<class T> bool operator >=(Log<T> log1, Log<T> log2)
{
  return (log1.log() >= log2.log());
}
template<class T> ostream& operator <<(ostream& out, const Log<T>& l)
{
  out << l.log();
  return out;
}
template<class T> istream& operator >>(istream& in, Log<T>& l)
{
  T d;
  in >> d;
  l = Log<T>(d, true);
  return in;
}

template <class T> const T Log<T>::expMax = numeric_limits<T>::max();
template <class T> const T Log<T>::expMin = numeric_limits<T>::min();
template <class T> const T Log<T>::expLimit = std::log(expMax);
template <class T> const T Log<T>::logInfinity = 1e100;
template <class T> const T Log<T>::logZero = -Log<T>::logInfinity;

#endif
