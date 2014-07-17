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
#include "meparser.h"

//Symbol classifier N-Best
#define NB 10

meParser::meParser(char *conf) {
  FILE *fconfig=fopen(conf, "r");
  if( !fconfig ) {
    fprintf(stderr, "Error: loading config file '%s'\n", conf);
    exit(-1);
  }

  //Read configuration file
  char auxstr[1024],path[1024];

  clusterF = -1;
  segmentsTH = -1;
  max_strokes = -1;
  ptfactor = -1;
  pbfactor = -1;
  qfactor = -1;
  dfactor = -1;
  gfactor = -1;
  rfactor = -1;
  path[0] = 0;
  gmm_spr = NULL;

  fscanf(fconfig, "%s", auxstr);
  while( !feof(fconfig) ) {
    if( !strcmp(auxstr,"GRAMMAR") )
      fscanf(fconfig, "%s", path); //Grammar path
    else if( !strcmp(auxstr,"MaxStrokes") ) {
      fscanf(fconfig, "%s", auxstr); //Info
      max_strokes = atoi( auxstr );
    }
    else if( !strcmp(auxstr,"SpatialRels") ) {
      fscanf(fconfig, "%s", auxstr);
      gmm_spr = new GMM(auxstr);
    }
    else if( !strcmp(auxstr,"InsPenalty") ) {
      fscanf(fconfig, "%s", auxstr);
      InsPen = atof( auxstr );
    }
    else if( !strcmp(auxstr,"ClusterF") ) {
      fscanf(fconfig, "%s", auxstr);
      clusterF = atof( auxstr );
    }
    else if( !strcmp(auxstr,"SegmentsTH") ) {
      fscanf(fconfig, "%s", auxstr);
      segmentsTH = atof( auxstr );
    }
    else if( !strcmp(auxstr,"ProductionTSF") ) {
      fscanf(fconfig, "%s", auxstr);
      ptfactor = atof( auxstr );
    }
    else if( !strcmp(auxstr,"ProductionBSF") ) {
      fscanf(fconfig, "%s", auxstr);
      pbfactor = atof( auxstr );
    }
    else if( !strcmp(auxstr,"RelationSF") ) {
      fscanf(fconfig, "%s", auxstr);
      rfactor = atof( auxstr );
    }
    else if( !strcmp(auxstr,"SymbolSF") ) {
      fscanf(fconfig, "%s", auxstr);
      qfactor = atof( auxstr );
    }
    else if( !strcmp(auxstr,"DurationSF") ) {
      fscanf(fconfig, "%s", auxstr);
      dfactor = atof( auxstr );
    }
    else if( !strcmp(auxstr,"SegmentationSF") ) {
      fscanf(fconfig, "%s", auxstr);
      gfactor = atof( auxstr );
    }
    else
      fscanf(fconfig, "%s", auxstr); //Info
    
    fscanf(fconfig, "%s", auxstr); //Next field id
  }

  if( path[0]==0 ) {
    fprintf(stderr, "Error: GRAMMAR field not found in config file '%s'\n", conf);
    exit(-1);
  }

  if( !gmm_spr ) {
    fprintf(stderr, "Error: Loading GMM model in config file '%s'\n", conf);
    exit(-1);
  }

  if( max_strokes <= 0 || max_strokes > 10 ) {
    fprintf(stderr, "Error: Wrong MaxStrokes value in config file '%s'\n", conf);
    exit(-1);
  }

  if( clusterF < 0 ) {
    fprintf(stderr, "Error: Wrong ClusterF value in config file '%s'\n", conf);
    exit(-1);
  }

  if( segmentsTH <= 0 ) {
    fprintf(stderr, "Error: Wrong SegmentsTH value in config file '%s'\n", conf);
    exit(-1);
  }

  if( InsPen <= 0 ) {
    fprintf(stderr, "Error: Wrong InsPenalty value in config file '%s'\n", conf);
    exit(-1);
  }

  if( qfactor <= 0 )
    fprintf(stderr, "WARNING: SymbolSF = %f\n", qfactor);
  if( ptfactor <= 0 )
    fprintf(stderr, "WARNING: ProductionTSF = %f\n", ptfactor);
  if( pbfactor <= 0 )
    fprintf(stderr, "WARNING: ProductionBSF = %f\n", pbfactor);
  if( rfactor <= 0 )
    fprintf(stderr, "WARNING: RelationSF = %f\n", rfactor);
  if( dfactor < 0 )
    fprintf(stderr, "WARNING: DurationSF = %f\n", dfactor);
  if( gfactor < 0 )
    fprintf(stderr, "WARNING: SegmentationSF = %f\n", gfactor);

  //Read grammar path
  fscanf(fconfig, "%s", path);

  //Remove the last \n character
  if( path[strlen(path)-1] == '\n' )
    path[strlen(path)-1] = '\0';

  fclose(fconfig);

  //Load symbol recognizer
  loadSymRec(conf);

  //Load grammar
  G = new Grammar(path, sym_rec);
}

