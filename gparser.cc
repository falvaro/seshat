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
#include "gparser.h"
#include <cstdlib>
#include <cstring>

#define SIZE 1024

gParser::gParser(Grammar *gram, FILE *fd, char *path) {
  g = gram;

  int n = strlen(path);

  if( n > 0 ) {
    pre = new char[n+1];
    strcpy(pre, path);
  }
  else {
    pre = new char[1];
    pre[0] = 0;
  }

  parse( fd );
}

gParser::~gParser() {
  delete[] pre;
}

bool gParser::isFillChar(char c) {
  switch(c) {
  case ' ':
  case '\t':
  case '\n':
  case '\r':
    return true;
  default: 
    return false;
  }
}

int gParser::split(char *str,char ***res){
  char tokensaux[2*SIZE];
  int n=0, i=0, j=0;

  while( isFillChar(str[i]) )  i++;

  while( str[i] ) {
    if( str[i] == '\"' ) {
      i++;
      while( str[i] && str[i] != '\"' ) {
	tokensaux[j] = str[i];
	i++; j++;
      }
      i++;
    }
    else {
      while( str[i] && !isFillChar(str[i]) ) {
	tokensaux[j] = str[i];
	i++; j++;
      }
    }
    tokensaux[j++] = 0;
    n++;
    while( str[i] && isFillChar(str[i]) )  i++;
  }

  char **toks=new char*[n];
  for(i=0, j=0; i<n; i++) {
    int tlen = strlen(&tokensaux[j])+1;
    toks[i] = new char[tlen];
    strcpy(toks[i], &tokensaux[j]);
    j += tlen;
  }

  *res = toks;

  return n;
}


bool gParser::nextLine(FILE *fd, char *lin) {
  do{
    if( fgets(lin, SIZE, fd) == NULL )
      return false;
  }while( lin[0]=='#' || strlen(lin)<=1 );

  return true;
}

void gParser::solvePath(char *in, char *out) {
  strcpy(out, pre); //Copy prefix
  strcat(out, in);  //Add the remainding path
  out[strlen(out)-1] = 0; //Remove the final '\n'
}

void gParser::parse(FILE *fd) {
  char linea[SIZE], tok1[SIZE], tok2[SIZE], aux[SIZE];

  //Read nonterminal symbols
  while( nextLine(fd, linea) && strcmp(linea, "START\n") ) {
    sscanf(linea, "%s", tok1);
    g->addNoTerminal(tok1);
  }

  //Read start symbol(s) of the grammar
  while( nextLine(fd, linea) && strcmp(linea, "PTERM\n") ) {
    sscanf(linea, "%s", tok1);
    g->addInitSym(tok1);
  }

  //Read terminal productions
  while( nextLine(fd, linea) && strcmp(linea, "PBIN\n") ) {
    float pr;

    sscanf(linea, "%f %s %s %s", &pr, tok1, tok2, aux);
    
    g->addTerminal(pr, tok1, tok2, aux);
  }

  //Read binary productions
  while( nextLine(fd, linea) ) {
    char **tokens;
    int ntoks = split(linea, &tokens);

    if( ntoks != 7 ) {
      fprintf(stderr, "Error: Grammar not valid (PBIN)\n");
      exit(-1);
    }

    if( !strcmp(tokens[1], "H") )
      g->addRuleH(atof(tokens[0]), tokens[2], tokens[3], tokens[4], tokens[5], tokens[6]);
    else if( !strcmp(tokens[1], "V") )
      g->addRuleV(atof(tokens[0]), tokens[2], tokens[3], tokens[4], tokens[5], tokens[6]);
    else if( !strcmp(tokens[1], "Ve") )
      g->addRuleVe(atof(tokens[0]), tokens[2], tokens[3], tokens[4], tokens[5], tokens[6]);
    else if( !strcmp(tokens[1], "Sup") )
      g->addRuleSup(atof(tokens[0]), tokens[2], tokens[3], tokens[4], tokens[5], tokens[6]);
    else if( !strcmp(tokens[1], "Sub") )
      g->addRuleSub(atof(tokens[0]), tokens[2], tokens[3], tokens[4], tokens[5], tokens[6]);
    else if( !strcmp(tokens[1], "SSE") )
      g->addRuleSSE(atof(tokens[0]), tokens[2], tokens[3], tokens[4], tokens[5], tokens[6]);
    else if( !strcmp(tokens[1], "Ins") )
      g->addRuleIns(atof(tokens[0]), tokens[2], tokens[3], tokens[4], tokens[5], tokens[6]);
    else if( !strcmp(tokens[1], "Mrt") )
      g->addRuleMrt(atof(tokens[0]), tokens[2], tokens[3], tokens[4], tokens[5], tokens[6]);
    else {
      fprintf(stderr, "Error: Binary rule type '%s' nor valid\n", tokens[1]);
      exit(-1);
    }
   
    //Free memory
    for(int j=0; j<ntoks; j++)
      delete[] tokens[j];
    delete[] tokens;
  }

}
