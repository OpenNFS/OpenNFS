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
#ifndef _INNOVATION_H_
#define _INNOVATION_H_

namespace NEAT {

	enum innovtype {
		NEWNODE = 0,
		NEWLINK = 1
	};

	// ------------------------------------------------------------
	// This Innovation class serves as a way to record innovations
	//   specifically, so that an innovation in one genome can be 
	//   compared with other innovations in the same epoch, and if they
	//   are the same innovation, they can both be assigned the same
	//   innovation number.
    //
	//  This class can encode innovations that represent a new link
	//  forming, or a new node being added.  In each case, two 
	//  nodes fully specify the innovation and where it must have
	//  occured.  (Between them)                                     
	// ------------------------------------------------------------ 
	class Innovation {
	private:
		enum innovtype {
			NEWNODE = 0,
			NEWLINK = 1
		};

		//typedef int innovtype;
		//const int NEWNODE = 0;
		//const int NEWLINK = 1;

	public:
		innovtype innovation_type;  //Either NEWNODE or NEWLINK

		int node_in_id;     //Two nodes specify where the innovation took place
		int node_out_id;

		double innovation_num1;  //The number assigned to the innovation
		double innovation_num2;  // If this is a new node innovation, then there are 2 innovations (links) added for the new node 

		double new_weight;   //  If a link is added, this is its weight 
		int new_traitnum; // If a link is added, this is its connected trait 

		int newnode_id;  // If a new node was created, this is its node_id 

		double old_innov_num;  // If a new node was created, this is the innovnum of the gene's link it is being stuck inside 

		bool recur_flag;

		//Constructor for the new node case
		Innovation(int nin,int nout,double num1,double num2,int newid,double oldinnov);

		//Constructor for new link case
		Innovation(int nin,int nout,double num1,double w,int t);

		//Constructor for a recur link
		Innovation(int nin,int nout,double num1,double w,int t,bool recur);

	};

} // namespace NEAT

#endif
