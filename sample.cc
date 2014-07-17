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
#include <unistd.h>
#include <algorithm>
#include <climits>
#include <cfloat>
#include <cstring>
#include <cmath>
#include <map>
#include <vector>
#include <queue>
#include "sample.h"

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/util/XMLUni.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMDocumentType.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/dom/DOMImplementation.hpp>
#include <xercesc/dom/DOMImplementationLS.hpp>
#include <xercesc/dom/DOMNodeIterator.hpp>
#include <xercesc/dom/DOMNodeList.hpp>
#include <xercesc/dom/DOMText.hpp>

#define PI 3.14159265

using namespace std;


bool isRelation(char *str) {
  if( !strcmp(str, "Hor") ) return true;
  if( !strcmp(str, "Sub") ) return true;
  if( !strcmp(str, "Sup") ) return true;
  if( !strcmp(str, "Ver") ) return true;
  if( !strcmp(str, "Ins") ) return true;
  if( !strcmp(str, "Mrt") ) return true;
  return false;
}


Sample::Sample(char *in) {

  RX = RY = 0;
  outinkml = outdot = NULL;

  //Read file extension
  bool isInkML = true;
  const char auxext[7] = ".inkml";
  for(int i=strlen(in)-1, j=0; i>=0 && j<6; i--, j++)
    if( in[i] != auxext[5-j] ) {
      isInkML = false;
      break;
    }

  if( !isInkML )
    loadSCGInk( in );
  else
    loadInkML( in );
  
  ox = oy =  INT_MAX;
  os = ot = -INT_MAX;
  for(int i=0; i<nStrokes(); i++) {
    //Compute bouding box
    if( dataon[i]->rx < ox ) ox = dataon[i]->rx;
    if( dataon[i]->ry < oy ) oy = dataon[i]->ry;
    if( dataon[i]->rs > os ) os = dataon[i]->rs;
    if( dataon[i]->rt > ot ) ot = dataon[i]->rt;
    
    //Compute centroid
    dataon[i]->cx = dataon[i]->cy = 0;
    int np;
    for(np=0; np < dataon[i]->getNpuntos() ; np++) {
      Punto *pto = dataon[ i ]->get(np);
      dataon[i]->cx += pto->x;
      dataon[i]->cy += pto->y;
    }
    dataon[i]->cx /= np;
    dataon[i]->cy /= np;
  }

  //Render image representation
  dataoff = render(&X, &Y);
 }


void Sample::setSymRec( SymRec *sr ){
  SR = sr;
}


void Sample::loadSCGInk(char *str) {
  FILE *fd=fopen(str,"r");
  if( !fd ) {
    fprintf(stderr, "Error loading SCGInk file '%s'\n", str);
    exit(1);
  }

  char line[1024];
  
  fgets(line, 1024, fd);
  if( strcmp(line, "SCG_INK\n") ) {
    fprintf(stderr, "Error: input file format is not SCG_INK\n");
    exit(-1);
  }

  int nstrokes, npuntos;
  fscanf(fd, "%d", &nstrokes);
  
  for(int i=0; i<nstrokes; i++) {
    fscanf(fd, "%d", &npuntos);

    dataon.push_back(new Stroke(npuntos, fd));
  }

  fclose(fd);
}

