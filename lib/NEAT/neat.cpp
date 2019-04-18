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
#include "neat.h"

#include <fstream>
#include <cmath>
#include <cstring>

double NEAT::trait_param_mut_prob = 0;
double NEAT::trait_mutation_power = 0; // Power of mutation on a signle trait param 
double NEAT::linktrait_mut_sig = 0; // Amount that mutation_num changes for a trait change inside a link
double NEAT::nodetrait_mut_sig = 0; // Amount a mutation_num changes on a link connecting a node that changed its trait 
double NEAT::weight_mut_power = 0; // The power of a linkweight mutation 
double NEAT::recur_prob = 0; // Prob. that a link mutation which doesn't have to be recurrent will be made recurrent 
double NEAT::disjoint_coeff = 0;
double NEAT::excess_coeff = 0;
double NEAT::mutdiff_coeff = 0;
double NEAT::compat_threshold = 0;
double NEAT::age_significance = 0; // How much does age matter? 
double NEAT::survival_thresh = 0; // Percent of ave fitness for survival 
double NEAT::mutate_only_prob = 0; // Prob. of a non-mating reproduction 
double NEAT::mutate_random_trait_prob = 0;
double NEAT::mutate_link_trait_prob = 0;
double NEAT::mutate_node_trait_prob = 0;
double NEAT::mutate_link_weights_prob = 0;
double NEAT::mutate_toggle_enable_prob = 0;
double NEAT::mutate_gene_reenable_prob = 0;
double NEAT::mutate_add_node_prob = 0;
double NEAT::mutate_add_link_prob = 0;
double NEAT::interspecies_mate_rate = 0; // Prob. of a mate being outside species 
double NEAT::mate_multipoint_prob = 0;     
double NEAT::mate_multipoint_avg_prob = 0;
double NEAT::mate_singlepoint_prob = 0;
double NEAT::mate_only_prob = 0; // Prob. of mating without mutation 
double NEAT::recur_only_prob = 0;  // Probability of forcing selection of ONLY links that are naturally recurrent 
int NEAT::pop_size = 0;  // Size of population 
int NEAT::dropoff_age = 0;  // Age where Species starts to be penalized 
int NEAT::newlink_tries = 0;  // Number of tries mutate_add_link will attempt to find an open link 
int NEAT::print_every = 0; // Tells to print population to file every n generations 
int NEAT::babies_stolen = 0; // The number of babies to siphen off to the champions 
int NEAT::num_runs = 0;
//MRandomR250 NEAT::NEATRandGen = MRandomR250(Platform::getRealMilliseconds()); // Random number generator; can pass seed value as argument here
//MRandomR250 NEAT::NEATRandGen = MRandomR250();

//const char* NEAT::getUnit(const char *string, int index, const char *set)
//{
//	int sz;
//	while(index--)
//	{
//		if(!*string)
//			return "";
//		sz = strcspn(string, set);
//		if (string[sz] == 0)
//			return "";
//		string += (sz + 1);    
//	}
//	sz = strcspn(string, set);
//	if (sz == 0)
//		return "";
//	char *ret = getReturnBuffer(sz+1);
//	strncpy(ret, string, sz);
//	ret[sz] = '\0';
//	return ret;
//}
//
//const char* NEAT::getUnits(const char *string, int startIndex, int endIndex, const char *set)
//{
//	int sz;
//	int index = startIndex;
//	while(index--)
//	{
//		if(!*string)
//			return "";
//		sz = strcspn(string, set);
//		if (string[sz] == 0)
//			return "";
//		string += (sz + 1);    
//	}
//	const char *startString = string;
//	while(startIndex <= endIndex--)
//	{
//		sz = strcspn(string, set);
//		string += sz;
//		if (*string == 0)
//			break;
//		string++;
//	}
//	if(!*string)
//		string++;
//	int totalSize = (int(string - startString));
//	char *ret = getReturnBuffer(totalSize);
//	strncpy(ret, startString, totalSize - 1);
//	ret[totalSize-1] = '\0';
//	return ret;
//}
//
int NEAT::getUnitCount(const char *string, const char *set)
{
	int count = 0;
	short last = 0;
	while(*string)
	{
		last = *string++;

		for(int i =0; set[i]; i++)
		{
			if(last == set[i])
			{
				count++;
				last = 0;
				break;
			}   
		}
	}
	if(last)
		count++;
	return count;
}   

