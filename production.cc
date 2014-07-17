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
#include <cstring>
#include <cmath>
#include <cfloat>
#include "hypothesis.h"
#include "production.h"

#define PSOLAP  0.75
#define PENALTY 0.05

using namespace std;

//Aux functions

int check_str(char *str, char *pat) {
  for(int i=0; str[i]; i++ ) 
    if( str[i] == pat[0] ) {
      int j=1;
      while( str[i+1] && pat[j] ) {
	if( str[i+j] != pat[j] )
	  break;
	j++;
      }
      if( !pat[j] )
	return i;
    }

  return -1;
}

//
// ProductionB methods
//

ProductionB::ProductionB(int s, int a, int b) {
  S = s;
  A = a;
  B = b;
  outStr = NULL;
}

ProductionB::ProductionB(int s, int a, int b, float pr, char *out) {
  S = s;
  A = a;
  B = b;
  prior = pr > 0.0 ? log(pr) : -FLT_MAX;

  setMerges('C');

  outStr = new char[strlen(out)+1];
  strcpy(outStr, out);
}

ProductionB::~ProductionB() {
  if( outStr ) delete[] outStr;
}

//Percentage of the are of regin A that overlaps with region B
float ProductionB::solape(Hypothesis *a, Hypothesis *b) {
  int x = max(a->parent->x, b->parent->x);
  int y = max(a->parent->y, b->parent->y);
  int s = min(a->parent->s, b->parent->s);
  int t = min(a->parent->t, b->parent->t);
  
  if( s >= x && t >= y ) {
    float aSolap = (s-x+1.0)*(t-y+1.0);
    float aTotal = (a->parent->s - a->parent->x+1.0)*(a->parent->t - a->parent->y+1.0);

    return aSolap/aTotal;
  }
  
  return 0.0;
}

bool ProductionB::check_out() {
  if( check_str(outStr, (char*)"$1") < 0 && check_str(outStr, (char*)"$2") < 0 )
    return false;
  return true;
}

char *ProductionB::get_outstr() {
  return outStr;
}


void ProductionB::printOut(Grammar *G, Hypothesis *H) {
  if( outStr ) {

    int pd1 = check_str(outStr, (char*)"$1");
    int pd2 = check_str(outStr, (char*)"$2");
    
    int i=0;
    if( pd2 >= 0 && pd1 >= 0 && pd2 < pd1 ) {
      while( outStr[i]!='$' || outStr[i+1] != '2') {
	putchar(outStr[i]);
	i++;
      }
      i+=2;
      
      if( H->hd->clase < 0 )
	H->hd->prod->printOut( G, H->hd );
      else
	printf("%s", H->hd->pt->getTeX( H->hd->clase ) );
      
      while( outStr[i]!='$' || outStr[i+1] != '1') {
	putchar(outStr[i]);
	i++;
      }
      i+=2;

      if( H->hi->clase < 0 )
	H->hi->prod->printOut( G, H->hi );
      else
	printf("%s", H->hi->pt->getTeX( H->hi->clase ) );
    }
    else {
      if( pd1 >= 0 ) {
	while( outStr[i]!='$' || outStr[i+1] != '1') {
	  putchar(outStr[i]);
	  i++;
	}
	i+=2;
	
	if( H->hi->clase < 0 )
	  H->hi->prod->printOut( G, H->hi );
	else
	  printf("%s", H->hi->pt->getTeX( H->hi->clase ) );
      }
      if( pd2 >= 0 ) {
	while( outStr[i]!='$' || outStr[i+1] != '2') {
	  putchar(outStr[i]);
	  i++;
	}
	i+=2;
	
	if( H->hd->clase < 0 )
	  H->hd->prod->printOut( G, H->hd );
	else
	  printf("%s", H->hd->pt->getTeX( H->hd->clase ) );
      }
    }
    while( outStr[i] ) {
      putchar(outStr[i]);
      i++;
    }
  }
}


void ProductionB::setMerges(char c) {
  merge_cen = c;
}