void Sample::loadInkML(char *str) {
  
  //Check if file exists
  FILE *auxfile=fopen(str,"r");
  if( !auxfile ) {
    fprintf(stderr, "Error loading InkML file '%s'\n", str);
    exit(1);
  }
  fclose(auxfile);

  //If file exists, let the XML library parse the InkML file
  
  try {
    xercesc::XMLPlatformUtils::Initialize();
  }
  catch (const xercesc::XMLException& toCatch) {
    fprintf(stderr, "Error initializing Xerces library: %s\n", xercesc::XMLString::transcode(toCatch.getMessage()));
    exit(1);
  }

  xercesc::XercesDOMParser *inkmlParser;
  XMLCh *TAG_trace, *TAG_traceFormat, *TAG_traceGroup, *TAG_traceView, *TAG_annotation;
  XMLCh *TAG_annotationXML, *TAG_math;
  XMLCh *ATTR_id, *ATTR_xmlid, *ATTR_traceDataRef, *ATTR_href, *ATTR_type;

  TAG_trace = xercesc::XMLString::transcode("trace");
  TAG_traceFormat = xercesc::XMLString::transcode("traceFormat");
  TAG_traceGroup = xercesc::XMLString::transcode("traceGroup");
  TAG_traceView  = xercesc::XMLString::transcode("traceView");
  TAG_annotation = xercesc::XMLString::transcode("annotation");
  TAG_annotationXML = xercesc::XMLString::transcode("annotationXML");
  TAG_math = xercesc::XMLString::transcode("math");
  
  ATTR_id = xercesc::XMLString::transcode("id");
  ATTR_xmlid = xercesc::XMLString::transcode("xml:id");
  ATTR_traceDataRef = xercesc::XMLString::transcode("traceDataRef");
  ATTR_href = xercesc::XMLString::transcode("href");
  ATTR_type = xercesc::XMLString::transcode("type");

  inkmlParser = new xercesc::XercesDOMParser;
  
  
  //Configure DOM parser
  inkmlParser->setValidationScheme( xercesc::XercesDOMParser::Val_Never );
  inkmlParser->setDoNamespaces( false );
  inkmlParser->setDoSchema( false );
  inkmlParser->setLoadExternalDTD( false );


  //Parse document
  try{
    
    inkmlParser->parse( str );
    
    //No need to free this pointer - owned by the parent parser object
    xercesc::DOMDocument* xmlDoc = inkmlParser->getDocument();

    xercesc::DOMElement *elementRoot = xmlDoc->getDocumentElement();
    if( !elementRoot ) fprintf(stderr, "Error: '%s' empty XML document\n", str);
    
    
    xercesc::DOMNodeList*     children = elementRoot->getChildNodes();
    const XMLSize_t nodeCount = children->getLength();
    
    for( XMLSize_t xx = 0; xx < nodeCount; ++xx ) {
      
      xercesc::DOMNode* currentNode = children->item(xx);
      
      if( currentNode->getNodeType() && 
	  currentNode->getNodeType() == xercesc::DOMNode::ELEMENT_NODE ) {
	
	xercesc::DOMElement* currentElement = dynamic_cast< xercesc::DOMElement* >( currentNode );
	if( xercesc::XMLString::equals(currentElement->getTagName(), TAG_trace) ) {
	  
	  const XMLCh* xmlch_id = currentElement->getAttribute(ATTR_id);
	  char *id = xercesc::XMLString::transcode(xmlch_id);
	  
	  int idstroke=atoi(id);
	  next_id = idstroke;

	  xercesc::XMLString::release( &id );
	  
	  xercesc::DOMNodeList *puntos = currentElement->getChildNodes();
	  for( XMLSize_t pp = 0; pp < puntos->getLength(); pp++ ) {
	    xercesc::DOMNode *pt = puntos->item(pp);
	    if( pt->getNodeType() && pt->getNodeType() == xercesc::DOMNode::TEXT_NODE ) {
	      char *aux = xercesc::XMLString::transcode(pt->getNodeValue());

	      dataon.push_back(new Stroke(aux, idstroke));

	      xercesc::XMLString::release( &aux );
	    }
	  }
	  
	}
	else if( xercesc::XMLString::equals(currentElement->getTagName(), TAG_annotation) ) {
	  const XMLCh* xmlch_type = currentElement->getAttribute(ATTR_type);
	  char *type = xercesc::XMLString::transcode(xmlch_type);
	  
	  if( !strcmp(type, "UI") ) {

	    xercesc::DOMNodeList *nodes = currentElement->getChildNodes();
	    for( XMLSize_t pp = 0; pp < nodes->getLength(); pp++ ) {
	      xercesc::DOMNode *pt = nodes->item(pp);
	      if( pt->getNodeType() && pt->getNodeType() == xercesc::DOMNode::TEXT_NODE ) {
		char *aux = xercesc::XMLString::transcode(pt->getNodeValue());
		
		UItag = aux;

		xercesc::XMLString::release( &aux );
	      }
	    }

	  }
	  
	  xercesc::XMLString::release( &type );
	}

      }
    }
  }
  catch( xercesc::XMLException& e ) {
    fprintf(stderr, "Error parsing file '%s': %s\n", str, xercesc::XMLString::transcode(e.getMessage()));
    exit(1);    
  }

  delete inkmlParser;
  xercesc::XMLString::release( &TAG_trace );
  
  xercesc::XMLPlatformUtils::Terminate();

  //Compute bounding box of the input expression
  float xmin=FLT_MAX, ymin=FLT_MAX;
  float xmax=-FLT_MAX, ymax=-FLT_MAX;

  for(int i=0; i<nStrokes(); i++)
    for(int j=0; j<dataon[i]->getNpuntos(); j++) {
      Punto *p = dataon[i]->get(j);
      if( p->x < xmin )	xmin = p->x;
      if( p->x > xmax )	xmax = p->x;
      if( p->y < ymin )	ymin = p->y;
      if( p->y > ymax )	ymax = p->y;
    }

  //Just in case there is only one point or a sequence of points perfectly
  //aligned with the x or y axis
  if( xmax == xmin ) xmax = xmin + 1;
  if( ymax == ymin ) ymax = ymin + 1;

  //Renormalize to height [0,10000] keeping the aspect ratio
  float H=10000;
  float W=H*(xmax-xmin)/(ymax-ymin);

  for(int i=0; i<nStrokes(); i++) {

    dataon[i]->rx = dataon[i]->ry =  INT_MAX;
    dataon[i]->rs = dataon[i]->rt = -INT_MAX;

    for(int j=0; j < dataon[i]->getNpuntos(); j++) {
      Punto *p = dataon[i]->get(j);

      p->x = (int)(W*(p->x-xmin)/(xmax-xmin));
      p->y = (int)(H*(p->y-ymin)/(ymax-ymin));

      //Compute bounding box
      if( p->x < dataon[i]->rx ) dataon[i]->rx = (int)p->x;
      if( p->x > dataon[i]->rs ) dataon[i]->rs = (int)p->x;
      if( p->y < dataon[i]->ry ) dataon[i]->ry = (int)p->y;
      if( p->y > dataon[i]->rt ) dataon[i]->rt = (int)p->y;
    }
  }

}


