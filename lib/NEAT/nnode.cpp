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
#include "nnode.h"
#include <iostream>
#include <sstream>
using namespace NEAT;

NNode::NNode(nodetype ntype,int nodeid) {
	active_flag=false;
	activesum=0;
	activation=0;
	output=0;
	last_activation=0;
	last_activation2=0;
	type=ntype; //NEURON or SENSOR type
	activation_count=0; //Inactive upon creation
	node_id=nodeid;
	ftype=SIGMOID;
	nodetrait=0;
	gen_node_label=HIDDEN;
	dup=0;
	analogue=0;
	frozen=false;
	trait_id=1;
	override=false;
}

NNode::NNode(nodetype ntype,int nodeid, nodeplace placement) {
	active_flag=false;
	activesum=0;
	activation=0;
	output=0;
	last_activation=0;
	last_activation2=0;
	type=ntype; //NEURON or SENSOR type
	activation_count=0; //Inactive upon creation
	node_id=nodeid;
	ftype=SIGMOID;
	nodetrait=0;
	gen_node_label=placement;
	dup=0;
	analogue=0;
	frozen=false;
	trait_id=1;
	override=false;
}

NNode::NNode(NNode *n,Trait *t) {
	active_flag=false;
	activation=0;
	output=0;
	last_activation=0;
	last_activation2=0;
	type=n->type; //NEURON or SENSOR type
	activation_count=0; //Inactive upon creation
	node_id=n->node_id;
	ftype=SIGMOID;
	nodetrait=0;
	gen_node_label=n->gen_node_label;
	dup=0;
	analogue=0;
	nodetrait=t;
	frozen=false;
	if (t!=0)
		trait_id=t->trait_id;
	else trait_id=1;
	override=false;
}

NNode::NNode (const char *argline, std::vector<Trait*> &traits) {
	int traitnum;
	std::vector<Trait*>::iterator curtrait;

	activesum=0;

    std::stringstream ss(argline);
	//char curword[128];
	//char delimiters[] = " \n";
	//int curwordnum = 0;

	//Get the node parameters
	//strcpy(curword, NEAT::getUnit(argline, curwordnum++, delimiters));
	//node_id = atoi(curword);
	//strcpy(curword, NEAT::getUnit(argline, curwordnum++, delimiters));
	//traitnum = atoi(curword);
	//strcpy(curword, NEAT::getUnit(argline, curwordnum++, delimiters));
	//type = (nodetype)atoi(curword);
	//strcpy(curword, NEAT::getUnit(argline, curwordnum++, delimiters));
	//gen_node_label = (nodeplace)atoi(curword);

    int nodety, nodepl;
    ss >> node_id >> traitnum >> nodety >> nodepl;
    type = (nodetype)nodety;
    gen_node_label = (nodeplace)nodepl;

	// Get the Sensor Identifier and Parameter String
	// mySensor = SensorRegistry::getSensor(id, param);
	frozen=false;  //TODO: Maybe change

	//Get a pointer to the trait this node points to
	if (traitnum==0) nodetrait=0;
	else {
		curtrait=traits.begin();
		while(((*curtrait)->trait_id)!=traitnum)
			++curtrait;
		nodetrait=(*curtrait);
		trait_id=nodetrait->trait_id;
	}

	override=false;
}

// This one might be incomplete
NNode::NNode (const NNode& nnode)
{
	active_flag = nnode.active_flag;
	activesum = nnode.activesum;
	activation = nnode.activation;
	output = nnode.output;
	last_activation = nnode.last_activation;
	last_activation2 = nnode.last_activation2;
	type = nnode.type; //NEURON or SENSOR type
	activation_count = nnode.activation_count; //Inactive upon creation
	node_id = nnode.node_id;
	ftype = nnode.ftype;
	nodetrait = nnode.nodetrait;
	gen_node_label = nnode.gen_node_label;
	dup = nnode.dup;
	analogue = nnode.dup;
	frozen = nnode.frozen;
	trait_id = nnode.trait_id;
	override = nnode.override;
}

