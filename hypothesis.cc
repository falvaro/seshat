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
#include "hypothesis.h"

Hypothesis::Hypothesis(int c, double p, CellCYK *cd, int nt) {
  clase = c;
  pr = p;
  hi = hd = NULL;
  prod = NULL;
  prod_sse = NULL;
  pt = NULL;
  lcen = rcen = 0;
  parent = cd;
  ntid = nt;
  inkml_id = "none";
}

void Hypothesis::copy(Hypothesis *H) {
  clase = H->clase;
  pr = H->pr;
  hi = H->hi;
  hd = H->hd;
  prod = H->prod;
  prod_sse = H->prod_sse;
  pt = H->pt;
  lcen = H->lcen;
  rcen = H->rcen;
  parent = H->parent;
  ntid = H->ntid;
  inkml_id = H->inkml_id;
}

Hypothesis::~Hypothesis() {
}