Sample::~Sample() {
  for(int i=0; i<nStrokes(); i++) {
    delete dataon[i];
    delete[] stk_dis[i];
  }
  delete[] stk_dis;

  for(int y=0; y<Y; y++)
    delete[] dataoff[y];
  delete[] dataoff;


  for(int i=0; i<Y; i++)
    delete[] pix_stk[i];
  delete[] pix_stk;

  if( outinkml ) delete[] outinkml;
  if( outdot ) delete[] outdot;
}

int Sample::get(int x, int y) {
  return dataoff[y][x];
}

Stroke *Sample::getStroke(int i) {
  return dataon[i];
}

int Sample::dimX() {
  return X;
}

int Sample::dimY() {
  return Y;
}

int Sample::nStrokes() {
  return (int)dataon.size();
}

void Sample::render_img(char *out) {
  
  FILE *frender=fopen(out, "w");
  if( frender ) {
    fprintf(frender, "P2\n%d %d\n255\n", X, Y);
    for(int i=0; i<Y; i++) {
      for(int j=0; j<X; j++)
	fprintf(frender, " %3d", dataoff[i][j]);
      fprintf(frender, "\n");
    }
    fclose(frender);
  }
  else
    fprintf(stderr, "WARNING: Error creating file '%s'\n", out);

}


void Sample::set_out_inkml(char *out) {
  outinkml = new char[strlen(out)+1];
  strcpy(outinkml, out);
}

void Sample::set_out_dot(char *out) {
  outdot = new char[strlen(out)+1];
  strcpy(outdot, out);
}

char *Sample::getOutDot() {
  return outdot;
}


void Sample::detRefSymbol() {
  vector<int> vmedx, vmedy;
  int nregs=0, lAr;
  float mAr=0;
  RX=0, RY=0;

  //Compute reference symbol for normalization
  for(int i=0; i<nStrokes(); i++) {
    int ancho = dataon[i]->rs - dataon[i]->rx + 1;
    int alto  = dataon[i]->rt - dataon[i]->ry + 1;
    float aspectratio = (float)ancho/alto;
    int area = ancho*alto;

    vmedx.push_back(ancho);
    vmedy.push_back(alto);

    mAr += area;
    if( aspectratio >= 0.25 && aspectratio <= 4.0 ) {
      RX += ancho;
      RY += alto;
      nregs++;
    }
  }

  //Average area
  mAr /= vmedx.size();
  lAr = (int)(sqrt(mAr)+0.5);
  lAr *= 0.9;

  if( nregs > 0 ) {
    RX /= nregs;
    RY /= nregs;
  }
  else {
    for(int i=0; i<nStrokes(); i++) {
      int ancho = dataon[i]->rs - dataon[i]->rx + 1;
      int alto  = dataon[i]->rt - dataon[i]->ry + 1;
      
      RX += ancho;
      RY += alto;
      nregs++;
    }
    RX /= nregs;
    RY /= nregs;
  }

  //Compute median
  sort(vmedx.begin(),vmedx.end());
  sort(vmedy.begin(),vmedy.end());

  //Reference is the average of (mean,median,avg_area)
  RX = (RX + vmedx[vmedx.size()/2] + lAr)/3.0;
  RY = (RY + vmedy[vmedy.size()/2] + lAr)/3.0;

}