meParser::~meParser() {
  delete G;
  delete sym_rec;
  delete duration;
  delete segmentation;
}


void meParser::loadSymRec(char *config) {
  FILE *fd=fopen(config, "r");
  if( !fd ) {
    fprintf(stderr, "Error: loading config file '%s'\n", config);
    exit(-1);
  }
  
  //Read symbol recognition information from config file
  char auxstr[1024], dur_path[1024], seg_path[1024];
  dur_path[0] = seg_path[0] = 0;
  
  fscanf(fd, "%s", auxstr);
  while( !feof(fd) ) {
    if( !strcmp(auxstr,"Duration") )
      fscanf(fd, "%s", dur_path);
    else if( !strcmp(auxstr,"Segmentation") )
      fscanf(fd, "%s", seg_path);
    else
      fscanf(fd, "%s", auxstr); //Info
    
    fscanf(fd, "%s", auxstr); //Next field id
  }

  if( dur_path[0]==0 ) {
    fprintf(stderr, "Error: Duration field not found in config file '%s'\n", config);
    exit(-1);
  }
  if( seg_path[0]==0 ) {
    fprintf(stderr, "Error: Segmentation field not found in config file '%s'\n", config);
    exit(-1);
  }
  
  //Close configure
  fclose(fd);
  
  //Load symbol recognizer
  sym_rec = new SymRec(config);

  //Load duration and segmentation model
  duration     = new DurationModel(dur_path,max_strokes,sym_rec);
  segmentation = new SegmentationModelGMM(seg_path);
}


//CYK table initialization with the terminal symbols
void meParser::initCYKterms(Sample *M, TableCYK *tcyk, int N, int K) {

  for(int i=0; i<M->nStrokes(); i++) {
      
    int cmy, asc, des;
    
    printf("Stroke %d:\n", i);
      
    int clase[NB];
    float pr[NB];
    
    cmy = sym_rec->clasificar( M, i, NB, clase, pr, &asc, &des );
      
    CellCYK *cd = new CellCYK(G->noTerminales.size(), N);
      
    M->setRegion(cd, i);
    
    bool insertar=false;
    for(list<ProductionT *>::iterator it=G->prodTerms.begin(); it!=G->prodTerms.end(); it++) {
      ProductionT *prod = *it;
      
      for(int k=0; k<NB; k++)
	if( pr[k] > 0.0 && prod->getClase( clase[k] ) && prod->getPrior(clase[k]) > -FLT_MAX ) {
	  
	  float prob = log(InsPen) 
	    + ptfactor * prod->getPrior(clase[k])
	    + qfactor  * log(pr[k])
	    + dfactor  * log(duration->prob(clase[k],1));
	  
	  if( cd->noterm[prod->getNoTerm()] ) {
	    if( cd->noterm[prod->getNoTerm()]->pr > prob + prod->getPrior(clase[k]))
	      continue;
	    else
	      delete cd->noterm[prod->getNoTerm()];
	  }
	  
	  insertar=true;
	  
	  //Create new symbol
	  cd->noterm[prod->getNoTerm()] = new Hypothesis(clase[k], prob, cd, prod->getNoTerm() );
	  cd->noterm[prod->getNoTerm()]->pt = prod;
	  
	  //Compute the vertical centroid according to the type of symbol
	  int cen, type = sym_rec->symType(clase[k]);
	  if( type==0 )       cen = cmy; //Normal
	  else if ( type==1 ) cen = asc; //Ascendant
	  else if ( type==2 ) cen = des; //Descendant
	  else                cen = (cd->t+cd->y)*0.5; //Middle point
	  
	  //Vertical center
	  cd->noterm[prod->getNoTerm()]->lcen = cen;
	  cd->noterm[prod->getNoTerm()]->rcen = cen;
	}
      
    }
    
    if( insertar ) {
      
      for(int j=0; j<K; j++) {
	if( cd->noterm[j] ) {
	  printf("%12s [%s] %g\n", sym_rec->strClase(cd->noterm[j]->clase),
		 G->key2str(j), exp(cd->noterm[j]->pr) );
	}
      }
      
      //Add to parsing table (size=1)
      tcyk->add(1, cd, -1, G->esInit);
    }
    else
      delete cd;
  }

}