bool NEAT::load_neat_params(const char *filename, bool output) {

    std::ifstream paramFile(filename);

	if(!paramFile) {
		return false;
	}
	char curword[128];
	//char delimiters[] = " \n"; // tab = bad, CR(int 13) = bad in the file
	//char delimiters[] = " \t\n";
	//char delimiters[] = {' ', '\n', (char)13};
	//int curwordnum = 1;
	//char filestring[1000000];
	//paramFile.read(sizeof(filestring), filestring);

	// **********LOAD IN PARAMETERS*************** //
    if(output)
	    printf("NEAT READING IN %s", filename);

	paramFile>>curword;
	paramFile>>NEAT::trait_param_mut_prob;

	//strcpy(curword, getUnit(filestring, curwordnum, delimiters));
	//NEAT::trait_param_mut_prob = atof(curword);
	//curwordnum += 2;

	paramFile>>curword;
	paramFile>>NEAT::trait_mutation_power;

	//strcpy(curword, getUnit(filestring, curwordnum, delimiters));
	//NEAT::trait_mutation_power = atof(curword);
	//curwordnum += 2;

	paramFile>>curword;
	paramFile>>NEAT::linktrait_mut_sig;

	//strcpy(curword, getUnit(filestring, curwordnum, delimiters));
	//NEAT::linktrait_mut_sig = atof(curword);
	//curwordnum += 2;

	paramFile>>curword;
	paramFile>>NEAT::nodetrait_mut_sig;
	
    //strcpy(curword, getUnit(filestring, curwordnum, delimiters));
	//NEAT::nodetrait_mut_sig = atof(curword);
	//curwordnum += 2;
	
    paramFile>>curword;
	paramFile>>NEAT::weight_mut_power;
	
    //strcpy(curword, getUnit(filestring, curwordnum, delimiters));
	//NEAT::weight_mut_power = atof(curword);
	//curwordnum += 2;
	
    paramFile>>curword;
	paramFile>>NEAT::recur_prob;
	
    //strcpy(curword, getUnit(filestring, curwordnum, delimiters));
	//NEAT::recur_prob = atof(curword);
	//curwordnum += 2;
	
    paramFile>>curword;
	paramFile>>NEAT::disjoint_coeff;
	
    //strcpy(curword, getUnit(filestring, curwordnum, delimiters));
	//NEAT::disjoint_coeff = atof(curword);
	//curwordnum += 2;
	
    paramFile>>curword;
	paramFile>>NEAT::excess_coeff;
	
    //strcpy(curword, getUnit(filestring, curwordnum, delimiters));
	//NEAT::excess_coeff = atof(curword);
	//curwordnum += 2;
	
    paramFile>>curword;
	paramFile>>NEAT::mutdiff_coeff;
	
    //strcpy(curword, getUnit(filestring, curwordnum, delimiters));
	//NEAT::mutdiff_coeff = atof(curword);
	//curwordnum += 2;
	
    paramFile>>curword;
	paramFile>>NEAT::compat_threshold;
	
    //strcpy(curword, getUnit(filestring, curwordnum, delimiters));
	//NEAT::compat_threshold = atof(curword);
	//curwordnum += 2;
	
    paramFile>>curword;
	paramFile>>NEAT::age_significance;
	
    //strcpy(curword, getUnit(filestring, curwordnum, delimiters));
	//NEAT::age_significance = atof(curword);
	//curwordnum += 2;
	
    paramFile>>curword;
	paramFile>>NEAT::survival_thresh;
	
    //strcpy(curword, getUnit(filestring, curwordnum, delimiters));
	//NEAT::survival_thresh = atof(curword);
	//curwordnum += 2;
	
    paramFile>>curword;
	paramFile>>NEAT::mutate_only_prob;
	
    //strcpy(curword, getUnit(filestring, curwordnum, delimiters));
	//NEAT::mutate_only_prob = atof(curword);
	//curwordnum += 2;
	
    paramFile>>curword;
	paramFile>>NEAT::mutate_random_trait_prob;
	
    //strcpy(curword, getUnit(filestring, curwordnum, delimiters));
	//NEAT::mutate_random_trait_prob = atof(curword);
	//curwordnum += 2;
	
    paramFile>>curword;
	paramFile>>NEAT::mutate_link_trait_prob;
	
    //strcpy(curword, getUnit(filestring, curwordnum, delimiters));
	//NEAT::mutate_link_trait_prob = atof(curword);
	//curwordnum += 2;
	
    paramFile>>curword;
	paramFile>>NEAT::mutate_node_trait_prob;
	
    //strcpy(curword, getUnit(filestring, curwordnum, delimiters));
	//NEAT::mutate_node_trait_prob = atof(curword);
	//curwordnum += 2;
	
    paramFile>>curword;
	paramFile>>NEAT::mutate_link_weights_prob;
	
    //strcpy(curword, getUnit(filestring, curwordnum, delimiters));
	//NEAT::mutate_link_weights_prob = atof(curword);
	//curwordnum += 2;
	
    paramFile>>curword;
	paramFile>>NEAT::mutate_toggle_enable_prob;
	
    //strcpy(curword, getUnit(filestring, curwordnum, delimiters));
	//NEAT::mutate_toggle_enable_prob = atof(curword);
	//curwordnum += 2;

	paramFile>>curword;
	paramFile>>NEAT::mutate_gene_reenable_prob;
	
    //strcpy(curword, getUnit(filestring, curwordnum, delimiters));
	//NEAT::mutate_gene_reenable_prob = atof(curword);
	//curwordnum += 2;
	
    paramFile>>curword;
	paramFile>>NEAT::mutate_add_node_prob;
	
    //strcpy(curword, getUnit(filestring, curwordnum, delimiters));
	//NEAT::mutate_add_node_prob = atof(curword);
	//curwordnum += 2;
	
    paramFile>>curword;
	paramFile>>NEAT::mutate_add_link_prob;
	
    //strcpy(curword, getUnit(filestring, curwordnum, delimiters));
	//NEAT::mutate_add_link_prob = atof(curword);
	//curwordnum += 2;
	
    paramFile>>curword;
	paramFile>>NEAT::interspecies_mate_rate;
	
    //strcpy(curword, getUnit(filestring, curwordnum, delimiters));
	//NEAT::interspecies_mate_rate = atof(curword);
	//curwordnum += 2;
	
    paramFile>>curword;
	paramFile>>NEAT::mate_multipoint_prob;
	
    //strcpy(curword, getUnit(filestring, curwordnum, delimiters));
	//NEAT::mate_multipoint_prob = atof(curword);
	//curwordnum += 2;
	
    paramFile>>curword;
	paramFile>>NEAT::mate_multipoint_avg_prob;
	
    //strcpy(curword, getUnit(filestring, curwordnum, delimiters));
	//NEAT::mate_multipoint_avg_prob = atof(curword);
	//curwordnum += 2;
	
    paramFile>>curword;
	paramFile>>NEAT::mate_singlepoint_prob;
	
    //strcpy(curword, getUnit(filestring, curwordnum, delimiters));
	//NEAT::mate_singlepoint_prob = atof(curword);
	//curwordnum += 2;
	
    paramFile>>curword;
	paramFile>>NEAT::mate_only_prob;
	
    //strcpy(curword, getUnit(filestring, curwordnum, delimiters));
	//NEAT::mate_only_prob = atof(curword);
	//curwordnum += 2;
	
    paramFile>>curword;
	paramFile>>NEAT::recur_only_prob;
	
    //strcpy(curword, getUnit(filestring, curwordnum, delimiters));
	//NEAT::recur_only_prob = atof(curword);
	//curwordnum += 2;
	
    paramFile>>curword;
	paramFile>>NEAT::pop_size;
	
    //strcpy(curword, getUnit(filestring, curwordnum, delimiters));
	//NEAT::pop_size = atoi(curword);
	//curwordnum += 2;
	
    paramFile>>curword;
	paramFile>>NEAT::dropoff_age;
	
    //strcpy(curword, getUnit(filestring, curwordnum, delimiters));
	//NEAT::dropoff_age = atoi(curword);
	//curwordnum += 2;
	
    paramFile>>curword;
	paramFile>>NEAT::newlink_tries;
	
    //strcpy(curword, getUnit(filestring, curwordnum, delimiters));
	//NEAT::newlink_tries = atoi(curword);
	//curwordnum += 2;
	
    paramFile>>curword;
	paramFile>>NEAT::print_every;
	
    //strcpy(curword, getUnit(filestring, curwordnum, delimiters));
	//NEAT::print_every = atoi(curword);
	//curwordnum += 2;
	
    paramFile>>curword;
	paramFile>>NEAT::babies_stolen;
	
    //strcpy(curword, getUnit(filestring, curwordnum, delimiters));
	//NEAT::babies_stolen = atoi(curword);
	//curwordnum += 2;

    paramFile>>curword;
	paramFile>>NEAT::num_runs;
	

    if(output) {
	    printf("trait_param_mut_prob=%f\n",trait_param_mut_prob);
	    printf("trait_mutation_power=%f\n",trait_mutation_power);
	    printf("linktrait_mut_sig=%f\n",linktrait_mut_sig);
	    printf("nodetrait_mut_sig=%f\n",nodetrait_mut_sig);
	    printf("weight_mut_power=%f\n",weight_mut_power);
	    printf("recur_prob=%f\n",recur_prob);
	    printf("disjoint_coeff=%f\n",disjoint_coeff);
	    printf("excess_coeff=%f\n",excess_coeff);
	    printf("mutdiff_coeff=%f\n",mutdiff_coeff);
	    printf("compat_threshold=%f\n",compat_threshold);
	    printf("age_significance=%f\n",age_significance);
	    printf("survival_thresh=%f\n",survival_thresh);
	    printf("mutate_only_prob=%f\n",mutate_only_prob);
	    printf("mutate_random_trait_prob=%f\n",mutate_random_trait_prob);
	    printf("mutate_link_trait_prob=%f\n",mutate_link_trait_prob);
	    printf("mutate_node_trait_prob=%f\n",mutate_node_trait_prob);
	    printf("mutate_link_weights_prob=%f\n",mutate_link_weights_prob);
	    printf("mutate_toggle_enable_prob=%f\n",mutate_toggle_enable_prob);
	    printf("mutate_gene_reenable_prob=%f\n",mutate_gene_reenable_prob);
	    printf("mutate_add_node_prob=%f\n",mutate_add_node_prob);
	    printf("mutate_add_link_prob=%f\n",mutate_add_link_prob);
	    printf("interspecies_mate_rate=%f\n",interspecies_mate_rate);
	    printf("mate_multipoint_prob=%f\n",mate_multipoint_prob);
	    printf("mate_multipoint_avg_prob=%f\n",mate_multipoint_avg_prob);
	    printf("mate_singlepoint_prob=%f\n",mate_singlepoint_prob);
	    printf("mate_only_prob=%f\n",mate_only_prob);
	    printf("recur_only_prob=%f\n",recur_only_prob);
	    printf("pop_size=%d\n",pop_size);
	    printf("dropoff_age=%d\n",dropoff_age);
	    printf("newlink_tries=%d\n",newlink_tries);
	    printf("print_every=%d\n",print_every);
	    printf("babies_stolen=%d\n",babies_stolen);
	    printf("num_runs=%d\n",num_runs);
    }

	paramFile.close();
	return true;
}