void Sample::setRegion(CellCYK *c, int nStk) {
  c->ccc[nStk] = true;

  c->x = dataon[nStk]->rx;
  c->y = dataon[nStk]->ry;
  c->s = dataon[nStk]->rs;
  c->t = dataon[nStk]->rt;
}

void Sample::setRegion(CellCYK *c, list<int> *LT) {

  c->x = c->y =  INT_MAX;
  c->s = c->t = -INT_MAX;

  for(list<int>::iterator it=LT->begin(); it!=LT->end(); it++) {

    c->ccc[*it] = true;
    
    if( dataon[*it]->rx < c->x )
      c->x = dataon[*it]->rx;
    if( dataon[*it]->ry < c->y )
      c->y = dataon[*it]->ry;
    if( dataon[*it]->rs > c->s )
      c->s = dataon[*it]->rs;
    if( dataon[*it]->rt > c->t )
      c->t = dataon[*it]->rt;

  }

}


void Sample::getAVGstroke_size(float *avgw, float *avgh) {
  *avgw = *avgh = 0.0;
  for(int i=0; i<(int)dataon.size(); i++) {
    *avgw += dataon[i]->rs - dataon[i]->rx;
    *avgh += dataon[i]->rt - dataon[i]->ry;
  }
  *avgw /= (int)dataon.size();
  *avgh /= (int)dataon.size();
}


void Sample::setRegion(CellCYK *c, int *v, int size) {
  c->x = c->y =  INT_MAX;
  c->s = c->t = -INT_MAX;

  for(int i=0; i<size; i++) {

    c->ccc[v[i]] = true;
    
    if( dataon[v[i]]->rx < c->x )
      c->x = dataon[v[i]]->rx;
    if( dataon[v[i]]->ry < c->y )
      c->y = dataon[v[i]]->ry;
    if( dataon[v[i]]->rs > c->s )
      c->s = dataon[v[i]]->rs;
    if( dataon[v[i]]->rt > c->t )
      c->t = dataon[v[i]]->rt;

  }

}

void Sample::print() {
  printf("Number of strokes: %d\n", nStrokes());
  
  // for(int i=0; i<nStrokes(); i++) {
  //   printf("Stroke %d: (%d,%d)-(%d,%d)\n", i, dataon[i]->rx, dataon[i]->ry,
  // 	   dataon[i]->rs, dataon[i]->rt);
  // }
}