void ProductionB::mergeRegions(Hypothesis *a, Hypothesis *b, Hypothesis *s) {

  switch( merge_cen ) {
  case 'A': //Data Hypothesis a
    s->lcen = a->lcen;
    s->rcen = a->rcen;
    break;
  case 'B': //Data Hypothesis b
    s->lcen = b->lcen;
    s->rcen = b->rcen;
    break;
  case 'C': //Center point
    s->lcen = (a->parent->y + a->parent->t)/2;
    s->rcen = (b->parent->y + b->parent->t)/2;
    break;
  case 'M': //Mean of both centers
    s->lcen = (a->lcen + b->lcen)/2; //a->lcen;
    s->rcen = (a->rcen + b->rcen)/2; //b->rcen;
    break;
  default:
    fprintf(stderr, "Error: Unrecognized option '%c' in merge regions\n", merge_cen);
    exit(-1);
  }

}


//
//ProductionH methdods
//

ProductionH::ProductionH(int s, int a, int b)
  : ProductionB(s, a, b)
{}

ProductionH::ProductionH(int s, int a, int b, float pr, char *out)
  : ProductionB(s, a, b, pr, out)
{}

void ProductionH::print() {
  printf("%d -> %d : %d\n", S, A, B);
}

char ProductionH::tipo() {
  return 'H';
}


void ProductionH::print_mathml(Grammar *G, Hypothesis *H, FILE *fout, int *nid) {

  if( !H->hi->pt && H->hi->prod->tipo() == 'P'
      && !H->hi->hi->pt && !H->hi->hi->hi->prod 
      && !strcmp(H->hi->hi->hi->pt->getTeX( H->hi->hi->hi->clase ), "(") ) {
    
    //Deal with a bracketed expression such that only the right parenthesis has a superscript
    //this is because CROHME evaluation requires this representation... in a non-ambiguous
    //evaluation escenario this must be removed

    Hypothesis *hip=H->hi->hi;
    while( hip->prod && hip->hd->prod && hip->hd->hd->prod )
      hip = hip->hd;

    Hypothesis *closep = hip->hd->hd;
    Hypothesis *rest=hip->hd;
    hip->hd = hip->hd->hi;

    Hypothesis *hsup = new Hypothesis(-1, 0, NULL, 0);
    hsup->hi = closep;
    hsup->hd = H->hi->hd;
    hsup->prod = H->hi->prod;
    
    Hypothesis *haux = new Hypothesis(-1, 0, NULL, 0);
    haux->hi = hip;
    haux->hd = hsup;
    haux->prod = this;

    Hypothesis *hbig = new Hypothesis(-1, 0, NULL, 0);
    hbig->hi = haux;
    hbig->hd = H->hd;
    hbig->prod = this;

    hbig->prod->print_mathml(G, hbig, fout, nid);
    
    delete hsup;
    delete haux;
    delete hbig;

    //Restore the original tree
    hip->hd=rest;
  }
  else {
    fprintf(fout, "<mrow>\n");

    if( !H->hi->pt )
      H->hi->prod->print_mathml(G, H->hi, fout, nid);
    else {
      char tipo   = H->hi->pt->getMLtype( H->hi->clase );
      char *clase = H->hi->pt->getTeX( H->hi->clase );
      *nid = *nid + 1;

      char inkid[128];
      sprintf(inkid, "%s_%d", clase, *nid);
      H->hi->inkml_id = inkid;

      fprintf(fout, "<m%c xml:id=\"%s\">%s</m%c>\n", 
	      tipo, inkid, clase, tipo);
    }
  
    if( !H->hd->pt )
      H->hd->prod->print_mathml(G, H->hd, fout, nid);
    else {
      char tipo   = H->hd->pt->getMLtype( H->hd->clase );
      char *clase = H->hd->pt->getTeX( H->hd->clase );
      *nid = *nid + 1;

      char inkid[128];
      sprintf(inkid, "%s_%d", clase, *nid);
      H->hd->inkml_id = inkid;

      fprintf(fout, "<m%c xml:id=\"%s\">%s</m%c>\n", 
	      tipo, inkid, clase, tipo);
    }
    
    fprintf(fout, "</mrow>\n");
  }

}



//
//ProductionV methods
//

ProductionV::ProductionV(int s, int a, int b)
  : ProductionB(s, a, b)
{}

ProductionV::ProductionV(int s, int a, int b, float pr, char *out)
  : ProductionB(s, a, b, pr, out)
{}

void ProductionV::print() {
  printf("%d -> %d / %d\n", S, A, B);
}

char ProductionV::tipo() {
  return 'V';
}