/* Inline Functions in Header file
int NEAT::randposneg() {
	if (NEAT::NEATRandGen.randI()%2) 
		return 1; 
	else 
		return -1;
}

int NEAT::randint(int x,int y) {
	return NEAT::NEATRandGen.randI()%(y-x+1)+x;
}

double NEAT::randfloat() {
	return NEAT::NEATRandGen.randF();
}
*/

double NEAT::gaussrand() {
	static int iset=0;
	static double gset;
	double fac,rsq,v1,v2;

	if (iset==0) {
		do {
			v1=2.0*(randfloat())-1.0;
			v2=2.0*(randfloat())-1.0;
			rsq=v1*v1+v2*v2;
		} while (rsq>=1.0 || rsq==0.0);
		fac=sqrt(-2.0*log(rsq)/rsq);
		gset=v1*fac;
		iset=1;
		return v2*fac;
	}
	else {
		iset=0;
		return gset;
	}
}

double NEAT::fsigmoid(double activesum,double slope,double constant) {
	//RIGHT SHIFTED ---------------------------------------------------------
	//return (1/(1+(exp(-(slope*activesum-constant))))); //ave 3213 clean on 40 runs of p2m and 3468 on another 40 
	//41394 with 1 failure on 8 runs

	//LEFT SHIFTED ----------------------------------------------------------
	//return (1/(1+(exp(-(slope*activesum+constant))))); //original setting ave 3423 on 40 runs of p2m, 3729 and 1 failure also

	//PLAIN SIGMOID ---------------------------------------------------------
	//return (1/(1+(exp(-activesum)))); //3511 and 1 failure

	//LEFT SHIFTED NON-STEEPENED---------------------------------------------
	//return (1/(1+(exp(-activesum-constant)))); //simple left shifted

	//NON-SHIFTED STEEPENED
	return (1/(1+(exp(-(slope*activesum))))); //Compressed
}

