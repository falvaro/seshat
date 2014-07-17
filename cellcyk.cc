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
#include "cellcyk.h"
#include <cstring>

CellCYK::CellCYK(int n, int ncc) {
  sig = NULL;
  nnt = n;
  nc = ncc;
  talla = 0;

  //Create (empty) hypotheses
  noterm = new Hypothesis*[nnt];
  for(int i=0; i<nnt; i++)
    noterm[i] = NULL;

  //Create (empty) strokes covered
  ccc = new bool[nc];
  for(int i=0; i<nc; i++)
    ccc[i] = false;
}

CellCYK::~CellCYK() {
  //Delete existing hypotheses
  for(int i=0; i<nnt; i++)
    if( noterm[i] )
      delete noterm[i];

  delete[] noterm;
  delete[] ccc;
}

//Comparison operator for logspace ordering
bool CellCYK::operator<(const CellCYK &C) {
  if( x < C.x )
    return true;
  if( x == C.x ) {
    if( y < C.y )
      return true;
    if( y == C.y ) {
      if( s < C.s )
	return true;
      if( s == C.s )
	if( t < C.t )
	  return true;
    }
  }
  return false;
}

//Set the covered strokes to the union of cells A and B
void CellCYK::ccUnion(CellCYK *A, CellCYK *B) {
  for(int i=0; i<nc; i++)
    ccc[i] = ( A->ccc[i] || B->ccc[i] ) ? true : false;
}

//Check if cell H covers the same strokes that this
bool CellCYK::ccEqual(CellCYK *H) {
  if( talla != H->talla )
    return false;
  
  for(int i=0; i<nc; i++)
    if( ccc[i] != H->ccc[i] )
      return false;
  
  return true;
}

//Check if the intersection between the strokes of this cell and H is empty
bool CellCYK::compatible(CellCYK *H) {
  for(int i=0; i<nc; i++)
    if( ccc[i] && H->ccc[i] )
      return false;
  
  return true;
}