void Sample::printInkML(Grammar *G, Hypothesis *H) {

  //If no output file specified, skip
  if( !outinkml ) return;

  FILE *fout=fopen(outinkml, "w");
  if( !fout ) {
    fprintf(stderr, "Error: Can't create inkml output file '%s'\n", outinkml);
    exit(1);
  }

  fprintf(fout, "<ink xmlns=\"http://www.w3.org/2003/InkML\">\n");
  fprintf(fout, "<annotation type=\"UI\">%s</annotation>\n", UItag.c_str());
  fprintf(fout, "<annotationXML type=\"truth\" encoding=\"Content-MathML\">\n");
  //fprintf(fout, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
  fprintf(fout, "<math xmlns='http://www.w3.org/1998/Math/MathML'>\n");

  //Save the value of next_id in order to restore it after printing the MathML expression
  //and generating the IDs for symbols
  int nid_bak = next_id;
  next_id++; //Skip one ID that will be the traceGroup that starts the symbol part

  //Print MathML
  if( !H->pt )
    H->prod->print_mathml(G, H, fout, &next_id);
  else {
    char tipo  = H->pt->getMLtype( H->clase );
    char *clase = H->pt->getTeX( H->clase );

    char inkid[128];
    sprintf(inkid, "%s_%d", clase, next_id+1);
    H->inkml_id = inkid;
    
    fprintf(fout, "<m%c xml:id=\"%s\">%s</m%c>\n", 
	    tipo, H->inkml_id.c_str(), clase, tipo);
  }

  //Restore next_id
  next_id = nid_bak;

  fprintf(fout, "</math>\n");
  fprintf(fout, "</annotationXML>\n");

  //Print the strokes
  for(int i=0; i<nStrokes(); i++) {
    fprintf(fout, "<trace id=\"%d\">\n", i);
    
    Punto *pto = dataon[ i ]->get(0);
    fprintf(fout, "%d %d", (int)pto->x, (int)pto->y);
    for(int np=1; np < dataon[ i ]->getNpuntos() ; np++) {
      pto = dataon[ i ]->get(np);
      fprintf(fout, ", %d %d", (int)pto->x, (int)pto->y);
    }

    fprintf(fout, "\n</trace>\n");
  }

  fprintf(fout, "<traceGroup xml:id=\"%d\">\n", ++next_id);
  fprintf(fout, "<annotation type=\"truth\">Segmentation</annotation>\n");

  //Print the information about symbol segmentation and recognition
  printSymRecInkML(H, fout);

  fprintf(fout, "</traceGroup>\n");
  fprintf(fout, "</ink>\n");

  fclose(fout);
}



void Sample::printSymRecInkML(Hypothesis *H, FILE *fout) {

  //Example (symbol + composed of strokes 3 and 4)

  // <traceGroup xml:id="17">
  //    <annotation type="truth">+</annotation>
  //    <traceView traceDataRef="3"/>
  //    <traceView traceDataRef="4"/>
  //    <annotationXML href="+_1"/>
  // </traceGroup>

  if( H->prod && !H->pt ) {
    printSymRecInkML( H->hi, fout );
    printSymRecInkML( H->hd, fout );
  }
  else {
    next_id++;
    fprintf(fout, "<traceGroup xml:id=\"%d\">\n", next_id);
    fprintf(fout, "  <annotation type=\"truth\">%s</annotation>\n", H->pt->getTeX(H->clase));
    for(int i=0; i<H->parent->nc; i++)
      if( H->parent->ccc[i] ) {
	fprintf(fout, "  <traceView traceDataRef=\"%d\"/>\n", i);
      }
    fprintf(fout, "  <annotationXML href=\"%s\"/>", H->inkml_id.c_str());
    fprintf(fout, "</traceGroup>\n");
  }
}



void Sample::linea(int **img, Punto *pa, Punto *pb, int stkid) {
  const float dl = 3.125e-3;
  int dx = (int)pb->x - (int)pa->x;
  int dy = (int)pb->y - (int)pa->y;

  for(float l=0.0; l < 1.0; l += dl) {
    int x = (int)pa->x + (int)(dx*l+0.5);
    int y = (int)pa->y + (int)(dy*l+0.5);

    for(int i=y-1; i<=y+1; i++)
      for(int j=x-1; j<=x+1; j++) {
	img[i][j] = 0;
	if( stkid>=0 ) pix_stk[i][j] = stkid;
      }
  }
}

void Sample::linea_pbm(int **img, Punto *pa, Punto *pb, int stkid) {
  const float dl = 3.125e-3;
  int dx = (int)pb->x - (int)pa->x;
  int dy = (int)pb->y - (int)pa->y;

  for(float l=0.0; l < 1.0; l += dl) {
    int x = (int)pa->x + (int)(dx*l+0.5);
    int y = (int)pa->y + (int)(dy*l+0.5);

    img[y][x] = 0;
  }
}


