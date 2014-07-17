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
#ifndef _LOGSPACE_
#define _LOGSPACE_

#include <cstdio>
#include <list>
#include "cellcyk.h"

class LogSpace{
  int N;
  int RX, RY;
  CellCYK **data;

  void quicksort(CellCYK **vec, int ini, int fin);
  int  partition(CellCYK **vec, int ini, int fin);
  void bsearch(int sx, int sy, int ss, int st, list<CellCYK*> *set);
  void bsearchStv(int sx, int sy, int ss, int st, list<CellCYK*> *set, bool U_V, CellCYK *cd);
  void bsearchHBP(int sx, int sy, int ss, int st, list<CellCYK*> *set, CellCYK *cd);

 public:
  LogSpace(CellCYK *c, int nr, int dx, int dy);
  ~LogSpace();

  void getH(CellCYK *c, list<CellCYK*> *set);
  void getV(CellCYK *c, list<CellCYK*> *set);
  void getU(CellCYK *c, list<CellCYK*> *set);
  void getI(CellCYK *c, list<CellCYK*> *set);
  void getM(CellCYK *c, list<CellCYK*> *set);
  void getS(CellCYK *c, list<CellCYK*> *set);
};

#endif
