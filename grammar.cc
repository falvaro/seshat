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
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cmath>
#include <cfloat>
#include <string>
#include <cstring>
#include <map>
#include <vector>
#include <algorithm>
#include "grammar.h"
#include "tablecyk.h"
#include "logspace.h"
#include "sparel.h"

using namespace std;

#define MIN_SPR_PR 0.01

//Aux functions

void error(const char *msg) {
  fprintf(stderr, "Grammar err[%s]\n", msg);
  exit(-1);
}

void error(const char *msg, char *str) {
  char linea[1024];
  sprintf(linea, "Grammar err[%s]\n", msg);
  fprintf(stderr, linea, str);
  exit(-1);
}


//
// Grammar methods
//

Grammar::Grammar(char *path, SymRec *sr) {
  //Load grammar file
  FILE *fd = fopen(path, "r");
  if( !fd ) {
    fprintf(stderr, "Error loading grammar '%s'\n", path);
    exit(-1);
  }

  //Get the path's prefix to determine relative locations
  int i = strlen(path)-1;
  while( i>=0 && path[i] != '/' )
    i--;

  path[i+1] = 0;

  //Save the symbol recognizer to convert between LaTeX and symbol id
  sym_rec = sr;

  gParser GP(this, fd, path);

  fclose(fd);

  esInit = new bool[noTerminales.size()];
  for(int i=0; i<(int)noTerminales.size(); i++)
    esInit[i] = false;

  for(list<int>::iterator it=initsyms.begin(); it!=initsyms.end(); it++)
    esInit[ *it ] = true;
}


void Grammar::addInitSym(char *str) {
  if( noTerminales.find(str) == noTerminales.end() )
    error("addInitSym: Non-terminal '%s' not defined.", str);

  initsyms.push_back( noTerminales[str] );
}

void Grammar::addNoTerminal(char *str) {
  int key = noTerminales.size();
  noTerminales[str] = key;
}

void Grammar::addTerminal(float pr, char *S, char *T, char *tex) {
  if( noTerminales.find(S) == noTerminales.end() )
    error("addTerminal: Non-terminal '%s' not defined.", S);

  bool create=true;
  for(list<ProductionT *>::iterator it=prodTerms.begin(); it!=prodTerms.end(); it++)
    if( (*it)->getNoTerm() == noTerminales[S] ) {

      int id=sym_rec->keyClase(T);
      if( id >= 0 )
	(*it)->setClase(id , pr, tex, 'i');
      else
	fprintf(stderr, "ERROR: %s -> %s (id < 0)\n", S, T);

      create = false;
      break;
    }

  if( create ) {
    ProductionT *pt = new ProductionT(noTerminales[S], sym_rec->getNClases());

    int id=sym_rec->keyClase(T);
    if( id >= 0 )
	pt->setClase(id , pr, tex, 'i');
    else
      fprintf(stderr, "ERROR: %s -> %s (id < 0)\n", S, T);
    
    prodTerms.push_back( pt );
  }

}


void Grammar::addRuleH(float pr, char *S, char *A, char *B, char *out, char *merge) {
  if( noTerminales.find(S) == noTerminales.end() )
    error("Rule: Non-terminal '%s' not defined.", S);
  if( noTerminales.find(A) == noTerminales.end() )
    error("Rule: Non-terminal '%s' not defined.", A);
  if( noTerminales.find(B) == noTerminales.end() )
    error("Rule: Non-terminal '%s' not defined.", B);
  
  ProductionB *pd = new ProductionH(noTerminales[S],
				    noTerminales[A], noTerminales[B], pr, out);

  pd->setMerges( merge[0] );

  prodsH.push_back( pd );
}

void Grammar::addRuleV(float pr, char *S, char *A, char *B, char *out, char *merge) {
  if( noTerminales.find(S) == noTerminales.end() )
    error("Rule: Non-terminal '%s' not defined.", S);
  if( noTerminales.find(A) == noTerminales.end() )
    error("Rule: Non-terminal '%s' not defined.", A);
  if( noTerminales.find(B) == noTerminales.end() )
    error("Rule: Non-terminal '%s' not defined.", B);

  ProductionB *pd = new ProductionV(noTerminales[S],
				    noTerminales[A], noTerminales[B], pr, out);

  pd->setMerges( merge[0] );

  prodsV.push_back( pd );
}

void Grammar::addRuleVe(float pr, char *S, char *A, char *B, char *out, char *merge) {
  if( noTerminales.find(S) == noTerminales.end() )
    error("Rule: Non-terminal '%s' not defined.", S);
  if( noTerminales.find(A) == noTerminales.end() )
    error("Rule: Non-terminal '%s' not defined.", A);
  if( noTerminales.find(B) == noTerminales.end() )
    error("Rule: Non-terminal '%s' not defined.", B);
  
  ProductionB *pd = new ProductionVe(noTerminales[S],
				     noTerminales[A], noTerminales[B], pr, out);
  
  pd->setMerges( merge[0] );

  prodsVe.push_back( pd );
}

