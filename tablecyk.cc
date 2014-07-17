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
#include "tablecyk.h"
#include <vector>
#include <algorithm>

bool operator<(const coo &A, const coo &B) {
  if( A.x < B.x ) return true;
  if( A.x == B.x ) {
    if( A.y < B.y ) return true;
    if( A.y == B.y ) {
      if( A.s < B.s ) return true;
      if( A.s == B.s )
	if( A.t < B.t ) return true;
    }
  }
  return false;
}

TableCYK::TableCYK(int n, int k) {
  N = n;
  K = k;

  //Target = NULL;
  Target = new Hypothesis(-1, -FLT_MAX, NULL, -1);
  pm_comps = 0.0;

  T = new CellCYK *[N];
  for(int i=0; i<N; i++)
    T[i] = NULL;

  TS = new map<coo,CellCYK*>[N];
}

TableCYK::~TableCYK() {
  for(int i=0; i<N; i++)
    while( T[i] ) {
      CellCYK *aux = T[i]->sig;
      delete T[i];
      T[i] = aux;
    }
  
  delete[] T;
  delete[] TS;
}

Hypothesis *TableCYK::getMLH() {
  return Target;
}

CellCYK *TableCYK::get(int n) {
  return T[n-1];
}

int TableCYK::size(int n) {
  return TS[n-1].size();
}

void TableCYK::updateTarget(coo *K, Hypothesis *H) {
  int pcomps = 0;
  
  for(int i=0; i < H->parent->nc; i++)
    if( H->parent->ccc[i] )
      pcomps++;
  
  if( pcomps > pm_comps || (pcomps==pm_comps && H->pr > Target->pr) ) {
    Target->copy( H );
    pm_comps = pcomps;
  }
}

void TableCYK::add(int n, CellCYK *celda, int noterm_id, bool *esinit) {
  coo key(celda->x, celda->y, celda->s, celda->t);
  map<coo,CellCYK*>::iterator it=TS[n-1].find( key );

  celda->talla = n;

  if( it == TS[n-1].end() ) {
    //Link as head of size  n
    celda->sig = T[n-1];
    T[n-1] = celda;
    TS[n-1][key] = celda;

    if( noterm_id >= 0 ) {
      if( esinit[noterm_id] )
	updateTarget(&key, celda->noterm[noterm_id] );
    }
    else {

      for(int nt=0; nt<celda->nnt; nt++)
	if( celda->noterm[nt] && esinit[nt] )
	  updateTarget(&key, celda->noterm[nt] );

    }
  }
  else { //Maximize probability avoiding duplicates

    int VA, VB;
    if( noterm_id < 0 ) {
      VA = 0;
      VB = celda->nnt;
    }
    else {
      VA = noterm_id;
      VB = VA+1;
    }
    
    CellCYK *r = it->second;

    if( !celda->ccEqual( r ) ) { 
      //The cells cover the same region with a different set of strokes

      float maxpr_c=-FLT_MAX;
      for(int i=VA; i<VB; i++)
	if( celda->noterm[i] && celda->noterm[i]->pr > maxpr_c )
	  maxpr_c = celda->noterm[i]->pr;

      float maxpr_r=-FLT_MAX;
      for(int i=0; i<r->nnt; i++)
	if( r->noterm[i] && r->noterm[i]->pr > maxpr_r )
	  maxpr_r = r->noterm[i]->pr;

      //If the new cell contains the most likely hypothesis, replace the hypotheses
      if( maxpr_c > maxpr_r ) {

	//Copy the new set of strokes
	for(int i=0; i<celda->nc; i++)
	  r->ccc[i] = celda->ccc[i];

	//Replace the hypotheses for each non-terminal
	for(int i=0; i<celda->nnt; i++)
	  if( celda->noterm[i] ) {

	    if( r->noterm[i] ) {
	      r->noterm[i]->copy( celda->noterm[i] );
	      r->noterm[i]->parent = r;
	      
	      if( esinit[i] )
		updateTarget(&key, r->noterm[i] );
	    }
	    else{
	      r->noterm[i] = celda->noterm[i];
	      r->noterm[i]->parent = r;
	      
	      //Set to NULL such that the "delete celda" doesn't delete the hypothesis
	      celda->noterm[i] = NULL;
	      
	      if( esinit[i] )
		updateTarget(&key, r->noterm[i] );
	    }

	  }
	  else if( r->noterm[i] ) {
	    delete r->noterm[i];
	    r->noterm[i] = NULL;
	  }

      }

      delete celda;

      //Finished
      return;
    }


    for(int i=VA; i<VB; i++) {

      if( celda->noterm[i] ) {
	if( r->noterm[i] ) {

	  if( celda->noterm[i]->pr > r->noterm[i]->pr ) {
	    //Maximize probability (replace)
	    r->noterm[i]->copy( celda->noterm[i] );
	    r->noterm[i]->parent = r;

	    if( esinit[i] )
	      updateTarget(&key, r->noterm[i] );
	  }

	}
	else {
	  r->noterm[i] = celda->noterm[i];
	  r->noterm[i]->parent = r;

	  //Set to NULL such that the "delete celda" doesn't delete the hypothesis
	  celda->noterm[i] = NULL;
      
	  if( esinit[i] )
	    updateTarget(&key, r->noterm[i] );
	}
      }

    }

    delete celda;
  }

}
