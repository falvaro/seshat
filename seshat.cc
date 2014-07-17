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
#include <cstring>
#include <unistd.h>
#include "grammar.h"
#include "sample.h"
#include "meparser.h"


#define MAXS 4096

using namespace std;

void usage(char *str) {
  fprintf(stderr, "SESHAT - Handwritten math expression parser\nhttps://github.com/falvaro/seshat\n");
  fprintf(stderr, "Copyright (C) 2014, Francisco Alvaro\n\n");
  fprintf(stderr, "Usage: %s -c config -i input [-o output] [-r render.pgm]\n\n", str);
  fprintf(stderr, "  -c config: set the configuration file\n");
  fprintf(stderr, "  -i input:  set the input math expression file\n");
  fprintf(stderr, "  -o output: save recognized expression to 'output' file (InkML format)\n");
  fprintf(stderr, "  -r render: save in 'render' the image representing the input expression (PGM format)\n");
  fprintf(stderr, "  -d graph:  save in 'graph' the description of the recognized tree (DOT format)\n");
}

int main(int argc, char *argv[]) {

  char input[MAXS], output[MAXS], config[MAXS], render[MAXS], dot[MAXS];
  bool rc=false,ri=false,ro=false,rr=false,rd=false;
  input[0] = output[0] = config[0] = 0;

  int option;
  while ((option = getopt (argc, argv, "c:i:o:r:d:")) != -1)
    switch (option) {  
    case 'c': strcpy(config, optarg); rc=true; break;
    case 'i': strcpy(input,  optarg); ri=true; break;
    case 'o': strcpy(output, optarg); ro=true; break;
    case 'r': strcpy(render, optarg); rr=true; break;
    case 'd': strcpy(dot,    optarg); rd=true; break;
    case '?': usage(argv[0]); return -1;
    }
  
  //Check mandatory args
  if( !rc || !ri ) {
    usage(argv[0]);
    return -1;
  }

  //Because some of the feature extraction code uses std::cout/cin
  ios_base::sync_with_stdio(true);

  //Load sample and system configuration
  Sample m( input );
  meParser seshat(config);

  //Render image to file
  if( rr ) m.render_img(render);

  //Set output InkML file
  if( ro ) m.set_out_inkml( output );

  //Set output DOT graph file
  if( rd ) m.set_out_dot( dot );

  //Print sample information
  m.print();
  printf("\n");

  //Parse math expression
  seshat.parse_me(&m);
  
  return 0;
}
