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
#ifndef _NNODE_H_
#define _NNODE_H_

#include <algorithm>
#include <vector>
#include "neat.h"
#include "trait.h"
#include "link.h"

namespace NEAT {

	enum nodetype {
		NEURON = 0,
		SENSOR = 1
	};

	enum nodeplace {
		HIDDEN = 0,
		INPUT = 1,
		OUTPUT = 2,
		BIAS = 3
	};

	enum functype {
		SIGMOID = 0
	};

	class Link;
	
	class Network;

	// ----------------------------------------------------------------------- 
	// A NODE is either a NEURON or a SENSOR.  
	//   - If it's a sensor, it can be loaded with a value for output
	//   - If it's a neuron, it has a list of its incoming input signals (List<Link> is used) 
	// Use an activation count to avoid flushing
	class NNode {

		friend class Network;
		friend class Genome;

	protected:

		int activation_count;  // keeps track of which activation the node is currently in
		double last_activation; // Holds the previous step's activation for recurrency
		double last_activation2; // Holds the activation BEFORE the prevous step's
		// This is necessary for a special recurrent case when the innode
		// of a recurrent link is one time step ahead of the outnode.
		// The innode then needs to send from TWO time steps ago

		Trait *nodetrait; // Points to a trait of parameters

		int trait_id;  // identify the trait derived by this node

		NNode *dup;       // Used for Genome duplication

		NNode *analogue;  // Used for Gene decoding

		bool override; // The NNode cannot compute its own output- something is overriding it

		double override_value; // Contains the activation value that will override this node's activation

		// Pointer to the Sensor corresponding to this Body.
		//Sensor* mySensor;

	public:
		bool frozen; // When frozen, cannot be mutated (meaning its trait pointer is fixed)

		functype ftype; // type is either SIGMOID ..or others that can be added
		nodetype type; // type is either NEURON or SENSOR 

		double activesum;  // The incoming activity before being processed 
		double activation; // The total activation entering the NNode 
		bool active_flag;  // To make sure outputs are active

		// NOT USED IN NEAT - covered by "activation" above
		double output;  // Output of the NNode- the value in the NNode 

		// ************ LEARNING PARAMETERS *********** 
		// The following parameters are for use in    
		//   neurons that learn through habituation,
		//   sensitization, or Hebbian-type processes  

		double params[NEAT::num_trait_params];

		std::vector<Link*> incoming; // A list of pointers to incoming weighted signals from other nodes
		std::vector<Link*> outgoing;  // A list of pointers to links carrying this node's signal

		// These members are used for graphing with GTK+/GDK
		std::vector<double> rowlevels;  // Depths from output where this node appears
		int row;  // Final row decided upon for drawing this NNode in
		int ypos;
		int xpos;

		int node_id;  // A node can be given an identification number for saving in files

		nodeplace gen_node_label;  // Used for genetic marking of nodes

		NNode(nodetype ntype,int nodeid);

		NNode(nodetype ntype,int nodeid, nodeplace placement);

		// Construct a NNode off another NNode for genome purposes
		NNode(NNode *n,Trait *t);

		// Construct the node out of a file specification using given list of traits
		NNode (const char *argline, std::vector<Trait*> &traits);

		// Copy Constructor
		NNode (const NNode& nnode);

		~NNode();

		// Just return activation for step
		double get_active_out();

		// Return activation from PREVIOUS time step
		double get_active_out_td();

		// Returns the type of the node, NEURON or SENSOR
		const nodetype get_type();

		// Allows alteration between NEURON and SENSOR.  Returns its argument
		nodetype set_type(nodetype);

		// If the node is a SENSOR, returns true and loads the value
		bool sensor_load(double);

		// Adds a NONRECURRENT Link to a new NNode in the incoming List
		void add_incoming(NNode*,double);

		// Adds a Link to a new NNode in the incoming List
		void add_incoming(NNode*,double,bool);

		// Recursively deactivate backwards through the network
		void flushback();

		// Verify flushing for debugging
		void flushback_check(std::vector<NNode*> &seenlist);

		// Print the node to a file
        void  print_to_file(std::ostream &outFile);
	void print_to_file(std::ofstream &outFile);

		// Have NNode gain its properties from the trait
		void derive_trait(Trait *curtrait);

		// Returns the gene that created the node
		NNode *get_analogue();

		// Force an output value on the node
		void override_output(double new_output);

		// Tell whether node has been overridden
		bool overridden();

		// Set activation to the override value and turn off override
		void activate_override();  

		// Writes back changes weight values into the genome
		// (Lamarckian trasnfer of characteristics)
		void Lamarck();

		//Find the greatest depth starting from this neuron at depth d
		int depth(int d,Network *mynet); 

	};


} // namespace NEAT

#endif
