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
#ifndef _SAMPLE_
#define _SAMPLE_

class SymRec;
class TableCYK;

#include <cstdio>
#include <list>
#include <climits>
#include "tablecyk.h"
#include "cellcyk.h"
#include "stroke.h"
#include "symrec.h"
#include "grammar.h"

using namespace std;

//Segmentation hypothesis
struct SegmentHyp{
  list<int> stks;  //List of strokes

  //Bounding box (online coordinates)
  int rx, ry; //Top-left
  int rs, rt; //Bottom-right

  int cen;
};


class Sample{
  vector<Stroke*> dataon;
  float **stk_dis;

  int **dataoff;
  int X, Y;
  int IMGxMIN, IMGyMIN, IMGxMAX, IMGyMAX;
  int **pix_stk;

  SymRec *SR;

  //Information to create the output InkML file
  char *outinkml, *outdot;
  string UItag;
  int next_id;

  void loadInkML(char *str);
  void loadSCGInk(char *str);

  void linea(int **img, Punto *pa, Punto *pb, int stkid);
  void linea_pbm(int **img, Punto *pa, Punto *pb, int stkid);
  bool not_visible(int si, int sj, Punto *pi, Punto *pj);

public:
  //Normalized reference symbol size
  int RX, RY;
  float INF_DIST;  //Infinite distance value (visibility)
  float NORMF;     //Normalization factor for distances

  int ox, oy, os, ot; //Online bounding box
  int bx, by, bs, bt; //Offline bounding box

  Sample(char *in);
  ~Sample();

  int dimX();
  int dimY();
  int nStrokes();
  int get(int x, int y);
  Stroke *getStroke(int i);

  void getCentroids(CellCYK *cd, int *ce, int *as, int *ds);
  void getAVGstroke_size(float *avgw, float *avgh);

  void  detRefSymbol();
  void  compute_strokes_distances(int rx, int ry);
  float stroke_distance(int si, int sj);
  float getDist(int si, int sj);
  void  get_close_strokes(int id, list<int> *L, float dist_th);

  float group_penalty(CellCYK *A, CellCYK *B);
  bool  visibility(list<int> *strokes_list);

  void setSymRec( SymRec *sr );

  void setRegion(CellCYK *c, int nComp);
  void setRegion(CellCYK *c, list<int> *LT);
  void setRegion(CellCYK *c, int *v, int size);

  int **render(int *pW, int *pH);
  void renderStrokesPBM(list<int> *SL, int ***img, int *rows, int *cols);

  void render_img(char *out);
  void set_out_inkml(char *out);
  void set_out_dot(char *out);
  char *getOutDot();

  void print();
  void printInkML(Grammar *G, Hypothesis *H);
  void printSymRecInkML(Hypothesis *H, FILE *fout);
};

#endif