void ProductionV::print_mathml(Grammar *G, Hypothesis *H, FILE *fout, int *nid) {

  char *hdhiclass = NULL;
  if( !H->hd->pt && !H->hd->hi->prod )
    hdhiclass = H->hd->hi->pt->getTeX( H->hd->hi->clase );

  if( hdhiclass && ( !strcmp(hdhiclass,"\\sum") || !strcmp(hdhiclass,"\\int") || !strcmp(hdhiclass,"-") ) ) {
    //Special cases: frac || msubsup bigop
    if( !strcmp(hdhiclass,"-") ) {
      *nid = *nid + 1;

      char inkid[128];
      sprintf(inkid, "-_%d", *nid);
      H->hd->hi->inkml_id = inkid;

      //Init mfrac
      fprintf(fout, "<mfrac xml:id=\"%s\">\n", inkid);
      //Numerator
      if( !H->hi->pt )
	H->hi->prod->print_mathml(G, H->hi, fout, nid);
      else {
	char tipo   = H->hi->pt->getMLtype( H->hi->clase );
	char *clase = H->hi->pt->getTeX( H->hi->clase );
	
	*nid = *nid + 1;
	sprintf(inkid, "%s_%d", clase, *nid);
	H->hi->inkml_id = inkid;	

	fprintf(fout, "<m%c xml:id=\"%s\">%s</m%c>\n",
		tipo, inkid, clase, tipo);
      }

      //Denominator
      if( !H->hd->hd->pt )
        H->hd->hd->prod->print_mathml(G, H->hd->hd, fout, nid);
      else {
        char tipo   = H->hd->hd->pt->getMLtype( H->hd->hd->clase );
        char *clase = H->hd->hd->pt->getTeX( H->hd->hd->clase );

        *nid = *nid + 1;
        sprintf(inkid, "%s_%d", clase, *nid);
        H->hd->hd->inkml_id = inkid;

        fprintf(fout, "<m%c xml:id=\"%s\">%s</m%c>\n",
                tipo, inkid, clase, tipo);
      }
      //End mfrac
      fprintf(fout, "</mfrac>\n");
    }
    else {
      //Cases Something V BigOp V Something      
      fprintf(fout, "<munderover>\n");

      //Base: \sum or \lim
      int tipo  = H->hd->hi->pt->getMLtype( H->hd->hi->clase );
      *nid = *nid + 1;

      char inkid[128];
      sprintf(inkid, "%s_%d", hdhiclass, *nid);
      H->hd->hi->inkml_id = inkid;

      fprintf(fout, "<m%c xml:id=\"%s\">%s</m%c>\n",
	      tipo, inkid, hdhiclass, tipo);

      //Under
      if( !H->hd->hd->pt )
        H->hd->hd->prod->print_mathml(G, H->hd->hd, fout, nid);
      else {
        char tipo   = H->hd->hd->pt->getMLtype( H->hd->hd->clase );
	char *clase = H->hd->hd->pt->getTeX( H->hd->hd->clase );

	*nid = *nid + 1;
	sprintf(inkid, "%s_%d", clase, *nid);
	H->hd->hd->inkml_id = inkid;

	fprintf(fout, "<m%c xml:id=\"%s\">%s</m%c>\n",
		tipo, inkid, clase, tipo);
      }

      //Over
      if( !H->hi->pt )
        H->hi->prod->print_mathml(G, H->hi, fout, nid);
      else {
        char tipo   = H->hi->pt->getMLtype( H->hi->clase );
        char *clase = H->hi->pt->getTeX( H->hi->clase );

        *nid = *nid + 1;
        sprintf(inkid, "%s_%d", clase, *nid);
        H->hi->inkml_id = inkid;

        fprintf(fout, "<m%c xml:id=\"%s\">%s</m%c>\n",
		tipo, inkid, clase, tipo);
      }
      //End munderover
      fprintf(fout, "</munderover>\n");

    }

    // End special cases
  } else {

    //Normal production print, munder
    fprintf(fout, "<munder>\n");

    if( !H->hi->pt )
      H->hi->prod->print_mathml(G, H->hi, fout, nid);
    else {
      char tipo   = H->hi->pt->getMLtype( H->hi->clase );
      char *clase = H->hi->pt->getTeX( H->hi->clase );
      
      *nid = *nid + 1;
      char inkid[128];
      sprintf(inkid, "%s_%d", clase, *nid);
      H->hi->inkml_id = inkid;
      
      fprintf(fout, "<m%c xml:id=\"%s\">%s</m%c>\n",
	      tipo, inkid, clase, tipo);
    }

    if( !H->hd->pt )
      H->hd->prod->print_mathml(G, H->hd, fout, nid);
    else {
      char tipo   = H->hd->pt->getMLtype( H->hd->clase );
      char *clase = H->hd->pt->getTeX( H->hd->clase );
      
      *nid = *nid + 1;
      char inkid[128];
      sprintf(inkid, "%s_%d", clase, *nid);
      H->hd->inkml_id = inkid;

      fprintf(fout, "<m%c xml:id=\"%s\">%s</m%c>\n",
	      tipo, inkid, clase, tipo);
    }
    fprintf(fout, "</munder>\n");

  }

}



