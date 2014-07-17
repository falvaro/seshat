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
#include <cmath>
#include <cfloat>
#include "gmm.h"

#define PI 3.14159265359

using namespace std;

GMM::GMM( char *model ) {
  loadModel( model );
}

void GMM::loadModel( char *str ) {
  FILE *fd = fopen(str, "r");
  if( !fd ) {
    fprintf(stderr, "Error loading GMM model file '%s'\n", str);
    exit(-1);
  }

  //Read parameters
  fscanf(fd, "%d %d %d", &C, &D, &G);
  
  //Read prior probabilities
  prior = new float[C];
  for(int i=0; i<C; i++)
    fscanf(fd, "%f", &prior[i]);
    
  invcov  = new float*[C*G];
  mean    = new float*[C*G];
  weight  = new float*[C];
  det     = new float[C*G];

  //Read a GMM for each class
  for(int c=0; c<C; c++) {

    //Read diagonal covariances
    for(int i=0; i<G; i++) {
      invcov[c*G+i] = new float[D];
      det[c*G+i] = 1.0;

      for(int j=0; j<D; j++) {
	fscanf(fd, "%f", &invcov[c*G+i][j]);

	//Compute determinant of convariance matrix (diagonal)
	det[c*G+i] *= invcov[c*G+i][j];

	//Save the inverse of the convariance to save future computations
	if( invcov[c*G+i][j] == 0.0 ) {
	  fprintf(stderr, "Warning: covariance value equal to zero in GMM\n");
	  invcov[c*G+i][j] = 1.0/1.0e-10;
	}
	else
	  invcov[c*G+i][j] = 1.0/invcov[c*G+i][j];
      }
    }

    //Read means
    for(int i=0; i<G; i++) {
      mean[c*G+i] = new float[D];
      for(int j=0; j<D; j++)
	fscanf(fd, "%f", &mean[c*G+i][j]);	
    }

    //Read mixture weights
    weight[c] = new float[G];
    for(int i=0; i<G; i++)
      fscanf(fd, "%f", &weight[c][i]);	 
  }

  fclose(fd);
}


//Probability density function
float GMM::pdf(int c, float *v) {
  float pr = 0.0;

  for(int i=0; i<G; i++) {

    float exponent = 0.0;
    for(int j=0; j<D; j++)
      exponent += (v[j] - mean[c*G+i][j]) * invcov[c*G+i][j] * (v[j] - mean[c*G+i][j]);

    exponent *= -0.5;

    pr +=  weight[c][i] * pow(2 * PI, -D/2.0) * pow(det[c*G+i], -0.5) * exp( exponent );
  }

  return prior[c] * pr;
}

GMM::~GMM() {
  for(int c=0; c<C; c++) {
    for(int i=0; i<G; i++) {
      delete[] invcov[c*G+i];
      delete[] mean[c*G+i];
    }
    delete[] weight[c];
  }

  delete[] det;
  delete[] prior;
  delete[] invcov;
  delete[] mean;
  delete[] weight;
}


void GMM::posterior(float *x, float *pr) {
  float total=0.0;
  for(int c=0; c<C; c++) {
    pr[c]  = pdf(c, x);
    total += pr[c];
  }

  for(int c=0; c<C; c++)
    pr[c] /= total;
}
