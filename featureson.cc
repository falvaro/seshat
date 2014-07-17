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
#include "featureson.h"

//frame methods
int frame::get_fr_dim() {
  return 7;
}


//sentenceF methods
sentenceF::sentenceF(): n_frames(0), frames(NULL) {};

sentenceF::~sentenceF() {
  delete [] frames;
}

void sentenceF::calculate_features(sentence &S) {
    vector<Point> points;

    for (int s=0; s<S.n_strokes; s++)
      if (S.strokes[s].pen_down) 
	for (int p=0;p<S.strokes[s].n_points;p++) {
	  Point pt=S.strokes[s].points[p];
	  // Set the last point of each stroke --> point_pu=1
	  if (p == (S.strokes[s].n_points-1)) pt.setpu();
	  points.push_back(pt);
	}
    
    
    // Aspect normalization
    vector<PointR> pointsN=normalizaAspect(points);
    points.clear();

    n_frames=pointsN.size();

    frames = new frame[n_frames];

    // Normalizaed "x" and "y" as first features
    for (int i=0; i<n_frames; i++) {
      frames[i].x=pointsN[i].x;
      frames[i].y=pointsN[i].y;
    }

    // Derivatives
    calculate_derivatives(pointsN);
    
    // kurvature
    calculate_kurvature();
    
    pointsN.clear();
}


//sentenceF Private methods

// Signal normalization
vector<PointR> sentenceF::normalizaAspect(vector<Point> & puntos) {
  double ymax=-100000, xmax=-100000, ymin=100000, xmin=100000;
  
  // Calculate x,y max and min
  for (int i=0; i<puntos.size(); i++) {
    if (puntos[i].y<ymin) ymin=puntos[i].y;
    if (puntos[i].y>ymax) ymax=puntos[i].y;
    if (puntos[i].x<xmin) xmin=puntos[i].x;
    if (puntos[i].x>xmax) xmax=puntos[i].x;
  }
  // Prevent the ymin=ymax case (e.g. for "-" and ".")
  if (ymin < (ymax+.5) && ymin > (ymax-.5)) ymax=ymin+1;

  vector<PointR> trazoNorm;
  for (int i = 0; i < puntos.size(); i++) {
    const float TAM=100; 
    
    PointR p(TAM * ((puntos[i].x - xmin)/(ymax - ymin)),TAM * (puntos[i].y - ymin)/(ymax - ymin));
    
    // Set in 'p' the attribute of last point of the stroke
    if (puntos[i].getpu()) p.setpu();
    trazoNorm.push_back(p);
  }
  return trazoNorm;
}


// HTK style derivatives
void sentenceF::calculate_derivatives(vector<PointR> & points, bool norm) {
  unsigned int sigma=0;
  const int tamW=2;
  // Denominator calculation
  for (int i=1; i<=tamW; i++) sigma+=i*i;
  sigma=2*sigma;

  // First derivative
  for (int i=0; i<points.size(); i++) {
    frames[i].dx=0;
    frames[i].dy=0;
    for (int c=1; c<=tamW; c++) {
      double context_ant_x,context_ant_y,context_post_x,context_post_y;
      if (i-c<0) {
	context_ant_x=points[0].x;
	context_ant_y=points[0].y;
      } else {
	context_ant_x=points[i-c].x;
	context_ant_y=points[i-c].y;
      }  
      if (i+c>=points.size()) {
	context_post_x=points[points.size()-1].x;
	context_post_y=points[points.size()-1].y;
      } else {
	context_post_x=points[i+c].x;
	context_post_y=points[i+c].y;
      }
      frames[i].dx+=c*(context_post_x-context_ant_x)/sigma;
      frames[i].dy+=c*(context_post_y-context_ant_y)/sigma;

      // ---------------------------------------------------
      if (norm) {
	double module=sqrt(frames[i].dx*frames[i].dx+frames[i].dy*frames[i].dy);
	if (module>0) {
	  frames[i].dx /= module;
	  frames[i].dy /= module;
	}
      }
      // ---------------------------------------------------
    }
    if (fabs(frames[i].dx)<FLT_MIN) frames[i].dx=0.0;
    if (fabs(frames[i].dy)<FLT_MIN) frames[i].dy=0.0;
  }
  
  // Second derivative
  for (int i=0; i<points.size(); i++) {
    double context_ant_dx,context_ant_dy,context_post_dx,context_post_dy;
    frames[i].ax=0;
    frames[i].ay=0;
    for (int c=1; c<=tamW; c++) {
      if (i-c<0){
	context_ant_dx=frames[0].dx;
	context_ant_dy=frames[0].dy;
      } else {
	context_ant_dx=frames[i-c].dx;
	context_ant_dy=frames[i-c].dy;
      }
      if (i+c>=points.size()) {
	context_post_dx=frames[points.size()-1].dx;
	context_post_dy=frames[points.size()-1].dy;
      } else {
	context_post_dx=frames[i+c].dx;
	context_post_dy=frames[i+c].dy;
      }
      frames[i].ax+=c*(context_post_dx-context_ant_dx)/sigma;
      frames[i].ay+=c*(context_post_dy-context_ant_dy)/sigma;
    }
    if (fabs(frames[i].ax)<FLT_MIN) frames[i].ax=0.0;
    if (fabs(frames[i].ay)<FLT_MIN) frames[i].ay=0.0;
  }
}


void sentenceF::calculate_kurvature() {
  for (int i=0; i<n_frames; i++) {
    double norma=sqrt(frames[i].dx*frames[i].dx+frames[i].dy*frames[i].dy);
    if (norma==0) norma=1;
    frames[i].k=(frames[i].dx*frames[i].ay-frames[i].ax*frames[i].dy)/(norma*norma*norma);
  }
}