//
//ProductionVe methods
//

ProductionVe::ProductionVe(int s, int a, int b)
  : ProductionB(s, a, b)
{}

ProductionVe::ProductionVe(int s, int a, int b, float pr, char *out)
  : ProductionB(s, a, b, pr, out)
{}

void ProductionVe::print() {
  printf("%d -> %d /e %d\n", S, A, B);
}

char ProductionVe::tipo() {
  return 'e';
}

void ProductionVe::print_mathml(Grammar *G, Hypothesis *H, FILE *fout, int *nid) {

  char *hdhiclass = NULL;
  if( !H->hd->pt && !H->hd->hi->prod )
    hdhiclass = H->hd->hi->pt->getTeX( H->hd->hi->clase );

  if( hdhiclass && ( !strcmp(hdhiclass,"\\sum") || !strcmp(hdhiclass,"\\int") || !strcmp(hdhiclass,"-") ) ) {
    //Special cases: frac || msubsup bigop
    if( !strcmp(hdhiclass,"-") ) {
      *nid = *nid + 1;

      char inkid[128];
      sprintf(inkid, "-_%d", *nid);
      H->hd->hi->inkml_id = inkid;

      //Init mfrac
      fprintf(fout, "<mfrac xml:id=\"%s\">\n", inkid);
      //Numerator
      if( !H->hi->pt )
	H->hi->prod->print_mathml(G, H->hi, fout, nid);
      else {
	char tipo   = H->hi->pt->getMLtype( H->hi->clase );
	char *clase = H->hi->pt->getTeX( H->hi->clase );
	
	*nid = *nid + 1;
	sprintf(inkid, "%s_%d", clase, *nid);
	H->hi->inkml_id = inkid;	

	fprintf(fout, "<m%c xml:id=\"%s\">%s</m%c>\n",
		tipo, inkid, clase, tipo);
      }

      //Denominator
      if( !H->hd->hd->pt )
        H->hd->hd->prod->print_mathml(G, H->hd->hd, fout, nid);
      else {
        char tipo   = H->hd->hd->pt->getMLtype( H->hd->hd->clase );
        char *clase = H->hd->hd->pt->getTeX( H->hd->hd->clase );

        *nid = *nid + 1;
        sprintf(inkid, "%s_%d", clase, *nid);
        H->hd->hd->inkml_id = inkid;

        fprintf(fout, "<m%c xml:id=\"%s\">%s</m%c>\n",
                tipo, inkid, clase, tipo);
      }
      //End mfrac
      fprintf(fout, "</mfrac>\n");
    }
    else {
      //Cases Something V BigOp V Something      
      fprintf(fout, "<munderover>\n");

      //Base: \sum or \lim
      int tipo  = H->hd->hi->pt->getMLtype( H->hd->hi->clase );
      *nid = *nid + 1;

      char inkid[128];
      sprintf(inkid, "%s_%d", hdhiclass, *nid);
      H->hd->hi->inkml_id = inkid;

      fprintf(fout, "<m%c xml:id=\"%s\">%s</m%c>\n",
	      tipo, inkid, hdhiclass, tipo);

      //Under
      if( !H->hd->hd->pt )
        H->hd->hd->prod->print_mathml(G, H->hd->hd, fout, nid);
      else {
        char tipo   = H->hd->hd->pt->getMLtype( H->hd->hd->clase );
	char *clase = H->hd->hd->pt->getTeX( H->hd->hd->clase );

	*nid = *nid + 1;
	sprintf(inkid, "%s_%d", clase, *nid);
	H->hd->hd->inkml_id = inkid;

	fprintf(fout, "<m%c xml:id=\"%s\">%s</m%c>\n",
		tipo, inkid, clase, tipo);
      }

      //Over
      if( !H->hi->pt )
        H->hi->prod->print_mathml(G, H->hi, fout, nid);
      else {
        char tipo   = H->hi->pt->getMLtype( H->hi->clase );
        char *clase = H->hi->pt->getTeX( H->hi->clase );

        *nid = *nid + 1;
        sprintf(inkid, "%s_%d", clase, *nid);
        H->hi->inkml_id = inkid;

        fprintf(fout, "<m%c xml:id=\"%s\">%s</m%c>\n",
		tipo, inkid, clase, tipo);
      }
      //End munderover
      fprintf(fout, "</munderover>\n");

    }

    // End special cases
  } else {

    //Normal production print, munder
    fprintf(fout, "<munder>\n");

    if( !H->hi->pt )
      H->hi->prod->print_mathml(G, H->hi, fout, nid);
    else {
      char tipo   = H->hi->pt->getMLtype( H->hi->clase );
      char *clase = H->hi->pt->getTeX( H->hi->clase );
      
      *nid = *nid + 1;
      char inkid[128];
      sprintf(inkid, "%s_%d", clase, *nid);
      H->hi->inkml_id = inkid;
      
      fprintf(fout, "<m%c xml:id=\"%s\">%s</m%c>\n",
	      tipo, inkid, clase, tipo);
    }

    if( !H->hd->pt )
      H->hd->prod->print_mathml(G, H->hd, fout, nid);
    else {
      char tipo   = H->hd->pt->getMLtype( H->hd->clase );
      char *clase = H->hd->pt->getTeX( H->hd->clase );
      
      *nid = *nid + 1;
      char inkid[128];
      sprintf(inkid, "%s_%d", clase, *nid);
      H->hd->inkml_id = inkid;

      fprintf(fout, "<m%c xml:id=\"%s\">%s</m%c>\n",
	      tipo, inkid, clase, tipo);
    }
    fprintf(fout, "</munder>\n");

  }

}



