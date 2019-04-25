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
#ifndef _LINK_H_
#define _LINK_H_

#include "neat.h"
#include "trait.h"
#include "nnode.h"

namespace NEAT {

	class NNode;

	// ----------------------------------------------------------------------- 
	// A LINK is a connection from one node to another with an associated weight 
	// It can be marked as recurrent 
	// Its parameters are made public for efficiency 
	class Link {
	public: 
		double weight; // Weight of connection
		NNode *in_node; // NNode inputting into the link
		NNode *out_node; // NNode that the link affects
		bool is_recurrent;
		bool time_delay;

		Trait *linktrait; // Points to a trait of parameters for genetic creation

		int trait_id;  // identify the trait derived by this link

		// ************ LEARNING PARAMETERS *********** 
		// These are link-related parameters that change during Hebbian type learning

		double added_weight;  // The amount of weight adjustment 
		double params[NEAT::num_trait_params];

		Link(double w,NNode *inode,NNode *onode,bool recur);

		// Including a trait pointer in the Link creation
		Link(Trait *lt,double w,NNode *inode,NNode *onode,bool recur);

		// For when you don't know the connections yet
		Link(double w);

		// Copy Constructor
		Link(const Link& link);

		// Derive a trait into link params
		void derive_trait(Trait *curtrait);

	};

} // namespace NEAT

#endif
