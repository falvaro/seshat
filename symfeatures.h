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
#ifndef _SYMFEATURES_
#define _SYMFEATURES_

class DataSequence;

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <istream>
#include <fstream>
#include <vector>
#include <string>
#include "online.h"
#include "featureson.h"
#include "sample.h"

class SymFeatures{
  static const int  ON_FEAT = 7;
  static const int OFF_FEAT = 9;
  double means_on[ON_FEAT], means_off[OFF_FEAT];
  double  stds_on[ON_FEAT],  stds_off[OFF_FEAT];
  
public:
  SymFeatures(char *mav_on, char *mav_off);
  ~SymFeatures();

  DataSequence *getOnline(Sample *M, SegmentHyp *SegHyp);
  DataSequence *getOfflineFKI(int **img, int H, int W);
};

#endif
