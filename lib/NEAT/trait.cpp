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

#include "trait.h"

#include <iostream>
#include <sstream>
using namespace NEAT;

Trait::Trait () {
	for (int count=0;count<NEAT::num_trait_params;count++)
		params[count]=0;
	trait_id=0;
}

Trait::Trait(int id,double p1,double p2,double p3,double p4,double p5,double p6,double p7,double p8,double p9) {
	trait_id=id;
	params[0]=p1;
	params[1]=p2;
	params[2]=p3;
	params[3]=p4;
	params[4]=p5;
	params[5]=p6;
	params[6]=p7;
	params[7]=0;
}

Trait::Trait(const Trait& t) {
	for(int count=0; count < NEAT::num_trait_params; count++)
		params[count]=(t.params)[count];

	trait_id = t.trait_id;
}


Trait::Trait(Trait *t) {
	for(int count=0;count<NEAT::num_trait_params;count++)
		params[count]=(t->params)[count];

	trait_id=t->trait_id;
}


Trait::Trait(const char *argline) {

    std::stringstream ss(argline);
	//Read in trait id
 //   std::string curword;
	//char delimiters[] = " \n";
	//int curwordnum = 0;

	//strcpy(curword, NEAT::getUnit(argline, curwordnum++, delimiters));
    
//	trait_id = atoi(curword);
    ss >> trait_id;

    //std::cout << ss.str() << " trait_id: " << trait_id << std::endl;

	//IS THE STOPPING CONDITION CORRECT?  ALERT
	for(int count=0;count<NEAT::num_trait_params;count++) {
		//strcpy(curword, NEAT::getUnit(argline, curwordnum++, delimiters));
		//params[count] = atof(curword);
        ss >> params[count];
		//iFile>>params[count];
	}

}

Trait::Trait(Trait *t1,Trait *t2) {
	for(int count=0;count<NEAT::num_trait_params;count++)
		params[count]=(((t1->params)[count])+((t2->params)[count]))/2.0;
	trait_id=t1->trait_id;
}

void Trait::print_to_file(std::ofstream &outFile) {
  outFile<<"trait "<<trait_id<<" ";
  for(int count=0;count<NEAT::num_trait_params;count++)
    outFile<<params[count]<<" ";

  outFile<<std::endl;

}


void Trait::print_to_file(std::ostream &outFile) { 

	//outFile<<"trait "<<trait_id<<" ";
	//for(int count=0;count<NEAT::num_trait_params;count++)
	//	outFile<<params[count]<<" ";
	//outFile<<endl;

	char tempbuf[128];
	sprintf(tempbuf, "trait %d ", trait_id);
	outFile << tempbuf;

	for (int count = 0; count<NEAT::num_trait_params;count++) {
		char tempbuf2[128];
		sprintf(tempbuf2, "%f ", params[count]);
		outFile << tempbuf2;
	}

    outFile << std::endl;
}

void Trait::mutate() {
	for(int count=0;count<NEAT::num_trait_params;count++) {
		if (randfloat()>NEAT::trait_param_mut_prob) {
			params[count]+=(randposneg()*randfloat())*NEAT::trait_mutation_power;
			if (params[count]<0) params[count]=0;
			if (params[count]>1.0) params[count]=1.0;
		}
	}
}