//
//ProductionSSE methods
//

ProductionSSE::ProductionSSE(int s, int a, int b)
  : ProductionB(s, a, b)
{}

ProductionSSE::ProductionSSE(int s, int a, int b, float pr, char *out)
  : ProductionB(s, a, b, pr, out)
{}

void ProductionSSE::print() {
  printf("%d -> %d sse %d\n", S, A, B);
}

char ProductionSSE::tipo() {
  return 'S';
}

void ProductionSSE::print_mathml(Grammar *G, Hypothesis *H, FILE *fout, int *nid) {
  fprintf(fout, "<msubsup>\n");

  if( !H->hi->hi->pt )
    H->hi->hi->prod->print_mathml(G, H->hi->hi, fout, nid);
  else {
    char tipo   = H->hi->hi->pt->getMLtype( H->hi->hi->clase );
    char *clase = H->hi->hi->pt->getTeX( H->hi->hi->clase );
    *nid = *nid + 1;

    char inkid[128];
    sprintf(inkid, "%s_%d", clase, *nid);
    H->hi->hi->inkml_id = inkid;

    fprintf(fout, "<m%c xml:id=\"%s\">%s</m%c>\n", 
    tipo, inkid, clase, tipo);
  }
  
  if( !H->hi->hd->pt )
    H->hi->hd->prod->print_mathml(G, H->hi->hd, fout, nid);
  else {
    char tipo   = H->hi->hd->pt->getMLtype( H->hi->hd->clase );
    char *clase = H->hi->hd->pt->getTeX( H->hi->hd->clase );
    *nid = *nid + 1;

    char inkid[128];
    sprintf(inkid, "%s_%d", clase, *nid);
    H->hi->hd->inkml_id = inkid;

    fprintf(fout, "<m%c xml:id=\"%s\">%s</m%c>\n", 
    tipo, inkid, clase, tipo);
  }

  if( !H->hd->pt )
    H->hd->prod->print_mathml(G, H->hd, fout, nid);
  else {
    char tipo   = H->hd->pt->getMLtype( H->hd->clase );
    char *clase = H->hd->pt->getTeX( H->hd->clase );
    *nid = *nid + 1;

    char inkid[128];
    sprintf(inkid, "%s_%d", clase, *nid);
    H->hd->inkml_id = inkid;

    fprintf(fout, "<m%c xml:id=\"%s\">%s</m%c>\n", 
    tipo, inkid, clase, tipo);
  }
  
  fprintf(fout, "</msubsup>\n");
}



//
//ProductionSup methods
//

ProductionSup::ProductionSup(int s, int a, int b)
  : ProductionB(s, a, b)
{}

ProductionSup::ProductionSup(int s, int a, int b, float pr, char *out)
  : ProductionB(s, a, b, pr, out)
{}