int **Sample::render(int *pW, int *pH) {

  int xMAX=-INT_MAX, yMAX=-INT_MAX, xMIN=INT_MAX, yMIN=INT_MAX;

  for(int i=0; i < nStrokes(); i++) {
    
    for(int np=0; np < dataon[ i ]->getNpuntos() ; np++) {
      Punto *pto = dataon[ i ]->get(np);

      if( pto->x > xMAX ) xMAX = pto->x;
      if( pto->x < xMIN ) xMIN = pto->x;
      if( pto->y > yMAX ) yMAX = pto->y;
      if( pto->y < yMIN ) yMIN = pto->y;
    }
    
  }

  //Image dimensions
  int W = xMAX - xMIN + 1;
  int H = yMAX - yMIN + 1;
  float R = (float)W/H;

  //Keeping the aspect ratio (R), scale to 256 pixels height

  H=256;
  W=(int)(H*R);
  if( W<=0 ) W=1;

  //Give some margin to the image
  W += 10;
  H += 10;

  //Create image
  int **img = new int*[H];
  for(int i=0; i<H; i++) {
    img[i] = new int[W];
    for(int j=0; j<W; j++)
      img[i][j] = 255;
  }

  //Create the structure that stores to which stroke belongs each pixel
  pix_stk = new int*[H];
  for(int i=0; i<H; i++) {
    pix_stk[i] = new int[W];
    for(int j=0; j<W; j++)
      pix_stk[i][j] = -1;
  }

  //Render image
  Punto pant, aux, *pto;
  for(int i=0; i<nStrokes(); i++) {
    
    for(int np=0; np < dataon[ i ]->getNpuntos() ; np++) {
      pto = dataon[ i ]->get(np);

      aux.x = 5 + (W-10)*(float)(pto->x - xMIN)/(xMAX - xMIN + 1);
      aux.y = 5 + (H-10)*(float)(pto->y - yMIN)/(yMAX - yMIN + 1);

      img[(int)aux.y][(int)aux.x] = 0;
      pix_stk[(int)aux.y][(int)aux.x] = i;

      //Draw a line between last point and current point
      if( np>=1 )
	linea(img, &pant, &aux, i);
      
      //Update last point
      pant.x = aux.x;
      pant.y = aux.y;
    }
  }

  *pW = W;
  *pH = H;

  IMGxMIN = xMIN;
  IMGyMIN = yMIN;
  IMGxMAX = xMAX;
  IMGyMAX = yMAX;

  return img;
}



void Sample::renderStrokesPBM(list<int> *SL, int ***img, int *rows, int *cols) {
  //Parameters used to render images while training the RNN classifier
  const int REND_H =  40;
  const int REND_W = 200;
  const int OFFSET =   1;

  int xMin, yMin, xMax, yMax, H, W;
  xMin = yMin =  INT_MAX;
  xMax = yMax = -INT_MAX;

  //Calculate bounding box of the region defined by the points
  for(list<int>::iterator it=SL->begin(); it!=SL->end(); it++)
    for(int i=0; i<dataon[*it]->getNpuntos(); i++) {
      Punto *p = dataon[*it]->get(i);

      if( p->x < xMin ) xMin = p->x;
      if( p->y < yMin ) yMin = p->y;
      if( p->x > xMax ) xMax = p->x;
      if( p->y > yMax ) yMax = p->y;
    }

  //Image dimensions
  W = xMax - xMin + 1;
  H = yMax - yMin + 1;

  //Scale image to height REND_H pixels, keeping the aspect ratio
  W = REND_H * (float)W/H;
  H = REND_H;
  
  //If image is too wide (for example, a fraction bar) truncate width to REND_W
  if( W > REND_W )
    W = REND_W;

  //Enforce a minimum size of 3 in both dimensions: height and width
  if( H < 3 ) H = 3;
  if( W < 3 ) W = 3;

  //Create image
  *rows = H+OFFSET*2;
  *cols = W+OFFSET*2;

  *img = new int*[ *rows ];
  for(int i=0; i<*rows; i++) {
    (*img)[i] = new int[ *cols ];
    for(int j=0; j<*cols; j++)
      (*img)[i][j] = 255;
  }

  Punto pant, aux;

  if( SL->size() == 1 && dataon[*(SL->begin())]->getNpuntos() == 1 ) {
    //A single point is represented with a full black image
    for(int i=OFFSET; i<H-OFFSET; i++)
      for(int j=OFFSET; j<W-OFFSET; j++)
	(*img)[i][j] = 0;
  }
  else {

    for(list<int>::iterator it=SL->begin(); it!=SL->end(); it++)
      for(int i=0; i<dataon[*it]->getNpuntos(); i++) {
	Punto *p = dataon[*it]->get(i);

	aux.x = OFFSET + (W-1)*(p->x - xMin)/(float)(xMax-xMin+1);
	aux.y = OFFSET + (H-1)*(p->y - yMin)/(float)(yMax-yMin+1);

	(*img)[(int)aux.y][(int)aux.x] = 0;

	//Draw a line between last point and current point
	if( i>=1 )
	  linea_pbm(*img, &pant, &aux, -1);
	else if( i==0 && dataon[*it]->getNpuntos()==1 )
	  linea_pbm(*img, &aux,  &aux, -1);
    
	//Update last point
	pant = aux;
      }

  }

  //Create smoothed image
  int **img_smo = new int*[ *rows ];
  for(int i=0; i<*rows; i++) {
    img_smo[i] = new int[ *cols ];
    for(int j=0; j<*cols; j++)
      img_smo[i][j] = 0;
  }

  //Smooth AVG(3x3)
  for(int y=0; y<*rows; y++)
    for(int x=0; x<*cols; x++) {
      
      for(int i=y-1; i<=y+1; i++)
  	for(int j=x-1; j<=x+1; j++)
  	  if( i>=0 && j>=0 && i<*rows && j<*cols )
  	    img_smo[y][x] += (*img)[i][j];
  	  else
  	    img_smo[y][x] += 255; //Background
      
      img_smo[y][x] /= 9; //3x3
    }

  //Replace IMG with the smoothed image and free memory
  for(int y=0; y<*rows; y++)
    for(int x=0; x<*cols; x++)
      (*img)[y][x] = img_smo[y][x] < 255 ? 1 : 0;

  for(int y=0; y<H+2*OFFSET; y++)
    delete[] img_smo[y];
  delete[] img_smo;
}



