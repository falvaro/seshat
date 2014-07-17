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
#ifndef _DURATION_MODEL_
#define _DURATION_MODEL_

#include <cstdio>
#include <cstdlib>
#include "symrec.h"

using namespace std;

class DurationModel{
  int max_strokes;
  int Nsyms;
  float **duration_prob;

  void loadModel(FILE *fd, SymRec *sr);

 public:
  DurationModel(char *str, int mxs, SymRec *sr);
  ~DurationModel();

  float prob(int symclas, int size);
};

#endif
