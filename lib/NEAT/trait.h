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
#ifndef _TRAIT_H_
#define _TRAIT_H_

#include <fstream>
#include "neat.h"

namespace NEAT {

	// ------------------------------------------------------------------ 
	// TRAIT: A Trait is a group of parameters that can be expressed     
	//        as a group more than one time.  Traits save a genetic      
	//        algorithm from having to search vast parameter landscapes  
	//        on every node.  Instead, each node can simply point to a trait 
	//        and those traits can evolve on their own 
	class Trait {

		// ************ LEARNING PARAMETERS *********** 
		// The following parameters are for use in    
		//   neurons that learn through habituation,
		//   sensitization, or Hebbian-type processes  

	public:
		int trait_id; // Used in file saving and loading
		double params[NEAT::num_trait_params]; // Keep traits in an array

		Trait ();

		Trait(int id,double p1,double p2,double p3,double p4,double p5,double p6,double p7,double p8,double p9);

		// Copy Constructor
		Trait(const Trait& t);

		// Create a trait exactly like another trait
		Trait(Trait *t);

		// Special constructor off a file assume word "trait" has been read in
		Trait(const char *argline);

		// Special Constructor creates a new Trait which is the average of 2 existing traits passed in
		Trait(Trait *t1,Trait *t2);

		// Dump trait to a stream
        void print_to_file(std::ostream &outFile);
	void print_to_file(std::ofstream &outFile);

		// Perturb the trait parameters slightly
		void mutate();

	};

} // namespace NEAT

#endif