void Sample::getCentroids(CellCYK *cd, int *ce, int *as, int *ds) {
  int regy = INT_MAX, regt=-INT_MAX, N=0;
  *ce = 0;

  for(int i=0; i<cd->nc; i++) 
    if( cd->ccc[i] ) {

      for(int j=0; j<dataon[i]->getNpuntos(); j++) {
	Punto *p = dataon[i]->get(j);
	
	if( dataon[i]->ry < regy )
	  regy = dataon[i]->ry;
	if( dataon[i]->rt > regt )
	  regt = dataon[i]->rt;
	
	*ce += p->y;
	
	N++;
      }
      
    }
  
  *ce /= N;
  *as = (*ce+regt)/2;
  *ds = (regy+*ce)/2;
}





void Sample::compute_strokes_distances(int rx, int ry) {

  //Create distances matrix NxN (strokes)
  stk_dis = new float*[nStrokes()];
  for(int i=0; i<nStrokes(); i++)
    stk_dis[i] = new float[nStrokes()];

  float aux_x = rx;
  float aux_y = ry;
  NORMF    = sqrt(aux_x*aux_x + aux_y*aux_y);
  INF_DIST = FLT_MAX/NORMF;

  //Compute distance among every stroke.
  for(int i=0; i<nStrokes(); i++) {
    stk_dis[i][i] = 0.0;

    for(int j=i+1; j<nStrokes(); j++) {
      stk_dis[i][j] = stroke_distance( i, j )/NORMF;
      stk_dis[j][i] = stk_dis[i][j];
    }
  }

#ifdef VERBOSE
  fprintf(stderr, "===INI Strokes Dist LIST===\n");
  for(int i=0; i<nStrokes(); i++) {
    for(int j=0; j<nStrokes(); j++) {
      if( i!=j && stk_dis[i][j] < INF_DIST )
	fprintf(stderr, "%d -> %d: d=%.2f\n", i, j, stk_dis[i][j]);
    }
    fprintf(stderr, "\n");
  }
  fprintf(stderr, "===END Strokes Dist LIST===\n\n");
  fprintf(stderr, "===INI DISTANCE MATRIX===\n");
  for(int i=0; i<nStrokes(); i++) {
    for(int j=0; j<nStrokes(); j++) {
      if( stk_dis[i][j] >= INF_DIST ) fprintf(stderr, "   *  ");
      else fprintf(stderr, " %5.2f", stk_dis[i][j]);
    }
    fprintf(stderr, "\n");
  }
  fprintf(stderr, "===END DISTANCE MATRIX===\n");
#endif
}