void meParser::combineStrokes(Sample *M, TableCYK *tcyk, LogSpace **LSP, int N) {
  if( N<=1 ) return;

  int asc, cmy, des;
  int clase[NB];
  float pr[NB];
  int ntested=0;

  //Set distance threshold
  float distance_th = segmentsTH;

  //For every single stroke
  for(int stkc1=1; stkc1<N; stkc1++) {

    CellCYK *c1 = new CellCYK(G->noTerminales.size(), N);
    M->setRegion(c1, stkc1);
    
    for(int size=2; size<=min(max_strokes,N); size++) {
      
      list<int> close_list;

      //Add close and visible strokes to the closer list
      if( size==2 ) {
	
	for(int i=0; i<stkc1; i++)
	  if( M->getDist(stkc1, i) < distance_th )
	    close_list.push_back(i);

      }
      else
	M->get_close_strokes( stkc1, &close_list, distance_th );
      
      //If there are not enough strokes to compose a hypothesis of "size", continue
      if( (int)close_list.size() < size-1 )
	continue;
      
      int *stkvec = new int[close_list.size()], VS=0;
      for(list<int>::iterator it=close_list.begin(); it!=close_list.end(); it++)
	stkvec[VS++] = *it;
      
      sort(stkvec, stkvec+VS);
      
      for(int i=size-2; i<VS; i++) {
	list<int> stks_list;
	
	//Add stkc1 and current stroke (ith)
	stks_list.push_back( stkvec[i] );
	stks_list.push_back( stkc1 );
	
	//Add strokes up to size
	for(int j=i-(size-2); j<i; j++)
	  stks_list.push_back( stkvec[j] );
	
	//Sort list (stroke's order is important in online classification)
	stks_list.sort();
	
	CellCYK *cd = new CellCYK(G->noTerminales.size(), N);
	M->setRegion(cd, &stks_list);
	
	//Print hypothesis information
	printf("Multi-stroke (%d) hypothesis: {", size);
	for(list<int>::iterator it=stks_list.begin(); it!=stks_list.end(); it++)
	  printf(" %d", *it);
	printf(" }\n");
	
	float seg_prob = segmentation->prob(cd,M);
	
	cmy = sym_rec->clasificar(M, &stks_list, NB, clase, pr, &asc, &des);
	
	ntested++;
	
	//Add to parsing table
	bool insertar=false;
	for(list<ProductionT *>::iterator it=G->prodTerms.begin(); it!=G->prodTerms.end(); it++) {
	  ProductionT *prod = *it;
	  
	  for(int k=0; k<NB; k++)
	    if( pr[k] > 0.0 && prod->getClase( clase[k] ) && prod->getPrior(clase[k]) > -FLT_MAX ) {
	      
	      float prob = log(InsPen)
		+ ptfactor * prod->getPrior(clase[k])
		+ qfactor  * log(pr[k])
		+ dfactor  * log( duration->prob(clase[k],size) )
		+ gfactor  * log( seg_prob );

	      if( cd->noterm[prod->getNoTerm()] ) {
		if( cd->noterm[prod->getNoTerm()]->pr > prob )
		  continue;
		else
		  delete cd->noterm[prod->getNoTerm()];
	      }
	      
	      insertar=true;
	      
	      cd->noterm[prod->getNoTerm()] = new Hypothesis(clase[k], prob, cd, prod->getNoTerm() );
	      cd->noterm[prod->getNoTerm()]->pt = prod;
	      
	      int cen, type = sym_rec->symType(clase[k]);
	      if( type==0 )       cen = cmy; //Normal
	      else if ( type==1 ) cen = asc; //Ascendant
	      else if ( type==2 ) cen = des; //Descendant
	      else                cen = (cd->t+cd->y)*0.5; //Middle point
	      
	      //Vertical center
	      cd->noterm[prod->getNoTerm()]->lcen = cen;
	      cd->noterm[prod->getNoTerm()]->rcen = cen;
	    }
	}
	
	if( insertar ) {

	  for(int j=0; j<cd->nnt; j++) {
	    if( cd->noterm[j] ) {
	      printf("%12s [%s] %g\n", sym_rec->strClase(cd->noterm[j]->clase),
		     G->key2str(j), exp(cd->noterm[j]->pr));
	    }
	  }
	  tcyk->add(size, cd, -1, G->esInit);

	}
	else
	  delete cd;
	
      }//end for close_list (VS)
      
      delete[] stkvec;
      
    }//end for size
    
  }//end for stroke stkc1

}



