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
#ifndef _PRODUCTION_
#define _PRODUCTION_

class CellCYK;

#include <cstdio>
#include <string>
#include <list>
#include "hypothesis.h"
#include "cellcyk.h"
#include "symrec.h"
#include "grammar.h"

//Binary productions of the grammar (2D-PCFG)
class ProductionB{
 protected:
  char *outStr;
  char merge_cen;

 public:
  int S;
  int A, B;
  float prior;

  ProductionB(int s, int a, int b);
  ProductionB(int s, int a, int b, float pr, char *out);
  ~ProductionB();

  float solape(Hypothesis *a, Hypothesis *b);
  void printOut(Grammar *G, Hypothesis *H);
  void setMerges(char c);
  void mergeRegions(Hypothesis *a, Hypothesis *b, Hypothesis *s);
  bool check_out();
  char *get_outstr();

  //Pure virtual functions
  virtual char tipo() = 0;
  virtual void print() = 0;
  virtual void print_mathml(Grammar *G, Hypothesis *H, FILE *fout, int *nid) = 0;
};


//Production S -> A : B
class ProductionH : public ProductionB{

 public:
  ProductionH(int s, int a, int b);
  ProductionH(int s, int a, int b, float pr, char *out);
  
  void print();
  char tipo();
  void mergeRegions(Hypothesis *a, Hypothesis *b, Hypothesis *s);
  void print_mathml(Grammar *G, Hypothesis *H, FILE *fout, int *nid);
};


//Production: S -> A / B
class ProductionV : public ProductionB{
  
 public:
  ProductionV(int s, int a, int b);
  ProductionV(int s, int a, int b, float pr, char *out);
  
  void print();
  char tipo();
  void mergeRegions(Hypothesis *a, Hypothesis *b, Hypothesis *s);
  void print_mathml(Grammar *G, Hypothesis *H, FILE *fout, int *nid);
};


//Production: S -> A /u B
class ProductionU : public ProductionB{
  
 public:
  ProductionU(int s, int a, int b);
  ProductionU(int s, int a, int b, float pr, char *out);
  
  void print();
  char tipo();
  void mergeRegions(Hypothesis *a, Hypothesis *b, Hypothesis *s);
  void print_mathml(Grammar *G, Hypothesis *H, FILE *fout, int *nid);
};


//Production: S -> A /e B
class ProductionVe : public ProductionB{
  
 public:
  ProductionVe(int s, int a, int b);
  ProductionVe(int s, int a, int b, float pr, char *out);
  
  void print();
  char tipo();
  void mergeRegions(Hypothesis *a, Hypothesis *b, Hypothesis *s);
  void print_mathml(Grammar *G, Hypothesis *H, FILE *fout, int *nid);
};



//Production: S -> A sse B
class ProductionSSE : public ProductionB{
  
 public:
  ProductionSSE(int s, int a, int b);
  ProductionSSE(int s, int a, int b, float pr, char *out);
  
  void print();
  char tipo();
  void mergeRegions(Hypothesis *a, Hypothesis *b, Hypothesis *s);
  void print_mathml(Grammar *G, Hypothesis *H, FILE *fout, int *nid);
};



//Production: S -> A ^ B
class ProductionSup : public ProductionB{
  
 public:
  ProductionSup(int s, int a, int b);
  ProductionSup(int s, int a, int b, float pr, char *out);
  
  void print();
  char tipo();
  void mergeRegions(Hypothesis *a, Hypothesis *b, Hypothesis *s);
  void print_mathml(Grammar *G, Hypothesis *H, FILE *fout, int *nid);
};


//Production: S -> A _ B
class ProductionSub : public ProductionB{
  
 public:
  ProductionSub(int s, int a, int b);
  ProductionSub(int s, int a, int b, float pr, char *out);
  
  void print();
  char tipo();
  void mergeRegions(Hypothesis *a, Hypothesis *b, Hypothesis *s);
  void print_mathml(Grammar *G, Hypothesis *H, FILE *fout, int *nid);
};


//Production: S -> A ins B
class ProductionIns : public ProductionB{
  
 public:
  ProductionIns(int s, int a, int b);
  ProductionIns(int s, int a, int b, float pr, char *out);
  
  void print();
  char tipo();
  void mergeRegions(Hypothesis *a, Hypothesis *b, Hypothesis *s);
  void print_mathml(Grammar *G, Hypothesis *H, FILE *fout, int *nid);
};


//Production: S -> A mroot B
class ProductionMrt : public ProductionB{
  
 public:
  ProductionMrt(int s, int a, int b);
  ProductionMrt(int s, int a, int b, float pr, char *out);
  
  void print();
  char tipo();
  void mergeRegions(Hypothesis *a, Hypothesis *b, Hypothesis *s);
  void print_mathml(Grammar *G, Hypothesis *H, FILE *fout, int *nid);
};


//Production S -> term ( N clases )
class ProductionT{
  int S;
  bool *clases;
  char **texStr;
  char *mltype;
  float *probs;
  int N;

 public:
  ProductionT(int s, int nclases);
  ~ProductionT();
  
  void setClase(int k, float pr, char *tex, char mlt);
  bool getClase(int k);
  float getPrior(int k);
  char *getTeX(int k);
  char getMLtype(int k);
  int  getNoTerm();
  void print();
};

#endif