void ProductionSup::print() {
  printf("%d -> %d ^ %d\n", S, A, B);
}

char ProductionSup::tipo() {
  return 'P';
}

void ProductionSup::print_mathml(Grammar *G, Hypothesis *H, FILE *fout, int *nid) {

  if( !H->hi->pt && !H->hi->hi->prod 
      && !strcmp(H->hi->hi->pt->getTeX( H->hi->hi->clase ), "(") ) {

    //Deal with a bracketed expression such that only the right parenthesis has a superscript
    //this is because CROHME evaluation requires this representation... in a non-ambiguous
    //evaluation escenario this must be removed

    Hypothesis *hip=H->hi;
    while( !hip->pt && !hip->hd->pt && !hip->hd->hd->pt )
      hip = hip->hd;

    Hypothesis *closep = hip->hd->hd;
    Hypothesis *rest=hip->hd;
    hip->hd = hip->hd->hi;

    Hypothesis *haux = new Hypothesis(-1, 0, NULL, 0);
    
    haux->hi = closep;
    haux->hd = H->hd;
    haux->prod = this;

    Hypothesis *hbig = new Hypothesis(-1, 0, NULL, 0);
    hbig->hi = H->hi;
    hbig->hd = haux;
    hbig->prod = H->hi->prod;

    hbig->prod->print_mathml(G, hbig, fout, nid);
    
    delete haux;
    delete hbig;

    //Restore original tree
    hip->hd=rest;
  }
  else if( !H->hi->pt && H->hi->prod->tipo() == 'V' && H->hi->hi->pt
	   && !strcmp(H->hi->hi->pt->getTeX( H->hi->hi->clase ), "\\int") ) {
    //Print as msubsup (BigOp [Below] Algo) [Sup] Algo
    fprintf(fout, "<msubsup>\n");

    //\int
    {
      char tipo   = H->hi->hi->pt->getMLtype( H->hi->hi->clase );
      char *clase = H->hi->hi->pt->getTeX( H->hi->hi->clase );
      *nid = *nid + 1;
      
      char inkid[128];
      sprintf(inkid, "%s_%d", clase, *nid);
      H->hi->hi->inkml_id = inkid;
      
      fprintf(fout, "<m%c xml:id=\"%s\">%s</m%c>\n",
	      tipo, inkid, clase, tipo);
    }

    //subscript
    if( !H->hi->hd->pt )
      H->hi->hd->prod->print_mathml(G, H->hi->hd, fout, nid);
    else {
      char tipo   = H->hi->hd->pt->getMLtype( H->hi->hd->clase );
      char *clase = H->hi->hd->pt->getTeX( H->hi->hd->clase );
      *nid = *nid + 1;
      
      char inkid[128];
      sprintf(inkid, "%s_%d", clase, *nid);
      H->hi->hd->inkml_id = inkid;
      
      fprintf(fout, "<m%c xml:id=\"%s\">%s</m%c>\n",
	      tipo, inkid, clase, tipo);
    }

    //superscript
    if( !H->hd->pt )
      H->hd->prod->print_mathml(G, H->hd, fout, nid);
    else {
      char tipo   = H->hd->pt->getMLtype( H->hd->clase );
      char *clase = H->hd->pt->getTeX( H->hd->clase );
      *nid = *nid + 1;
      
      char inkid[128];
      sprintf(inkid, "%s_%d", clase, *nid);
      H->hd->inkml_id = inkid;
      
      fprintf(fout, "<m%c xml:id=\"%s\">%s</m%c>\n",
	      tipo, inkid, clase, tipo);
    }

    fprintf(fout, "</msubsup>\n");

  }
  else {

    fprintf(fout, "<msup>\n");

    if( !H->hi->pt )
      H->hi->prod->print_mathml(G, H->hi, fout, nid);
    else {
      char tipo   = H->hi->pt->getMLtype( H->hi->clase );
      char *clase = H->hi->pt->getTeX( H->hi->clase );
      *nid = *nid + 1;

      char inkid[128];
      sprintf(inkid, "%s_%d", clase, *nid);
      H->hi->inkml_id = inkid;

      fprintf(fout, "<m%c xml:id=\"%s\">%s</m%c>\n", 
      tipo, inkid, clase, tipo);
    }
  
    if( !H->hd->pt )
      H->hd->prod->print_mathml(G, H->hd, fout, nid);
    else {
      char tipo   = H->hd->pt->getMLtype( H->hd->clase );
      char *clase = H->hd->pt->getTeX( H->hd->clase );
      *nid = *nid+1;

      char inkid[128];
      sprintf(inkid, "%s_%d", clase, *nid);
      H->hd->inkml_id = inkid;

      fprintf(fout, "<m%c xml:id=\"%s\">%s</m%c>\n", 
      tipo, inkid, clase, tipo);
    }
  
    fprintf(fout, "</msup>\n");
  }

}




