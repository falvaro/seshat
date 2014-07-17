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
#include "logspace.h"

LogSpace::LogSpace(CellCYK *c, int nr, int dx, int dy) {
  //List length
  N=nr;
  //Size of the "reference symbol"
  RX = dx;
  RY = dy;

  //Create a vector to store the regions
  data = new CellCYK*[N];
  int i=0;
  for(CellCYK *r=c; r; r=r->sig)
    data[i++] = r;

  //Sort the regions
  quicksort(data, 0, N-1);
}

LogSpace::~LogSpace() {
  delete[] data;
}

void LogSpace::getH(CellCYK *c, list<CellCYK*> *set) {
  int sx, sy, ss, st;

  //Set the region to search
  sx = max(c->x+1, c->s-(int)(RX*2));  // (sx,sy)------
  ss = c->s + RX*8;                    //  ------------
  sy = c->y - RY;                      //  ------------
  st = c->t + RY;                      //  ------(ss,st)

  //Retrieve the regions
  bsearchHBP(sx, sy, ss, st, set, c);
}

//Below region
void LogSpace::getV(CellCYK *c, list<CellCYK*> *set) {
  int sx, sy, ss, st;

  //Set the region to search
  sx = c->x - 2*RX;
  ss = c->s + 2*RX;
  sy = max(c->t - RY, c->y+1);
  st = c->t + RY*3;

  //Retrieve the regions
  bsearchStv(sx, sy, ss, st, set, false, c);
}

//Above region. Although only Below is really considered this is necessary to
//solve the problem of the case | aaa|
//                              |bbbb|
//such that "a" would never find "b" because its 'sx' would start before "b.x"
void LogSpace::getU(CellCYK *c, list<CellCYK*> *set) {
  int sx, sy, ss, st;

  //Set the region to search
  sx = c->x - 2*RX;
  ss = c->s + 2*RX;
  sy = c->y - RY*3;
  st = min(c->y + RY, c->t-1);

  //Retrieve the regions
  bsearchStv(sx, sy, ss, st, set, true, c);
}

//Inside region (sqrt)
void LogSpace::getI(CellCYK *c, list<CellCYK*> *set) {
  int sx, sy, ss, st;

  //Set the region to search
  sx = c->x + 1;  // (sx,sy)------
  ss = c->s + RX; //  ------------
  sy = c->y + 1;  //  ------------
  st = c->t + RY; //  ------(ss,st)

  //Retrieve the regions
  bsearch(sx, sy, ss, st, set);
}

//Mroot region (n-th sqrt)
void LogSpace::getM(CellCYK *c, list<CellCYK*> *set) {
  int sx, sy, ss, st;

  //Set the region to search
  sx = c->x - 2*RX;            // (sx,sy)------
  ss = min(c->x + 2*RX, c->s); //  ------------
  sy = c->y - RY;              //  ------------
  st = min(c->y + 2*RY, c->t); //  ------(ss,st)

  //Retrieve the regions
  bsearch(sx, sy, ss, st, set);
}

//SubSupScript regions
void LogSpace::getS(CellCYK *c, list<CellCYK*> *set) {
  int sx, sy, ss, st;

  //Set the region to search
  sx = c->x-1;      // (sx,sy)------
  ss = c->x+1;      //  ------------
  sy = c->y - RY;   //  ------------
  st = c->t + RY;   //  ------(ss,st)

  bsearch(sx, sy, ss, st, set);
}



void LogSpace::bsearch(int sx, int sy, int ss, int st, list<CellCYK*> *set) {
  //Binary search of "sx"
  int i,j;
  for(i=0, j=N; i<j; ) {
    int m=(i+j)/2;

    if( sx <= data[m]->x )
      j=m;
    else
      i=m+1;
  }

  //Retrieve the compatible regions
  while( i<N && data[i]->x <= ss ) {
    if( data[i]->y <= st && data[i]->t >= sy ) {
      set->push_back(data[i]);
    }
    i++;
  }
}

//Version more strict with the upper/lower vertical positions
void LogSpace::bsearchStv(int sx, int sy, int ss, int st, list<CellCYK*> *set, bool U_V, CellCYK *cd) {

  //Binary search of "sx"
  int i,j;
  for(i=0, j=N; i<j; ) {
    int m=(i+j)/2;

    if( sx <= data[m]->x )
      j=m;
    else
      i=m+1;
  }

  //Retrieve the compatible regions
  if( U_V ) { //Direction 'Up' (U)
    while( i<N && data[i]->x <= ss ) {
      if( data[i]->t <= st && data[i]->t >= sy && data[i]->s <= ss ) {
	if( data[i]->t < cd->y )
	  sy = max(max(data[i]->y, data[i]->t-RY),sy);
	set->push_back(data[i]);
      }
      i++;
    }
  }
  else { //Direction 'Down' (V)
    while( i<N && data[i]->x <= ss ) {
      if( data[i]->y <= st && data[i]->y >= sy && data[i]->s <= ss ) {
	if( data[i]->y > cd->t )
	  st = min(min(data[i]->t, data[i]->y+RY),st);
	set->push_back(data[i]);
      }
      i++;
    }
  }
}



//Version that reduces the sx-ss region as soon as hypotheses are found

void LogSpace::bsearchHBP(int sx, int sy, int ss, int st, list<CellCYK*> *set, CellCYK *cd) {
  //Binary search of "sx"
  int i,j;
  for(i=0, j=N; i<j; ) {
    int m=(i+j)/2;

    if( sx <= data[m]->x )
      j=m;
    else
      i=m+1;
  }

  //Retrieve the compatible regions
  while( i<N && data[i]->x <= ss ) {
    if( data[i]->y <= st && data[i]->t >= sy ) {
      if( data[i]->x > cd->s )
	ss = min(min(data[i]->s, data[i]->x+RX),ss);
      set->push_back(data[i]);
    }
    i++;
  }
}



//Sort according to x coordinate of region (x,y)-(s,t)
void LogSpace::quicksort(CellCYK **vec, int ini, int fin) {
  if( ini < fin ) {
    int piv = partition(vec, ini, fin);
    quicksort(vec, ini,   piv);
    quicksort(vec, piv+1, fin);
  }
}

int LogSpace::partition(CellCYK **vec, int ini, int fin) {
  int piv = vec[ini]->x;
  int i=ini-1, j=fin+1;

  do{
    do{
      j--;
    }while(vec[j]->x > piv);
    do{
      i++;
    }while(vec[i]->x < piv);

    if( i<j ) {
      CellCYK *aux = vec[i];
      vec[i] = vec[j];
      vec[j] = aux;
    }
  }while( i<j );

  return j;
}