double NEAT::oldhebbian(double weight, double maxweight, double active_in, double active_out, double hebb_rate, double pre_rate, double post_rate) {

	bool neg=false;
	double delta;

	//double weight_mag;

	if (maxweight<5.0) maxweight=5.0;

	if (weight>maxweight) weight=maxweight;

	if (weight<-maxweight) weight=-maxweight;

	if (weight<0) {
		neg=true;
		weight=-weight;
	}

	//if (weight<0) {
	//  weight_mag=-weight;
	//}
	//else weight_mag=weight;

	if (!(neg)) {
		//if (true) {
		delta=
			hebb_rate*(maxweight-weight)*active_in*active_out+
			pre_rate*(weight)*active_in*(active_out-1.0)+
			post_rate*(weight)*(active_in-1.0)*active_out;

		//delta=delta-hebb_rate/2; //decay

		//delta=delta+randposneg()*randfloat()*0.01; //noise

		//cout<<"delta: "<<delta<<endl;

		if (weight+delta>0)
			return weight+delta;
		//else return 0.01;

		//return weight+delta;

	}
	else {
		//In the inhibatory case, we strengthen the synapse when output is low and
		//input is high
		delta=
			hebb_rate*(maxweight-weight)*active_in*(1.0-active_out)+ //"unhebb"
			//hebb_rate*(maxweight-weight)*(1.0-active_in)*(active_out)+
			-5*hebb_rate*(weight)*active_in*active_out+ //anti-hebbian
			//hebb_rate*(maxweight-weight)*active_in*active_out+
			//pre_rate*weight*active_in*(active_out-1.0)+
			//post_rate*weight*(active_in-1.0)*active_out;
			0;

		//delta=delta-hebb_rate; //decay

		//delta=delta+randposneg()*randfloat()*0.01; //noise

		if (-(weight+delta)<0)
			return -(weight+delta);
		else return -0.01;

		return -(weight+delta);

	}

	return 0;

}