//Combine hypotheses A and B to create new hypothesis S using production 'S -> A B'
CellCYK *meParser::fusion(Sample *M, ProductionB *pd, Hypothesis *A, Hypothesis *B, int N, double prob) {
  CellCYK *S=NULL;

  if( !A->parent->compatible(B->parent) || pd->prior == -FLT_MAX )
    return S;

  //Penalty according to distance between strokes
  float grpen;

  if( clusterF > 0.0 ) {

    grpen = M->group_penalty(A->parent, B->parent);
    //If distance is infinity -> not visible
    if( grpen >= M->INF_DIST )
      return NULL;
    
    //Compute penalty
    grpen = 1.0/(1.0 + grpen);
    grpen = pow( grpen, clusterF );
  }
  else
    grpen = 1.0;

  //Get nonterminal
  int ps = pd->S;
  
  //Create new cell
  S = new CellCYK(G->noTerminales.size(), N);
  
  //Compute the (log)probability
  prob = pbfactor * pd->prior + rfactor * log(prob * grpen) + A->pr + B->pr;
  
  //Copute resulting region
  S->x = min(A->parent->x, B->parent->x);
  S->y = min(A->parent->y, B->parent->y);
  S->s = max(A->parent->s, B->parent->s);
  S->t = max(A->parent->t, B->parent->t);
  
  //Set the strokes covered
  S->ccUnion(A->parent,B->parent);
  
  int clase=-1;
  if( !pd->check_out() && sym_rec->checkClase( pd->get_outstr() ) )
    clase = sym_rec->keyClase( pd->get_outstr() );
  
  //Create hypothesis
  S->noterm[ps] = new Hypothesis(clase, prob, S, ps);
  
  pd->mergeRegions(A, B, S->noterm[ps]);
  
  //Save the tree path
  S->noterm[ps]->hi = A;
  S->noterm[ps]->hd = B;
  S->noterm[ps]->prod = pd;
  
  //Special treatment for binary productions that compose terminal symbols (e.g. Equal --V--> Hline Hline)
  if( clase >= 0 ) {
    for(list<ProductionT *>::iterator it=G->prodTerms.begin(); it!=G->prodTerms.end(); it++) {
      ProductionT *prod = *it;
      
      if( prod->getClase( clase ) && prod->getPrior(clase) > -FLT_MAX ) {
	S->noterm[ps]->pt = prod;
	break;
      }
    }
  }

  return S;
}





