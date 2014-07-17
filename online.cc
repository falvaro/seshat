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

This file is a modification of the online features original software
covered by the following copyright and permission notice:

*/
/*
    Copyright (C) 2006,2007 Moisés Pastor <mpastorg@dsic.upv.es>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "online.h"


// Aux functions

inline int MAX(int a, int b) {
  if (a>=b) return a;
  else return b;		    
}

inline int MIN(int a, int b) {
  if (a<=b) return a;
  else return b;		    
}

//
// "stroke" methods
//

stroke::stroke(int n_p, bool pen_d, bool is_ht): n_points(n_p), pen_down(pen_d), is_hat(is_ht) {}


int stroke::F_XMIN() {
  int xmin=INT_MAX;
  for (int p=0; p<n_points; p++)
    if (xmin>points[p].x) xmin=points[p].x;
  return xmin;
}

int stroke::F_XMAX() {
  int xmax=INT_MIN;
  for (int p=0; p<n_points; p++)
    if (xmax<points[p].x) xmax=points[p].x;
  return xmax;
}

int stroke::F_XMED() {
  int xmed=0;
  for (int p=0; p<n_points; p++) xmed+=points[p].x;
  return xmed/n_points;
}




//
// "sentence" methods
//

sentence::sentence(int n_s): n_strokes(n_s) {}

// Remove repeated points
sentence * sentence::anula_rep_points() {
  sentence * sent_norep=new sentence(n_strokes);
  for (int s=0; s<n_strokes; s++) {
    stroke stroke_norep;
    vector<Point> puntos=strokes[s].points;
    int np=strokes[s].n_points;
    for (int p=0; p<np; p++) {
      if (p<(np-1) && puntos[p]==puntos[p+1]) continue;
      Point point(puntos[p].x,puntos[p].y);
      stroke_norep.points.push_back(point);
    }
    stroke_norep.pen_down=strokes[s].pen_down;
    stroke_norep.n_points=stroke_norep.points.size();
    (*sent_norep).strokes.push_back(stroke_norep);
  }
  return sent_norep;
}

// Smoothing: median filter
sentence * sentence::suaviza_traza(int cont_size) {
  int sum_x,sum_y;
  sentence * sentNorm=new sentence(n_strokes);
  for (int i=0; i<n_strokes; i++) {
    stroke strokeNorm;
    vector<Point> puntos=strokes[i].points;
    int np=strokes[i].n_points;
    for (int p=0; p<np; p++){
      sum_x=sum_y=0;
      for (int c=p-cont_size; c<=p+cont_size; c++)
	if (c<0) {
	  sum_x+=puntos[0].x;
	  sum_y+=puntos[0].y;
	} else if (c>=np) {
	  sum_x+=puntos[np-1].x;
	  sum_y+=puntos[np-1].y;
	} else {
	  sum_x+=puntos[c].x;
	  sum_y+=puntos[c].y;
	}
      Point point(int(sum_x/(cont_size*2+1)),int(sum_y/(cont_size*2+1)));
      strokeNorm.points.push_back(point);
    }
    strokeNorm.pen_down=strokes[i].pen_down;
    strokeNorm.n_points=strokeNorm.points.size();
    (*sentNorm).strokes.push_back(strokeNorm);
  }
  return sentNorm;
}