//
//ProductionSub methods
//

ProductionSub::ProductionSub(int s, int a, int b)
  : ProductionB(s, a, b)
{}

ProductionSub::ProductionSub(int s, int a, int b, float pr, char *out)
  : ProductionB(s, a, b, pr, out)
{}

void ProductionSub::print() {
  printf("%d -> %d _ %d\n", S, A, B);
}

char ProductionSub::tipo() {
  return 'B';
}

void ProductionSub::print_mathml(Grammar *G, Hypothesis *H, FILE *fout, int *nid) {
  fprintf(fout, "<msub>\n");

  if( !H->hi->pt )
    H->hi->prod->print_mathml(G, H->hi, fout, nid);
  else {
    char tipo   = H->hi->pt->getMLtype( H->hi->clase );
    char *clase = H->hi->pt->getTeX( H->hi->clase );
    *nid = *nid + 1;

    char inkid[128];
    sprintf(inkid, "%s_%d", clase, *nid);
    H->hi->inkml_id = inkid;

    fprintf(fout, "<m%c xml:id=\"%s\">%s</m%c>\n", 
	    tipo, inkid, clase, tipo);
  }
  
  if( !H->hd->pt )
    H->hd->prod->print_mathml(G, H->hd, fout, nid);
  else {
    char tipo   = H->hd->pt->getMLtype( H->hd->clase );
    char *clase = H->hd->pt->getTeX( H->hd->clase );
    *nid = *nid + 1;

    char inkid[128];
    sprintf(inkid, "%s_%d", clase, *nid);
    H->hd->inkml_id = inkid;

    fprintf(fout, "<m%c xml:id=\"%s\">%s</m%c>\n", 
	    tipo, inkid, clase, tipo);
  }
  
  fprintf(fout, "</msub>\n");
}




//
//ProductionIns methods
//

ProductionIns::ProductionIns(int s, int a, int b)
  : ProductionB(s, a, b)
{}

ProductionIns::ProductionIns(int s, int a, int b, float pr, char *out)
  : ProductionB(s, a, b, pr, out)
{}

void ProductionIns::print() {
  printf("%d -> %d /e %d\n", S, A, B);
}

char ProductionIns::tipo() {
  return 'I';
}

void ProductionIns::print_mathml(Grammar *G, Hypothesis *H, FILE *fout, int *nid) {
  *nid = *nid + 1;

  if( !H->hi->pt && H->hi->prod->tipo() == 'M' ) {
    //Mroot case
    char inkid[128];
    sprintf(inkid, "\\sqrt_%d", *nid);
    H->hi->hd->inkml_id = inkid;

    fprintf(fout, "<mroot xml:id=\"%s\">\n", inkid);

    //Sqrt content
    if( !H->hd->pt )
      H->hd->prod->print_mathml(G, H->hd, fout, nid);
    else {
      char tipo   = H->hd->pt->getMLtype( H->hd->clase );
      char *clase = H->hd->pt->getTeX( H->hd->clase );
      *nid = *nid + 1;
      
      sprintf(inkid, "%s_%d", clase, *nid);
      H->hd->inkml_id = inkid;
      
      fprintf(fout, "<m%c xml:id=\"%s\">%s</m%c>\n", 
	      tipo, inkid, clase, tipo);
    }
    
    //sqrt index
    if( !H->hi->hi->pt )
      H->hi->hi->prod->print_mathml(G, H->hi->hi, fout, nid);
    else {
      char tipo   = H->hi->hi->pt->getMLtype( H->hi->hi->clase );
      char *clase = H->hi->hi->pt->getTeX( H->hi->hi->clase );
      *nid = *nid + 1;
      
      sprintf(inkid, "%s_%d", clase, *nid);
      H->hi->hi->inkml_id = inkid;
      
      fprintf(fout, "<m%c xml:id=\"%s\">%s</m%c>\n", 
	      tipo, inkid, clase, tipo);
    }

    fprintf(fout, "</mroot>\n");
  }
  else {
    //Regular msqrt
    char inkid[128];
    sprintf(inkid, "\\sqrt_%d", *nid);
    H->hi->inkml_id = inkid;

    fprintf(fout, "<msqrt xml:id=\"%s\">\n", inkid);
    
    if( !H->hd->pt )
      H->hd->prod->print_mathml(G, H->hd, fout, nid);
    else {
      char tipo   = H->hd->pt->getMLtype( H->hd->clase );
      char *clase = H->hd->pt->getTeX( H->hd->clase );
      *nid = *nid + 1;
      
      sprintf(inkid, "%s_%d", clase, *nid);
      H->hd->inkml_id = inkid;
      
      fprintf(fout, "<m%c xml:id=\"%s\">%s</m%c>\n", 
	      tipo, inkid, clase, tipo);
    }
    
    fprintf(fout, "</msqrt>\n");
  }
}