NNode::~NNode() {
	std::vector<Link*>::iterator curlink;

	//Kill off all incoming links
	for(curlink=incoming.begin();curlink!=incoming.end();++curlink) {
		delete (*curlink);
	}
	//if (nodetrait!=0) delete nodetrait;
}

//Returns the type of the node, NEURON or SENSOR
const nodetype NNode::get_type() {
	return type;
}

//Allows alteration between NEURON and SENSOR.  Returns its argument
nodetype NNode::set_type(nodetype newtype) {
	type=newtype;
	return newtype;
}

//If the node is a SENSOR, returns true and loads the value
bool NNode::sensor_load(double value) {
	if (type==SENSOR) {

		//Time delay memory
		last_activation2=last_activation;
		last_activation=activation;

		activation_count++;  //Puts sensor into next time-step
		activation=value;
		return true;
	}
	else return false;
}

// Note: NEAT keeps track of which links are recurrent and which
// are not even though this is unnecessary for activation.
// It is useful to do so for 2 other reasons: 
// 1. It makes networks visualization of recurrent networks possible
// 2. It allows genetic control of the proportion of connections
//    that may become recurrent

// Add an incoming connection a node
void NNode::add_incoming(NNode *feednode,double weight,bool recur) {
	Link *newlink=new Link(weight,feednode,this,recur);
	incoming.push_back(newlink);
	(feednode->outgoing).push_back(newlink);
}

// Nonrecurrent version
void NNode::add_incoming(NNode *feednode,double weight) {
	Link *newlink=new Link(weight,feednode,this,false);
	incoming.push_back(newlink);
	(feednode->outgoing).push_back(newlink);
}

// Return activation currently in node, if it has been activated
double NNode::get_active_out() {
	if (activation_count>0)
		return activation;
	else return 0.0;
}

// Return activation currently in node from PREVIOUS (time-delayed) time step,
// if there is one
double NNode::get_active_out_td() {
	if (activation_count>1)
		return last_activation;
	else return 0.0;
}

// This recursively flushes everything leading into and including this NNode, including recurrencies
void NNode::flushback() {
	std::vector<Link*>::iterator curlink;

	//A sensor should not flush black
	if (type!=SENSOR) {

		if (activation_count>0) {
			activation_count=0;
			activation=0;
			last_activation=0;
			last_activation2=0;
		}

		//Flush back recursively
		for(curlink=incoming.begin();curlink!=incoming.end();++curlink) {
			//Flush the link itself (For future learning parameters possibility) 
			(*curlink)->added_weight=0;
			if ((((*curlink)->in_node)->activation_count>0))
				((*curlink)->in_node)->flushback();
		}
	}
	else {
		//Flush the SENSOR
		activation_count=0;
		activation=0;
		last_activation=0;
		last_activation2=0;

	}

}

// This recursively checks everything leading into and including this NNode, 
// including recurrencies
// Useful for debugging
void NNode::flushback_check(std::vector<NNode*> &seenlist) {
	std::vector<Link*>::iterator curlink;
	//int pause;
	std::vector<Link*> innodes=incoming;
	std::vector<NNode*>::iterator location;

	if (!(type==SENSOR)) {


		//std::cout<<"ALERT: "<<this<<" has activation count "<<activation_count<<std::endl;
		//std::cout<<"ALERT: "<<this<<" has activation  "<<activation<<std::endl;
		//std::cout<<"ALERT: "<<this<<" has last_activation  "<<last_activation<<std::endl;
		//std::cout<<"ALERT: "<<this<<" has last_activation2  "<<last_activation2<<std::endl;

		if (activation_count>0) {
			std::cout<<"ALERT: "<<this<<" has activation count "<<activation_count<<std::endl;
		}

		if (activation>0) {
			std::cout<<"ALERT: "<<this<<" has activation  "<<activation<<std::endl;
		}

		if (last_activation>0) {
			std::cout<<"ALERT: "<<this<<" has last_activation  "<<last_activation<<std::endl;
		}

		if (last_activation2>0) {
			std::cout<<"ALERT: "<<this<<" has last_activation2  "<<last_activation2<<std::endl;
		}

		for(curlink=innodes.begin();curlink!=innodes.end();++curlink) {
            location = std::find(seenlist.begin(),seenlist.end(),((*curlink)->in_node));
			if (location==seenlist.end()) {
				seenlist.push_back((*curlink)->in_node);
				((*curlink)->in_node)->flushback_check(seenlist);
			}
		}

	}
	else {
		//Flush_check the SENSOR


		std::cout<<"sALERT: "<<this<<" has activation count "<<activation_count<<std::endl;
		std::cout<<"sALERT: "<<this<<" has activation  "<<activation<<std::endl;
		std::cout<<"sALERT: "<<this<<" has last_activation  "<<last_activation<<std::endl;
		std::cout<<"sALERT: "<<this<<" has last_activation2  "<<last_activation2<<std::endl;


		if (activation_count>0) {
			std::cout<<"ALERT: "<<this<<" has activation count "<<activation_count<<std::endl;
		}

		if (activation>0) {
			std::cout<<"ALERT: "<<this<<" has activation  "<<activation<<std::endl;
		}

		if (last_activation>0) {
			std::cout<<"ALERT: "<<this<<" has last_activation  "<<last_activation<<std::endl;
		}

		if (last_activation2>0) {
			std::cout<<"ALERT: "<<this<<" has last_activation2  "<<last_activation2<<std::endl;
		}

	}

}

