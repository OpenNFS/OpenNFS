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
#ifndef _NERO_NEAT_H_
#define _NERO_NEAT_H_

#include <cstdlib>
#include <cstring>

namespace NEAT {

	const int num_trait_params = 8;

	extern double trait_param_mut_prob;
	extern double trait_mutation_power; // Power of mutation on a signle trait param 
	extern double linktrait_mut_sig;  // Amount that mutation_num changes for a trait change inside a link
	extern double nodetrait_mut_sig; // Amount a mutation_num changes on a link connecting a node that changed its trait 
	extern double weight_mut_power;  // The power of a linkweight mutation 
	extern double recur_prob;        // Prob. that a link mutation which doesn't have to be recurrent will be made recurrent 

	// These 3 global coefficients are used to determine the formula for
	// computating the compatibility between 2 genomes.  The formula is:
	// disjoint_coeff*pdg+excess_coeff*peg+mutdiff_coeff*mdmg.
	// See the compatibility method in the Genome class for more info
	// They can be thought of as the importance of disjoint Genes,
	// excess Genes, and parametric difference between Genes of the
	// same function, respectively. 
	extern double disjoint_coeff;
	extern double excess_coeff;
	extern double mutdiff_coeff;

	// This global tells compatibility threshold under which two Genomes are considered the same species 
	extern double compat_threshold;

	// Globals involved in the epoch cycle - mating, reproduction, etc.. 
	extern double age_significance;          // How much does age matter? 
	extern double survival_thresh;           // Percent of ave fitness for survival 
	extern double mutate_only_prob;          // Prob. of a non-mating reproduction 
	extern double mutate_random_trait_prob;
	extern double mutate_link_trait_prob;
	extern double mutate_node_trait_prob;
	extern double mutate_link_weights_prob;
	extern double mutate_toggle_enable_prob;
	extern double mutate_gene_reenable_prob;
	extern double mutate_add_node_prob;
	extern double mutate_add_link_prob;
	extern double interspecies_mate_rate;    // Prob. of a mate being outside species 
	extern double mate_multipoint_prob;     
	extern double mate_multipoint_avg_prob;
	extern double mate_singlepoint_prob;
	extern double mate_only_prob;            // Prob. of mating without mutation 
	extern double recur_only_prob;  // Probability of forcing selection of ONLY links that are naturally recurrent 
	extern int pop_size;  // Size of population 
	extern int dropoff_age;  // Age where Species starts to be penalized 
	extern int newlink_tries;  // Number of tries mutate_add_link will attempt to find an open link 
	extern int print_every; // Tells to print population to file every n generations 
	extern int babies_stolen; // The number of babies to siphon off to the champions 

	extern int num_runs; //number of times to run experiment

	//extern MRandomR250 NEATRandGen; // Random number generator; can pass seed value as argument

	//const char *getUnit(const char *string, int index, const char *set);
	//const char *getUnits(const char *string, int startIndex, int endIndex, const char *set);
	int getUnitCount(const char *string, const char *set);

	// Inline Random Functions 
	extern inline int randposneg() {
        if (rand()%2) 
            return 1; 
        else 
            return -1;
    }
    
	extern inline int randint(int x,int y) {
        return rand()%(y-x+1)+x;
    }

    extern inline double randfloat() {
        return rand() / (double) RAND_MAX;        
    }


	// SIGMOID FUNCTION ********************************
	// This is a signmoidal activation function, which is an S-shaped squashing function
	// It smoothly limits the amplitude of the output of a neuron to between 0 and 1
	// It is a helper to the neural-activation function get_active_out
	// It is made inline so it can execute quickly since it is at every non-sensor 
	// node in a network.
	// NOTE:  In order to make node insertion in the middle of a link possible,
	// the signmoid can be shifted to the right and more steeply sloped:
	// slope=4.924273
	// constant= 2.4621365
	// These parameters optimize mean squared error between the old output,
	// and an output of a node inserted in the middle of a link between
	// the old output and some other node. 
	// When not right-shifted, the steepened slope is closest to a linear
	// ascent as possible between -0.5 and 0.5
	extern double fsigmoid(double,double,double);

	// Hebbian Adaptation Function
	// Based on equations in Floreano & Urzelai 2000
	// Takes the current weight, the maximum weight in the containing network,
	// the activation coming in and out of the synapse,
	// and three learning rates for hebbian, presynaptic, and postsynaptic
	// modification
	// Returns the new modified weight
	// NOTE: For an inhibatory connection, it makes sense to
	//      emphasize decorrelation on hebbian learning!
	extern double oldhebbian(double weight, double maxweight, double active_in, double active_out, double hebb_rate, double pre_rate, double post_rate);

	// Hebbian Adaptation Function
	// Based on equations in Floreano & Urzelai 2000
	// Takes the current weight, the maximum weight in the containing network,
	// the activation coming in and out of the synapse,
	// and three learning rates for hebbian, presynaptic, and postsynaptic
	// modification
	// Returns the new modified weight
	// NOTE: For an inhibatory connection, it makes sense to
	//      emphasize decorrelation on hebbian learning!	
	extern double hebbian(double weight, double maxweight, double active_in, double active_out, double hebb_rate, double pre_rate, double post_rate);

	// Returns a normally distributed deviate with 0 mean and unit variance
	// Algorithm is from Numerical Recipes in C, Second Edition
	extern double gaussrand();

	//This is an incorrect gassian distribution...but it is faster than gaussrand (maybe it's good enough?)
	//inline double gaussrand_wrong() {return (randposneg())*(sqrt(-log((rand()*1.0)/RAND_MAX)));}   

	bool load_neat_params(const char *filename, bool output = false);

} // namespace NEAT

#endif