/*************************************
Parse Math Expression
**************************************/
void meParser::parse_me(Sample *M) {

  M->setSymRec( sym_rec );

  //Compute the normalized size of a symbol for sample M
  M->detRefSymbol();

  int N = M->nStrokes();
  int K = G->noTerminales.size();

  //Cocke-Younger-Kasami (CYK) algorithm for 2D-SCFG
  TableCYK tcyk( N, K );

  printf("CYK table initialization:\n");
  initCYKterms(M, &tcyk, N, K);

  //Compute distances and visibility among strokes
  M->compute_strokes_distances(M->RX, M->RY);

  //Spatial structure for retrieving hypotheses within a certain region
  LogSpace **logspace = new LogSpace*[N];
  list<CellCYK*> c1setH, c1setV, c1setU, c1setI, c1setM, c1setS; 
  SpaRel SPR(gmm_spr, M);

  //Init spatial space for size 1
  logspace[1] = new LogSpace(tcyk.get(1), tcyk.size(1), M->RX, M->RY);

  //Init the parsing table with several multi-stroke symbol segmentation hypotheses
  combineStrokes(M, &tcyk, logspace, N);
  
  printf("\nCYK parsing algorithm\n");
  printf("Size 1: Generated %d\n", tcyk.size(1));
  
  //CYK algorithm main loop
  for(int talla=2; talla<=N; talla++) {

    for(int a=1; a<talla; a++) {
      int b = talla-a;

      for(CellCYK *c1=tcyk.get(a); c1; c1=c1->sig) {
	//Clear lists
	c1setH.clear();
	c1setV.clear();
	c1setU.clear();
	c1setI.clear();
	c1setM.clear();
	c1setS.clear();

	//Get the subset of regions close to c1 according to different spatial relations
	logspace[b]->getH(c1, &c1setH); //Horizontal (right)
	logspace[b]->getV(c1, &c1setV); //Vertical (down)
	logspace[b]->getU(c1, &c1setU); //Vertical (up)
	logspace[b]->getI(c1, &c1setI); //Inside (sqrt)
	logspace[b]->getM(c1, &c1setM); //mroot (sqrt[i])

	for(list<CellCYK*>::iterator c2=c1setH.begin(); c2!=c1setH.end(); c2++) {

	  for(list<ProductionB*>::iterator it=G->prodsH.begin(); it!=G->prodsH.end(); it++) {
	    if( (*it)->prior == -FLT_MAX ) continue;

	    //Production S -> A B
	    int ps = ((ProductionB*)*it)->S;
	    int pa = ((ProductionB*)*it)->A;
	    int pb = ((ProductionB*)*it)->B;

	    if( c1->noterm[ pa ] && (*c2)->noterm[ pb ] ) {
	      double cdpr = SPR.getHorProb(c1->noterm[pa], (*c2)->noterm[ pb ]);
	      if( cdpr <= 0.0 ) continue;

	      CellCYK *cd = fusion(M, *it, c1->noterm[ pa ], (*c2)->noterm[ pb ], M->nStrokes(), cdpr);

	      if( !cd ) continue;

	      if( cd->noterm[ps] ) {
		tcyk.add(talla, cd, ps, G->esInit); //Add to parsing table (size=talla)
	      }
	      else {
		tcyk.add(talla, cd, -1, G->esInit); //Add to parsing table
	      }
	    }
	  }

	  for(list<ProductionB*>::iterator it=G->prodsSup.begin(); it!=G->prodsSup.end(); it++) {
	    if( (*it)->prior == -FLT_MAX ) continue;
	      
	    //Production S -> A B
	    int ps = ((ProductionB*)*it)->S;
	    int pa = ((ProductionB*)*it)->A;
	    int pb = ((ProductionB*)*it)->B;
	      
	    if( c1->noterm[ pa ] && (*c2)->noterm[ pb ] ) {
	      double cdpr = SPR.getSupProb(c1->noterm[pa], (*c2)->noterm[ pb ]);
	      if( cdpr <= 0.0 ) continue;
		
	      CellCYK *cd = fusion(M, *it, c1->noterm[ pa ], (*c2)->noterm[ pb ], M->nStrokes(), cdpr);
		  
	      if( !cd ) continue;

	      if( cd->noterm[ps] ) {
		tcyk.add(talla, cd, ps, G->esInit); //Add to parsing table
	      }
	      else {
		tcyk.add(talla, cd, -1, G->esInit); //Add to parsing table
	      }
	      
	    }
	  }


	  for(list<ProductionB*>::iterator it=G->prodsSub.begin(); it!=G->prodsSub.end(); it++) {
	    if( (*it)->prior == -FLT_MAX ) continue;

	    //Production S -> A B
	    int ps = ((ProductionB*)*it)->S;
	    int pa = ((ProductionB*)*it)->A;
	    int pb = ((ProductionB*)*it)->B;

	    if( c1->noterm[ pa ] && (*c2)->noterm[ pb ] ) {
	      double cdpr = SPR.getSubProb(c1->noterm[pa], (*c2)->noterm[ pb ]);
	      if( cdpr <= 0.0 ) continue;
      
	      CellCYK *cd = fusion(M, *it, c1->noterm[ pa ], (*c2)->noterm[ pb ], M->nStrokes(), cdpr);

	      if( !cd ) continue;

	      if( cd->noterm[ps] ) {
		tcyk.add(talla, cd, ps, G->esInit); //Add to parsing table
	      }
	      else {
		tcyk.add(talla, cd, -1, G->esInit); //Add to parsing table
	      }
	      
	      
	    }
	  }

	}//end c2=c1setH

	for(list<CellCYK*>::iterator c2=c1setV.begin(); c2!=c1setV.end(); c2++) {
	  
	  for(list<ProductionB*>::iterator it=G->prodsV.begin(); it!=G->prodsV.end(); it++) {
	    if( (*it)->prior == -FLT_MAX ) continue;

	    //Production S -> A B
	    int ps = ((ProductionB*)*it)->S;
	    int pa = ((ProductionB*)*it)->A;
	    int pb = ((ProductionB*)*it)->B;

	    if( c1->noterm[ pa ] && (*c2)->noterm[ pb ] ) {
	      double cdpr = SPR.getVerProb(c1->noterm[pa], (*c2)->noterm[ pb ]);
	      if( cdpr <= 0.0 ) continue;

	      CellCYK *cd = fusion(M, *it, c1->noterm[ pa ], (*c2)->noterm[ pb ], M->nStrokes(), cdpr);

	      if( !cd ) continue;

	      if( cd->noterm[ps] )
		tcyk.add(talla, cd, ps, G->esInit); //Add to parsing table
	      else
		tcyk.add(talla, cd, -1, G->esInit); //Add to parsing table
	    }
	  }

	  //prodsVe
	  for(list<ProductionB*>::iterator it=G->prodsVe.begin(); it!=G->prodsVe.end(); it++) {
	    if( (*it)->prior == -FLT_MAX ) continue;

	    //Production S -> A B
	    int ps = ((ProductionB*)*it)->S;
	    int pa = ((ProductionB*)*it)->A;
	    int pb = ((ProductionB*)*it)->B;

	    if( c1->noterm[ pa ] && (*c2)->noterm[ pb ] ) {
	      double cdpr = SPR.getVerProb(c1->noterm[pa], (*c2)->noterm[ pb ], true);
	      if( cdpr <= 0.0 ) continue;

	      CellCYK *cd = fusion(M, *it, c1->noterm[ pa ], (*c2)->noterm[ pb ], M->nStrokes(), cdpr);

	      if( !cd ) continue;

	      if( cd->noterm[ps] ) {
		tcyk.add(talla, cd, ps, G->esInit); //Add to parsing table
	      }
	      else  {
		tcyk.add(talla, cd, -1, G->esInit); //Add to parsing table
	      }
		
	    }
	  }

	}//for in c1setV


	for(list<CellCYK*>::iterator c2=c1setU.begin(); c2!=c1setU.end(); c2++) {
	  
	  for(list<ProductionB*>::iterator it=G->prodsV.begin(); it!=G->prodsV.end(); it++) {
	    if( (*it)->prior == -FLT_MAX ) continue;

	    //Production S -> A B
	    int ps = ((ProductionB*)*it)->S;
	    int pa = ((ProductionB*)*it)->A;
	    int pb = ((ProductionB*)*it)->B;
	    
	    if( c1->noterm[ pb ] && (*c2)->noterm[ pa ] ) {
	      double cdpr = SPR.getVerProb((*c2)->noterm[pa], c1->noterm[ pb ]);
	      if( cdpr <= 0.0 ) continue;

	      CellCYK *cd = fusion(M, *it, (*c2)->noterm[ pa ], c1->noterm[ pb ], M->nStrokes(), cdpr);
		
	      if( !cd ) continue;

	      if( cd->noterm[ps] ) {
		tcyk.add(talla, cd, ps, G->esInit); //Add to parsing table
	      }
	      else {
		tcyk.add(talla, cd, -1, G->esInit); //Add to parsing table
	      }

	    }
	  }


	  //ProdsVe
	  for(list<ProductionB*>::iterator it=G->prodsVe.begin(); it!=G->prodsVe.end(); it++) {
	    if( (*it)->prior == -FLT_MAX ) continue;

	    //Production S -> A B
	    int ps = ((ProductionB*)*it)->S;
	    int pa = ((ProductionB*)*it)->A;
	    int pb = ((ProductionB*)*it)->B;
	    
	    if( c1->noterm[ pb ] && (*c2)->noterm[ pa ] ) {
	      double cdpr = SPR.getVerProb((*c2)->noterm[pa], c1->noterm[ pb ], true);
	      if( cdpr <= 0.0 ) continue;

	      CellCYK *cd = fusion(M, *it, (*c2)->noterm[ pa ], c1->noterm[ pb ], M->nStrokes(), cdpr);
		
	      if( !cd ) continue;

	      if( cd->noterm[ps] ) {
		tcyk.add(talla, cd, ps, G->esInit); //Add to parsing table
	      }
	      else {
		tcyk.add(talla, cd, -1, G->esInit); //Add to parsing table
	      }

	    }
	  }

	}

	for(list<CellCYK*>::iterator c2=c1setI.begin(); c2!=c1setI.end(); c2++) {
	  
	  for(list<ProductionB*>::iterator it=G->prodsIns.begin(); it!=G->prodsIns.end(); it++) {
	    if( (*it)->prior == -FLT_MAX ) continue;

	    //Production S -> A B
	    int ps = ((ProductionB*)*it)->S;
	    int pa = ((ProductionB*)*it)->A;
	    int pb = ((ProductionB*)*it)->B;
	      
	    if( c1->noterm[ pa ] && (*c2)->noterm[ pb ] ) {
	      double cdpr = SPR.getInsProb(c1->noterm[pa], (*c2)->noterm[ pb ]);
	      if( cdpr <= 0.0 ) continue;

	      CellCYK *cd = fusion(M, *it, c1->noterm[ pa ], (*c2)->noterm[ pb ], M->nStrokes(), cdpr);
		
	      if( !cd ) continue;

	      if( cd->noterm[ps] ) {
		tcyk.add(talla, cd, ps, G->esInit); //Add to parsing table
	      }
	      else {
		tcyk.add(talla, cd, -1, G->esInit); //Add to parsing table
	      }
		
	    }
	  }
	}


	//Mroot
	for(list<CellCYK*>::iterator c2=c1setM.begin(); c2!=c1setM.end(); c2++) {
	  
	  for(list<ProductionB*>::iterator it=G->prodsMrt.begin(); it!=G->prodsMrt.end(); it++) {
	    if( (*it)->prior == -FLT_MAX ) continue;

	    //Production S -> A B
	    int ps = ((ProductionB*)*it)->S;
	    int pa = ((ProductionB*)*it)->A;
	    int pb = ((ProductionB*)*it)->B;
	      
	    if( c1->noterm[ pa ] && (*c2)->noterm[ pb ] ) {
	      double cdpr = SPR.getMrtProb(c1->noterm[pa], (*c2)->noterm[ pb ]);
	      if( cdpr <= 0.0 ) continue;

	      CellCYK *cd = fusion(M, *it, c1->noterm[ pa ], (*c2)->noterm[ pb ], M->nStrokes(), cdpr);
		
	      if( !cd ) continue;

	      if( cd->noterm[ps] ) {
		tcyk.add(talla, cd, ps, G->esInit); //Add to parsing table
	      }
	      else {
		tcyk.add(talla, cd, -1, G->esInit); //Add to parsing table
	      }
		
	    }
	  }
	}
	//End Mroot


	//Look for combining {x_subs} y {x^sups} in {x_subs^sups}
	for(int pps=0; pps<c1->nnt; pps++) {

	  //If c1->noterm[pa] is a Hypothesis of a subscript (parent_son)
	  if( c1->noterm[pps] && c1->noterm[pps]->prod && c1->noterm[pps]->prod->tipo() == 'B' ) {

	    logspace[b+c1->noterm[pps]->hi->parent->talla]->getS(c1, &c1setS); //sup/sub-scripts union
	    
	    for(list<CellCYK*>::iterator c2=c1setS.begin(); c2!=c1setS.end(); c2++) {
	      
	      if( (*c2)->x == c1->x && c1 != *c2 ) {
		
		for(list<ProductionB*>::iterator it=G->prodsSSE.begin(); it!=G->prodsSSE.end(); it++) {
		  if( (*it)->prior == -FLT_MAX ) continue;
		  
		  //Production S -> A B
		  int ps = ((ProductionB*)*it)->S;
		  int pa = ((ProductionB*)*it)->A;
		  int pb = ((ProductionB*)*it)->B;
		  
		  if( c1->noterm[pa] && (*c2)->noterm[pb] 
		      && c1->noterm[pa]->prod && (*c2)->noterm[pb]->prod
		      && c1->noterm[pa]->hi == (*c2)->noterm[pb]->hi
		      && c1->noterm[pa]->prod->tipo() == 'B'
		      && (*c2)->noterm[pb]->prod->tipo() == 'P'
		      && c1->noterm[pa]->hd->parent->compatible( (*c2)->noterm[pb]->hd->parent ) ) {
		    
		    
		    //Subscript and superscript should start almost vertically aligned
		    if( abs(c1->noterm[pa]->hd->parent->x - (*c2)->noterm[pb]->hd->parent->x) > 3*M->RX ) continue;
		    //Subscript and superscript should not overlap
		    if( max((*it)->solape(c1->noterm[pa]->hd, (*c2)->noterm[pb]->hd),
			    (*it)->solape((*c2)->noterm[pb]->hd, c1->noterm[pa]->hd) ) > 0.1 ) continue;
		    
		    float prob = c1->noterm[pa]->pr + (*c2)->noterm[pb]->pr - c1->noterm[pa]->hi->pr;
		    
		    CellCYK *cd = new CellCYK(G->noTerminales.size(), M->nStrokes() );
		    
		    cd->x = min(c1->x, (*c2)->x);
		    cd->y = min(c1->y, (*c2)->y);
		    cd->s = max(c1->s, (*c2)->s);
		    cd->t = max(c1->t, (*c2)->t);
		    
		    cd->noterm[ ps ] = new Hypothesis(-1, prob, cd, ps);
		    
		    cd->noterm[ ps ]->lcen = c1->noterm[pa]->lcen;
		    cd->noterm[ ps ]->rcen = c1->noterm[pa]->rcen;
		    cd->ccUnion(c1,(*c2));
		    
		    cd->noterm[ ps ]->hi = c1->noterm[pa];
		    cd->noterm[ ps ]->hd = (*c2)->noterm[pb]->hd;
		    cd->noterm[ ps ]->prod = *it;
		    //Save the production of the superscript in order to recover it when printing the used productions
		    cd->noterm[ ps ]->prod_sse = (*c2)->noterm[pb]->prod;
		    
		    tcyk.add(talla, cd, ps, G->esInit);
		  }
		}
	      }

	    }//end for c2 in c1setS

	    c1setS.clear();
	  }
	}//end for(int pps=0; pps<c1->nnt; pps++)


      } //end for(CellCYK *c1=tcyk.get(a); c1; c1=c1->sig)

    } //for 1 <= a < talla
    
    if( talla < N ) {
      //Create new logspace structure of size "talla"
      logspace[talla] = new LogSpace(tcyk.get(talla), tcyk.size(talla), M->RX, M->RY);
    }

    printf("Size %d: Generated %d\n", talla, tcyk.size(talla));

#ifdef VERBOSE
    for(CellCYK *cp=tcyk.get(talla); cp; cp=cp->sig) {
      printf("  (%3d,%3d)-(%3d,%3d) { ", cp->x, cp->y, cp->s, cp->t);
      for(int i=0; i<cp->nnt; i++)
	if( cp->noterm[i] ) printf("%g[%s] ", cp->noterm[i]->pr, G->key2str(i));
      printf("}\n");
    }
    printf("\n");
#endif

  } //for 2 <= talla <= N


  //Free memory
  for(int i=1; i<N; i++)
    delete logspace[i];
  delete[] logspace;

  //Get Most Likely Hypothesis
  Hypothesis *mlh = tcyk.getMLH();

  if( !mlh ) {
    fprintf(stderr, "\nNo hypothesis found!!\n");
    exit(1);
  }

  printf("\nMost Likely Hypothesis (%d strokes)\n\n", mlh->parent->talla);

  printf("Math Symbols:\n");
  print_symrec(mlh);
  printf("\n");

  printf("LaTeX:\n");
  print_latex( mlh );

  //Save InkML file of the recognized expression
  M->printInkML( G, mlh );

  if( M->getOutDot() )
    save_dot( mlh, M->getOutDot() );
}

