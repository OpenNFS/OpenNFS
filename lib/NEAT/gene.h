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
#ifndef _GENE_H_
#define _GENE_H_

#include "neat.h"
#include "trait.h"
#include "link.h"
#include "network.h"

namespace NEAT {

	class Gene {
	public:

		Link *lnk;
		double innovation_num;
		double mutation_num;  //Used to see how much mutation has changed the link
		bool enable;  //When this is off the Gene is disabled
		bool frozen;  //When frozen, the linkweight cannot be mutated

		//Construct a gene with no trait
		Gene(double w,NNode *inode,NNode *onode,bool recur,double innov,double mnum);

		//Construct a gene with a trait
		Gene(Trait *tp,double w,NNode *inode,NNode *onode,bool recur,double innov,double mnum);

		//Construct a gene off of another gene as a duplicate
		Gene(Gene *g,Trait *tp,NNode *inode,NNode *onode);

		//Construct a gene from a file spec given traits and nodes
		Gene(const char *argline, std::vector<Trait*> &traits, std::vector<NNode*> &nodes);

		// Copy Constructor
		Gene(const Gene& gene);

		~Gene();

		//Print gene to a file- called from Genome
        void print_to_file(std::ostream &outFile);
	void print_to_file(std::ofstream &outFile);
	};

} // namespace NEAT


#endif
