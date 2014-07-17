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
#ifndef ONLINE_H
#define ONLINE_H

#include <math.h>
#include <values.h>
#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

//Real point
class PointR {
  // True if this is the last point of a stroke
  bool point_pu;

 public:
  float x, y;
  
 PointR(float _x, float _y): x(_x), y(_y), point_pu(false) {}
  PointR & operator= (const PointR & p) {
    x=p.x; y=p.y;
      point_pu=p.point_pu;
    return *this;
  }
  bool operator ==(const PointR & p) const {
    return p.x==x && p.y==y;
  }
  bool operator !=(const PointR & p) const {
    return p.x!=x || p.y!=y;
  }
  void setpu() {
    point_pu=1;
  }
  bool getpu() {
    return point_pu;
  }
};



//Integer point
class Point {
  // True if this is the last point of a stroke
  bool point_pu;
 public:
  int x, y;
  
 Point(int _x, int _y): x(_x), y(_y), point_pu(false) {}
  Point & operator= (const Point & p) {
    x=p.x; y=p.y;
    point_pu=p.point_pu;
    return *this;
  }
  bool operator == (const Point & p) const {
    return p.x==x && p.y==y;
  }
  bool operator !=(const Point & p) const {
    return p.x!=x || p.y!=y;
  }
  void setpu() {
    point_pu=1;
  }
  bool getpu() {
    return point_pu;
  }
};


class stroke {
 public:
  int n_points;
  bool pen_down;
  bool is_hat;
  vector<Point> points;
  
  stroke(int n_p=0, bool pen_d=0, bool is_ht=0);
  
  int F_XMIN();
  int F_XMAX();
  int F_XMED();
};


class sentence {
 public:
  int n_strokes;
  vector<stroke> strokes;
  
  sentence(int n_s);
  
  sentence * anula_rep_points();
  sentence * suaviza_traza(int cont_size=2);
};


#endif
 
