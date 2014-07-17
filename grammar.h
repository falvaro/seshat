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
#ifndef _GRAMMAR_
#define _GRAMMAR_

class gParser;

#include <cstdio>
#include <string>
#include <map>
#include <list>
#include "production.h"
#include "gparser.h"
#include "symrec.h"

using namespace std;

struct Grammar{
  map<string,int> noTerminales;
  list<int> initsyms;
  bool *esInit;
  SymRec *sym_rec;

  list<ProductionB *> prodsH, prodsSup, prodsSub;
  list<ProductionB *> prodsV, prodsVe, prodsIns, prodsMrt, prodsSSE;
  list<ProductionT *> prodTerms;

  Grammar(char *conf, SymRec *SR);
  ~Grammar();

  const char *key2str(int k);
  void addInitSym(char *str);
  void addNoTerminal(char *str);
  void addTerminal(float pr, char *S, char *T, char *tex);

  void addRuleH(float pr, char *S, char *A, char *B, char *out, char *merge);
  void addRuleV(float pr, char *S, char *A, char *B, char *out, char *merge);
  void addRuleVe(float pr, char *S, char *A, char *B, char *out, char *merge);
  void addRuleSup(float pr, char *S, char *A, char *B, char *out, char *merge);
  void addRuleSub(float pr, char *S, char *A, char *B, char *out, char *merge);
  void addRuleSSE(float pr, char *S, char *A, char *B, char *out, char *merge);
  void addRuleIns(float pr, char *S, char *A, char *B, char *out, char *merge);
  void addRuleMrt(float pr, char *S, char *A, char *B, char *out, char *merge);
};

#endif
