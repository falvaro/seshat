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
#ifndef _HYPOTHESIS_
#define _HYPOTHESIS_

class ProductionB;
class ProductionT;

struct CellCYK;
struct Grammar;

#include <cstdio>
#include <cmath>
#include <list>
#include "production.h"
#include "cellcyk.h"
#include "grammar.h"

using namespace std;

struct Hypothesis{
  int clase; //If the hypothesis encodes a terminal symbols this is the class id (-1 otherwise)
  double pr; //log-probability

  //References to left-child (hi) and right-child (hd) to create the derivation tree
  Hypothesis *hi, *hd;

  //The production used to create this hypothesis (either Binary or terminal)
  ProductionB *prod;
  ProductionT *pt;

  //INKML_id for terminal symbols in order to create the InkML output
  string inkml_id;
  //Auxiliar var to retrieve the used production in the special SSE treatment
  ProductionB *prod_sse;

  //Vertical center left and right
  int lcen, rcen;

  CellCYK *parent; //Parent cell
  int ntid;        //Nonterminal ID in parent

  //Methods
  Hypothesis(int c, double p, CellCYK *cd, int nt);
  ~Hypothesis();

  void copy(Hypothesis *SYM);
};

#endif