void Grammar::addRuleSSE(float pr, char *S, char *A, char *B, char *out, char *merge) {
  if( noTerminales.find(S) == noTerminales.end() )
    error("Rule: Non-terminal '%s' not defined.", S);
  if( noTerminales.find(A) == noTerminales.end() )
    error("Rule: Non-terminal '%s' not defined.", A);
  if( noTerminales.find(B) == noTerminales.end() )
    error("Rule: Non-terminal '%s' not defined.", B);

  ProductionB *pd = new ProductionSSE(noTerminales[S],
				      noTerminales[A], noTerminales[B], pr, out);
  pd->setMerges( merge[0] );

  prodsSSE.push_back( pd );
}

void Grammar::addRuleSup(float pr, char *S, char *A, char *B, char *out, char *merge) {
  if( noTerminales.find(S) == noTerminales.end() )
    error("Rule: Non-terminal '%s' not defined.", S);
  if( noTerminales.find(A) == noTerminales.end() )
    error("Rule: Non-terminal '%s' not defined.", A);
  if( noTerminales.find(B) == noTerminales.end() )
    error("Rule: Non-terminal '%s' not defined.", B);
  
  ProductionB *pd = new ProductionSup(noTerminales[S],
				      noTerminales[A], noTerminales[B], pr, out);
  pd->setMerges( merge[0] );

  prodsSup.push_back( pd );
}

void Grammar::addRuleSub(float pr, char *S, char *A, char *B, char *out, char *merge) {
  if( noTerminales.find(S) == noTerminales.end() )
    error("Rule: Non-terminal '%s' not defined.", S);
  if( noTerminales.find(A) == noTerminales.end() )
    error("Rule: Non-terminal '%s' not defined.", A);
  if( noTerminales.find(B) == noTerminales.end() )
    error("Rule: Non-terminal '%s' not defined.", B);
  
  ProductionB *pd = new ProductionSub(noTerminales[S],
				      noTerminales[A], noTerminales[B], pr, out);

  pd->setMerges( merge[0] );

  prodsSub.push_back( pd );
}

void Grammar::addRuleIns(float pr, char *S, char *A, char *B, char *out, char *merge) {
  if( noTerminales.find(S) == noTerminales.end() )
    error("Rule: Non-terminal '%s' not defined.", S);
  if( noTerminales.find(A) == noTerminales.end() )
    error("Rule: Non-terminal '%s' not defined.", A);
  if( noTerminales.find(B) == noTerminales.end() )
    error("Rule: Non-terminal '%s' not defined.", B);
  
  ProductionB *pd = new ProductionIns(noTerminales[S],
				      noTerminales[A], noTerminales[B], pr, out);

  pd->setMerges( merge[0] );

  prodsIns.push_back( pd );
}

void Grammar::addRuleMrt(float pr, char *S, char *A, char *B, char *out, char *merge) {
  if( noTerminales.find(S) == noTerminales.end() )
    error("Rule: Non-terminal '%s' not defined.", S);
  if( noTerminales.find(A) == noTerminales.end() )
    error("Rule: Non-terminal '%s' not defined.", A);
  if( noTerminales.find(B) == noTerminales.end() )
    error("Rule: Non-terminal '%s' not defined.", B);
  
  ProductionB *pd = new ProductionMrt(noTerminales[S],
				      noTerminales[A], noTerminales[B], pr, out);

  pd->setMerges( merge[0] );

  prodsMrt.push_back( pd );
}

Grammar::~Grammar() {
  for(list<ProductionB *>::iterator it=prodsH.begin(); it!=prodsH.end(); it++)
    delete *it;

  for(list<ProductionB *>::iterator it=prodsSub.begin(); it!=prodsSub.end(); it++)
    delete *it;

  for(list<ProductionB *>::iterator it=prodsSup.begin(); it!=prodsSup.end(); it++)
    delete *it;

  for(list<ProductionB *>::iterator it=prodsV.begin(); it!=prodsV.end(); it++)
    delete *it;

  for(list<ProductionB *>::iterator it=prodsVe.begin(); it!=prodsVe.end(); it++)
    delete *it;

  for(list<ProductionB *>::iterator it=prodsSSE.begin(); it!=prodsSSE.end(); it++)
    delete *it;

  for(list<ProductionB *>::iterator it=prodsIns.begin(); it!=prodsIns.end(); it++)
    delete *it;

  for(list<ProductionB *>::iterator it=prodsMrt.begin(); it!=prodsMrt.end(); it++)
    delete *it;

  for(list<ProductionT *>::iterator it=prodTerms.begin(); it!=prodTerms.end(); it++)
    delete *it;

  delete[] esInit;
}


const char *Grammar::key2str(int k) {
  for(map<string,int>::iterator it=noTerminales.begin(); it!=noTerminales.end(); it++) {
    if( it->second == k )
      return it->first.c_str();
  }
  return "NULL";
}