double NEAT::hebbian(double weight, double maxweight, double active_in, double active_out, double hebb_rate, double pre_rate, double post_rate) {

	bool neg=false;
	double delta;

	//double weight_mag;

	double topweight;

	if (maxweight<5.0) maxweight=5.0;

	if (weight>maxweight) weight=maxweight;

	if (weight<-maxweight) weight=-maxweight;

	if (weight<0) {
		neg=true;
		weight=-weight;
	}


	//if (weight<0) {
	//  weight_mag=-weight;
	//}
	//else weight_mag=weight;


	topweight=weight+2.0;
	if (topweight>maxweight) topweight=maxweight;

	if (!(neg)) {
		//if (true) {
		delta=
			hebb_rate*(maxweight-weight)*active_in*active_out+
			pre_rate*(topweight)*active_in*(active_out-1.0);
		//post_rate*(weight+1.0)*(active_in-1.0)*active_out;

		//delta=delta-hebb_rate/2; //decay

		//delta=delta+randposneg()*randfloat()*0.01; //noise

		//cout<<"delta: "<<delta<<endl;

		//if (weight+delta>0)
		//  return weight+delta;
		//else return 0.01;

		return weight+delta;

	}
	else {
		//In the inhibatory case, we strengthen the synapse when output is low and
		//input is high
		delta=
			pre_rate*(maxweight-weight)*active_in*(1.0-active_out)+ //"unhebb"
			//hebb_rate*(maxweight-weight)*(1.0-active_in)*(active_out)+
			-hebb_rate*(topweight+2.0)*active_in*active_out+ //anti-hebbian
			//hebb_rate*(maxweight-weight)*active_in*active_out+
			//pre_rate*weight*active_in*(active_out-1.0)+
			//post_rate*weight*(active_in-1.0)*active_out;
			0;

		//delta=delta-hebb_rate; //decay

		//delta=delta+randposneg()*randfloat()*0.01; //noise

		//if (-(weight+delta)<0)
		//  return -(weight+delta);
		//  else return -0.01;

		return -(weight+delta);

	}

}



