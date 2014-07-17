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
#include "symfeatures.h"

SymFeatures::SymFeatures(char *mav_on, char *mav_off) {

  //Load means and stds normalization
  FILE *fd = fopen(mav_on, "r");
  if( !fd ) {
    fprintf(stderr, "Error loading online mav file: %s\n", mav_on);
    exit(-1);
  }
  
  //Read values online
  for(int i=0; i<ON_FEAT; i++)
    fscanf(fd, "%lf", &means_on[i]);
  for(int i=0; i<ON_FEAT; i++)
    fscanf(fd, "%lf", &stds_on[i]);

  fclose(fd);

  fd = fopen(mav_off, "r");
  if( !fd ) {
    fprintf(stderr, "Error loading offline mav file: %s\n", mav_off);
    exit(-1);
  }
  
  //Read values offline
  for(int i=0; i<OFF_FEAT; i++)
    fscanf(fd, "%lf", &means_off[i]);
  for(int i=0; i<OFF_FEAT; i++)
    fscanf(fd, "%lf", &stds_off[i]);

  fclose(fd);
}

SymFeatures::~SymFeatures() {
}


DataSequence *SymFeatures::getOnline(Sample *M, SegmentHyp *SegHyp) {

  //Create and fill sequence of points
  sentence *sent=new sentence( SegHyp->stks.size() );

  for(list<int>::iterator it=SegHyp->stks.begin(); it!=SegHyp->stks.end(); it++) {
    
    stroke st(M->getStroke(*it)->getNpuntos(), 1); //means is pendown stroke

    for(int j=0; j<M->getStroke(*it)->getNpuntos(); j++) {
      Punto *p = M->getStroke(*it)->get(j);
      Point q(p->x, p->y);

      st.points.push_back( q );
    }

    sent->strokes.push_back(st);
  }

  // Remove repeated points
  sentence *no_rep = sent->anula_rep_points();
  // Median filter
  sentence * traz_suav=no_rep->suaviza_traza();

  //Compute online features
  sentenceF feat;
  feat.calculate_features(*traz_suav);

  //Create DataSequence
  
  //Set sequence shape
  int nvec = feat.n_frames;

  //Check number of online features
  if( feat.frames[0].get_fr_dim() != ON_FEAT ) {
    fprintf(stderr, "Error: unexpected number of online features\n");
    exit(-1);
  }

  //Create sequence
  DataSequence *seq = new DataSequence(ON_FEAT);

  vector<size_t> shape(1);
  shape[0] = nvec;

  //Create aux SeqBuffer to fill data
  SeqBuffer<real_t> *auxBuf = new SeqBuffer<real_t>(shape, ON_FEAT);

  //Save the input vectors following the SeqBuffer data representation
  for(int i=0; i<nvec; i++) {
    for(int j=0; j<ON_FEAT; j++) {
      double val = feat.frames[i].getFea(j);

      //Normalize to normal(0,1)
      val = (val - means_on[j])/stds_on[j];

      auxBuf->data[i*ON_FEAT + j] = val; 
    }
  }

  //Assign the loaded data
  seq->inputs = *auxBuf;
  delete auxBuf;

  //Create target vector (content doesn't matter, just because it's required)
  vector<int> target(nvec);
  shape[0] = nvec;
  seq->targetClasses.data = target;
  seq->targetClasses.shape = shape;
  seq->tag = "none";
  
  //Free memory
  delete sent;
  delete no_rep;
  delete traz_suav;

  //Return extracted features for the sequence of strokes
  return seq;
}


DataSequence *SymFeatures::getOfflineFKI(int **img, int H, int W) {

  //Create sequence
  DataSequence *seq = new DataSequence(OFF_FEAT);

  //Set sequence shape
  int nvec = W;
  vector<size_t> shape(1);
  shape[0] = nvec;

  //Create aux SeqBuffer to fill data
  SeqBuffer<real_t> *auxBuf = new SeqBuffer<real_t>(shape, OFF_FEAT);


  //Compute FKI offline features
  double c[OFF_FEAT+1];
  double c4ant=H+1, c5ant=0;

  //For every column
  for(int x=0; x<W; x++) {

    //Compute the FKI 9 features
    for(int i=0; i<OFF_FEAT; i++)
      c[i] = 0;
    c[4]=H+1;

    for(int y=1; y<=H; y++) {
      if( img[y-1][x] ) { //Black pixel
	c[1] += 1;
	c[2] += y;
	c[3] += y*y;
	if( y<c[4] ) c[4]=y;
	if( y>c[5] ) c[5]=y;
      }
      if( y>1 && img[y-1][x] != img[y-2][x] ) c[8]++; 
    }
    
    c[2] /= H;
    c[3] /= H*H;

    for(int y=c[4]+1; y<c[5]; y++)
      if( img[y-1][x] ) //Black pixel
	c[9]++;
    
    c[6]=H+1; c[7]=0;
    if( x+1 < W ) {
      for(int y=1; y<=H; y++) {
	if( img[y-1][x+1] ) { //Black pixel
	  if( y<c[6] ) c[6]=y;
	  if( y>c[7] ) c[7]=y;
	}
      }
    }
    c[6] = (c[6] - c4ant)/2;
    c[7] = (c[7] - c5ant)/2;

    c4ant = c[4];
    c5ant = c[5];

    //Save the input vectors following the SeqBuffer data representation
    for(int j=0; j<OFF_FEAT; j++) {
      //Normalize to normal(0,1)
      c[j+1] = (c[j+1] - means_off[j])/stds_off[j];

      auxBuf->data[x*OFF_FEAT + j] = c[j+1];
    }
  }

  //Assign the loaded data
  seq->inputs = *auxBuf;
  delete auxBuf;

  //Create target vector (content doesn't matter, just because it's required)
  vector<int> target(nvec);
  shape[0] = nvec;
  seq->targetClasses.data = target;
  seq->targetClasses.shape = shape;
  seq->tag = "none";
  
  //Return extracted features for the sequence of strokes
  return seq;
}
