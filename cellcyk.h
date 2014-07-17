/*Copyright 2014 Francisco Alvaro

 This file is part of SESHAT.

    SESHAT is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    SESHAT is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with SESHAT.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef _CELLCYK_
#define _CELLCYK_

struct Hypothesis;

#include <cstdio>
#include "hypothesis.h"

using namespace std;

struct CellCYK{
  //Bounding box spatial region coordinates
  int x,y; //top-left
  int s,t; //bottom-right

  //Hypotheses for every non-terminals
  int nnt;
  Hypothesis **noterm;

  //Strokes covered in this cell
  int nc;
  bool *ccc;
  int talla; //total number of strokes

  //Next cell in linked list (CYK table of same size)
  CellCYK *sig;


  //Methods
  CellCYK(int n, int ncc);
  ~CellCYK();

  bool operator<(const CellCYK &C);
  void ccUnion(CellCYK *A, CellCYK *B);
  bool ccEqual(CellCYK *H);
  bool compatible(CellCYK *H);
};


#endif
