/*
 Copyright 2001 The University of Texas at Austin

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/
//#include <stdlib.h>
//#include <stdio.h>
//#include <iostream>
//#include <iomanip>
//#include <sstream>
//#include <list>
//#include <vector>
//#include <algorithm>
//#include <cmath>
//#include <iostream.h>
//#include "networks.h"
//#include "neat.h"
//#include "genetics.h"
//#include "experiments.h"
//#include "neatmain.h"
#include <iostream>
#include <vector>
#include "neat.h"
#include "population.h"
#include "experiments.h"
using namespace std;


//  double testdoubval() {
//    return *testdoub;
//  }

//  double testdoubset(double val) {
//    *testdoub=val;
//  }

int main(int argc, char *argv[]) {

  //list<NNode*> nodelist;

  int pause;

  NEAT::Population *p=0;

  /* GTKMM */
//    myapp=new Gtk::Main(argc, argv);

//    testdoub=&val;

  //***********RANDOM SETUP***************//
  /* Seed the random-number generator with current time so that
      the numbers will be different every time we run.    */
  srand( (unsigned)time( NULL ) );

  if (argc != 2) {
    cerr << "A NEAT parameters file (.ne file) is required to run the experiments!" << endl;
    return -1;
  }

  //Load in the params
  NEAT::load_neat_params(argv[1],true);

  cout<<"loaded"<<endl;
  
  /*
  //Test a genome file on pole balancing
  Genome *g;
  Network *n;
  CartPole *thecart;
  thecart=new CartPole(true,0);
  g=Genome::new_Genome_load("tested");
  n=g->genesis(1);
  Organism *org= new Organism(0, g, 1);
  thecart->nmarkov_long=true;
  thecart->generalization_test=false;
  pole2_evaluate(org,0,thecart);
  cout<<"made score "<<org->fitness<<endl;
  cin>>pause;
  */

  
  //Here is an example of how to run an experiment directly from main
  //and then visualize the speciation that took place

  //p=xor_test(100);  //100 generation XOR experiment

  int choice;

  cout<<"Please choose an experiment: "<<endl;
  cout<<"1 - 1-pole balancing"<<endl;
  cout<<"2 - 2-pole balancing, velocity info provided"<<endl;
  cout<<"3 - 2-pole balancing, no velocity info provided (non-markov)"<<endl;
  cout<<"4 - XOR"<<endl;
  cout<<"Number: ";

  cin>>choice;

  switch ( choice )
    {
    case 1:
      p = pole1_test(100);
      break;
    case 2:
      p = pole2_test(100,1);
      break;
    case 3:
      p = pole2_test(100,0);
      break;
    case 4:
      p=xor_test(100);
      break;
    default:
      cout<<"Not an available option."<<endl;
    }


  //p = pole1_test(100); // 1-pole balancing
  //p = pole2_test(100,1); // 2-pole balancing, velocity
  //p = pole2_test(100,0); // 2-pole balancing, no velocity (non-markov)

  if (p)
    delete p;

  return(0);
 
}