float Sample::stroke_distance(int si, int sj) {
  Punto *pi, *pj, *min_i, *min_j;
  float dmin = FLT_MAX;  

  for(int npi=0; npi < dataon[ si ]->getNpuntos() ; npi++) {
    pi = dataon[ si ]->get(npi);

    for(int npj=0; npj < dataon[ sj ]->getNpuntos() ; npj++) {
      pj = dataon[ sj ]->get(npj);

      float dis = (pi->x - pj->x)*(pi->x - pj->x) + (pi->y - pj->y)*(pi->y - pj->y);
      
      if( dis < dmin ) {
	dmin = dis;
	min_i = pi;
	min_j = pj;
      }
    }
  }

  if( not_visible( si, sj, min_i, min_j ) )
    dmin = FLT_MAX;

  return dmin < FLT_MAX ? sqrt(dmin) : FLT_MAX;
}

float Sample::getDist(int si, int sj) {
  if( si<0 || sj<0 || si>=nStrokes() || sj>=nStrokes() ) {
    fprintf(stderr, "ERROR: stroke id out of range in getDist(%d,%d)\n", si, sj);
    exit(-1);
  }
  return stk_dis[si][sj];
}

//Go through the pixels from pi to pj checking that there is not a pixel that belongs
//to a stroke that is not si or sj. If so, then sj is not visible from si

bool Sample::not_visible(int si, int sj, Punto *pi, Punto *pj) {

  Punto pa, pb;
  //Coordinates in pixels of the rendered image
  pa.x = 5 + (X-10)*(float)(pi->x - IMGxMIN)/(IMGxMAX - IMGxMIN + 1);
  pa.y = 5 + (Y-10)*(float)(pi->y - IMGyMIN)/(IMGyMAX - IMGyMIN + 1);
  pb.x = 5 + (X-10)*(float)(pj->x - IMGxMIN)/(IMGxMAX - IMGxMIN + 1);
  pb.y = 5 + (Y-10)*(float)(pj->y - IMGyMIN)/(IMGyMAX - IMGyMIN + 1);

  
  const float dl = 3.125e-4;
  int dx = (int)pb.x - (int)pa.x;
  int dy = (int)pb.y - (int)pa.y;

  for(float l=0.0; l < 1.0; l += dl) {
    int x = (int)pa.x + (int)(dx*l+0.5);
    int y = (int)pa.y + (int)(dy*l+0.5);

    if( dataoff[y][x] == 0 && pix_stk[y][x] != si && pix_stk[y][x] != sj )
      return true;
  }
  
  return false;
}

bool Sample::visibility(list<int> *strokes_list) {

  map<int,bool> visited;
  for(list<int>::iterator it=strokes_list->begin(); it!=strokes_list->end(); it++)
    visited[*it] = false;

  queue<int> Q;
  Q.push( *(strokes_list->begin()) );
  visited[ *(strokes_list->begin()) ] = true;

  while( !Q.empty() ) {
    int id = Q.front(); Q.pop();

    for(list<int>::iterator it=strokes_list->begin(); it!=strokes_list->end(); it++)
      if( id != *it && !visited[*it] && getDist(id, *it) < INF_DIST ) {
	visited[*it] = true;
	Q.push(*it);
      }
  }

  for(list<int>::iterator it=strokes_list->begin(); it!=strokes_list->end(); it++)
    if( !visited[*it] )
      return false;

  return true;
}

float Sample::group_penalty(CellCYK *A, CellCYK *B) {
  
  //Minimum or single-linkage clustering
  float dmin = FLT_MAX;
  for(int i=0; i<A->nc; i++)
    if( A->ccc[ i ] ) {
      for(int j=0; j<B->nc; j++)
	if( B->ccc[ j ] && j!=i && getDist(i,j) < dmin )
	  dmin = getDist(i,j);
    }

  return dmin;
}

void Sample::get_close_strokes(int id, list<int> *L, float dist_th) {

  bool *added = new bool[id];
  for(int i=0; i<id; i++)
    added[i] = false;

  //Add linked strokes with distance < dist_th
  for(int i=0; i<id; i++) {
    if( getDist(id, i) < dist_th ) {
      L->push_back(i);
      added[i] = true;
    }
  }

  bool updated=true;

  //Add second degree distance < dist_th  
  list<int> auxlist;
  for(list<int>::iterator it=L->begin(); it!=L->end(); it++)
    for(int i=0; i<id; i++)
      if( !added[i] && getDist(*it, i) < dist_th ) {
	auxlist.push_back(i);
	added[i] = true;
      }
  
  for(list<int>::iterator it=auxlist.begin(); it!=auxlist.end(); it++)
    L->push_back( *it );
}