//
//ProductionMrt methods
//

ProductionMrt::ProductionMrt(int s, int a, int b)
  : ProductionB(s, a, b)
{}

ProductionMrt::ProductionMrt(int s, int a, int b, float pr, char *out)
  : ProductionB(s, a, b, pr, out)
{}

void ProductionMrt::print() {
  printf("%d -> %d /m %d\n", S, A, B);
}

char ProductionMrt::tipo() {
  return 'M';
}

void ProductionMrt::print_mathml(Grammar *G, Hypothesis *H, FILE *fout, int *nid) {
  *nid = *nid + 1;

  char inkid[128];
  sprintf(inkid, "\\sqrt_%d", *nid);
  H->hi->inkml_id = inkid;

  fprintf(fout, "<mroot xml:id=\"%s\">\n", inkid);

  if( !H->hd->pt )
    H->hd->prod->print_mathml(G, H->hd, fout, nid);
  else {
    char tipo   = H->hd->pt->getMLtype( H->hd->clase );
    char *clase = H->hd->pt->getTeX( H->hd->clase );
    *nid = *nid + 1;

    sprintf(inkid, "%s_%d", clase, *nid);
    H->hd->inkml_id = inkid;

    fprintf(fout, "<m%c xml:id=\"%s\">%s</m%c>\n", 
    tipo, inkid, clase, tipo);
  }


  if( !H->hi->hi->pt )
    H->hi->hi->prod->print_mathml(G, H->hi->hi, fout, nid);
  else {
    char tipo   = H->hd->pt->getMLtype( H->hi->hi->clase );
    char *clase = H->hd->pt->getTeX( H->hi->hi->clase );
    *nid = *nid + 1;

    sprintf(inkid, "%s_%d", clase, *nid);
    H->hd->inkml_id = inkid;

    fprintf(fout, "<m%c xml:id=\"%s\">%s</m%c>\n", 
    tipo, inkid, clase, tipo);
  }

  
  fprintf(fout, "</mroot>\n");
}



//
//ProductionT methods
//

ProductionT::ProductionT(int s, int nclases) {
  S = s;
  N = nclases;
  texStr = new char*[N];
  mltype = new char[N];
  clases = new bool[N];
  probs = new float[N];
  for(int i=0; i<N; i++) {
    clases[i] = false;
    texStr[i] = NULL;
    mltype[i] = 'z';
    probs[i] = 0;
  }
}

ProductionT::~ProductionT() {
  delete[] clases;
  delete[] texStr;
  delete[] probs;
  delete[] mltype;
}

void ProductionT::setClase(int k, float pr, char *tex, char mlt) {
  clases[k] = true;
  if( texStr[k] )
    fprintf(stderr, "WARNING: Terminal %d redefined with label '%s'\n", k, tex);
  else {
    texStr[k] = new char[strlen(tex)+1];
    strcpy(texStr[k], tex);
    probs[k] = pr > 0.0 ? log(pr) : -FLT_MAX;
    mltype[k] = mlt;
  }
}

bool ProductionT::getClase(int k) {
  return clases[k];
}

char *ProductionT::getTeX(int k) {
  return texStr[k];
}

char ProductionT::getMLtype(int k) {
  return mltype[k];
}

float ProductionT::getPrior(int k) {
  return probs[k];
}

int ProductionT::getNoTerm() {
  return S;
}


void ProductionT::print() {
  int nc=0;

  for(int i=0; i<N; i++)
    if( clases[i] )
      nc++;

  printf("%d -> [%d clases]\n", S, nc);
}
