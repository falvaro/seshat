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
#include "segmentation.h"
#include <cmath>

SegmentationModelGMM::SegmentationModelGMM(char *mod) {
  FILE *fd = fopen(mod,"r");
  if( !fd ) {
    fprintf(stderr, "Error loading segmentation model '%s'\n", mod);
    exit(-1);
  }
  fclose(fd);

  model = new GMM(mod);
}

SegmentationModelGMM::~SegmentationModelGMM() {
  delete model;
}

float SegmentationModelGMM::prob(CellCYK *cd, Sample *m) {
  int Nstrokes=0, nps=0;
  float dist=0, delta=0, sigma=0, mind=0, avgsize=0;

  for(int i=0; i<cd->nc; i++)
    if( cd->ccc[i] )
      Nstrokes++;

  int *strokes_list = new int[Nstrokes];
  Nstrokes = 0;
  for(int i=0; i<cd->nc; i++)
    if( cd->ccc[i] )
      strokes_list[Nstrokes++] = i;

  //For every stroke
  for(int i=0; i<Nstrokes; i++) {
    Stroke *Si = m->getStroke( strokes_list[i] );

    float size_i = max(Si->rs - Si->rx, Si->rt - Si->ry);
    avgsize += size_i;

    for(int j=i+1; j<Nstrokes; j++) {
      Stroke *Sj = m->getStroke( strokes_list[j] );

      //distance between stroke Si and Sj
      mind  += Si->min_dist( Sj );

      dist  += abs( (Si->rs + Si->rx)/2.0 - (Sj->rs + Sj->rx)/2.0 );
      sigma += abs( (Si->rt + Si->ry)/2.0 - (Sj->rt + Sj->ry)/2.0 );
      
      float size_j = max( Sj->rt - Sj->ry, Sj->rs - Sj->rx);
      delta += abs( size_i - size_j );

      nps++;
    }

  }

  float avgw, avgh, nf;
  m->getAVGstroke_size(&avgw, &avgh);
  nf = sqrt(avgw*avgw + avgh*avgh);

  mind  /= nps*nf;
  dist  /= nps*nf;
  delta /= nps*nf;
  sigma /= nps*nf;

  float sample[4];
  float probs[2];

  sample[0] = mind;
  sample[1] = dist;
  sample[2] = delta;
  sample[3] = sigma;

  model->posterior(sample, probs);

  delete[] strokes_list;

  //Return probability of being a proper segmentation hypothesis
  return probs[1];
}