// Reserved for future system expansion
void NNode::derive_trait(Trait *curtrait) {

	if (curtrait!=0) {
		for (int count=0;count<NEAT::num_trait_params;count++)
			params[count]=(curtrait->params)[count];
	}
	else {
		for (int count=0;count<NEAT::num_trait_params;count++)
			params[count]=0;
	}

	if (curtrait!=0)
		trait_id=curtrait->trait_id;
	else trait_id=1;

}

// Returns the gene that created the node
NNode *NNode::get_analogue() {
	return analogue;
}

// Force an output value on the node
void NNode::override_output(double new_output) {
	override_value=new_output;
	override=true;
}

// Tell whether node has been overridden
bool NNode::overridden() {
	return override;
}

// Set activation to the override value and turn off override
void NNode::activate_override() {
	activation=override_value;
	override=false;
}


void NNode::print_to_file(std::ofstream &outFile) {
  outFile<<"node "<<node_id<<" ";
  if (nodetrait!=0) outFile<<nodetrait->trait_id<<" ";
  else outFile<<"0 ";
  outFile<<type<<" ";
  outFile<<gen_node_label<<std::endl;
}


void NNode::print_to_file(std::ostream &outFile) {
	//outFile<<"node "<<node_id<<" ";
	//if (nodetrait!=0) outFile<<nodetrait->trait_id<<" ";
	//else outFile<<"0 ";
	//outFile<<type<<" ";
	//outFile<<gen_node_label<<std::endl;

	char tempbuf[128];
	sprintf(tempbuf, "node %d ", node_id);
	outFile << tempbuf;

	if (nodetrait != 0) {
		char tempbuf2[128];
		sprintf(tempbuf2, "%d ", nodetrait->trait_id);
		outFile << tempbuf2;
	}
	else outFile << "0 ";

	char tempbuf2[128];
	sprintf(tempbuf2, "%d %d\n", type, gen_node_label);
	outFile << tempbuf2;
}

//Find the greatest depth starting from this neuron at depth d
int NNode::depth(int d, Network *mynet) {
  std::vector<Link*> innodes=this->incoming;
  std::vector<Link*>::iterator curlink;
  int cur_depth; //The depth of the current node
  int max=d; //The max depth

  if (d>100) {
    //std::cout<<mynet->genotype<<std::endl;
    //std::cout<<"** DEPTH NOT DETERMINED FOR NETWORK WITH LOOP"<<std::endl;
    return 10;
  }

  //Base Case
  if ((this->type)==SENSOR)
    return d;
  //Recursion
  else {

    for(curlink=innodes.begin();curlink!=innodes.end();++curlink) {
      cur_depth=((*curlink)->in_node)->depth(d+1,mynet);
      if (cur_depth>max) max=cur_depth;
    }
  
    return max;

  } //end else

}
