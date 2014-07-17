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
#include "duration.h"


DurationModel::DurationModel(char *str, int mxs, SymRec *sr) {
  FILE *fd = fopen(str,"r");
  if( !fd ) {
    fprintf(stderr, "Error loading duration model '%s'\n", str);
    exit(-1);
  }

  max_strokes = mxs;
  Nsyms = sr->getNClases();

  duration_prob = new float*[Nsyms];
  for(int i=0; i<Nsyms; i++) {
    duration_prob[i] = new float[max_strokes];
    for(int j=0; j<max_strokes; j++)
      duration_prob[i][j] = 0;
  }

  loadModel(fd,sr);

  fclose(fd);
}

void DurationModel::loadModel(FILE *fd, SymRec *sr) {
  char str[64];
  int count, nums;

  //Load data
  while( fscanf(fd, "%d %s %d", &count, str, &nums) == 3 ) {
    if( nums <= max_strokes )
      duration_prob[ sr->keyClase(str) ][nums-1] = count;
  }

  //Compute probabilities
  for(int i=0; i<Nsyms; i++) {
    int total=0;

    for(int j=0; j<max_strokes; j++) {
      if( duration_prob[i][j] == 0 ) //Add-one smoothing
	duration_prob[i][j] = 1;
      total += duration_prob[i][j];
    }
    
    for(int j=0; j<max_strokes; j++)
      duration_prob[i][j] /= total;
  }

}

DurationModel::~DurationModel() {
  for(int i=0; i<Nsyms; i++)
    delete[] duration_prob[i];
  delete[] duration_prob;
}

float DurationModel::prob(int symclas, int size) {
  return duration_prob[symclas][size-1];
}