/*************************************
End Parsing Math Expression
*************************************/


void meParser::print_symrec(Hypothesis *H) {
  if( !H->pt ) {
    print_symrec( H->hi );
    print_symrec( H->hd );
  }
  else {
    string clatex = H->pt->getTeX( H->clase );
    
    printf("%s {", clatex.c_str());
    
    for(int i=0; i<H->parent->nc; i++)
      if( H->parent->ccc[i] )
	printf(" %d", i);
    printf(" }\n");
  }
}

void meParser::print_latex(Hypothesis *H) {
  //printf("\\displaystyle ");
  if( !H->pt )
    H->prod->printOut( G, H );
  else {
    string clatex = H->pt->getTeX( H->clase );
    printf("%s", clatex.c_str() );
  }
  printf("\n");
}


void meParser::save_dot( Hypothesis *H, char *outfile ) {
  FILE *fd=fopen(outfile, "w");
  if( !fd )
    fprintf(stderr, "Error creating '%s' file\n", outfile);
  else {
    fprintf(fd, "digraph mathExp{\n");
    tree2dot(fd, H, 0);
    fprintf(fd, "}\n");
  }
  fclose(fd);
}


int meParser::tree2dot(FILE *fd, Hypothesis *H, int id) {
  int nid;

  if( !H->pt ) {
    //Binary production
    int a = H->prod->A;
    int b = H->prod->B;
    
    fprintf(fd, "%s%d -> %s%d [label=%c]\n", G->key2str(H->ntid), id, G->key2str(a), id+1, H->prod->tipo());
    
    nid = tree2dot(fd, H->hi, id+1);

    fprintf(fd, "%s%d -> %s%d [label=%c]\n", G->key2str(H->ntid), id, G->key2str(b), nid, H->prod->tipo());

    nid = tree2dot(fd, H->hd, nid);
  }
  else {
    string aux = H->pt->getTeX(H->clase);
    for(int i=0; aux[i]; i++) {
      if( aux[i] == '\\' )aux[i]='s';
      if( aux[i] == '+' ) aux[i]='p';
      if( aux[i] == '-' ) aux[i]='m';
      if( aux[i] == '(' ) aux[i]='L';
      if( aux[i] == ')' ) aux[i]='R';
      if( aux[i] == '=' ) aux[i]='e';
    }

    //Terminal production
    fprintf(fd, "T%s%d [shape=box,label=\"%s\"]\n", aux.c_str(), id, H->pt->getTeX(H->clase));
    fprintf(fd, "%s%d -> T%s%d\n", 
	    G->key2str(H->pt->getNoTerm()), id, aux.c_str(), id);

    nid = id+1;
  }

  return nid;
}
