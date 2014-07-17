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
#ifndef _SPAREL_
#define _SPAREL_

class CellCYK;

#include <cstdio>
#include "hypothesis.h"
#include "cellcyk.h"
#include "gmm.h"
#include "sample.h"

class SpaRel{
 public:
  static const int NRELS = 6;
  static const int NFEAT = 9;

 private:
  GMM *model;
  Sample *mue;
  float probs[NRELS];

  double compute_prob(Hypothesis *h1, Hypothesis *h2, int k);
  void smooth(float *post);

 public:
  SpaRel(GMM *gmm, Sample *m);
  ~SpaRel();

  void getFeas(Hypothesis *a, Hypothesis *b, float *sample, int ry);

  double getHorProb(Hypothesis *ha, Hypothesis *hb);
  double getSubProb(Hypothesis *ha, Hypothesis *hb);
  double getSupProb(Hypothesis *ha, Hypothesis *hb);
  double getVerProb(Hypothesis *ha, Hypothesis *hb, bool strict=false);
  double getInsProb(Hypothesis *ha, Hypothesis *hb);
  double getMrtProb(Hypothesis *ha, Hypothesis *hb);
};

#endif
