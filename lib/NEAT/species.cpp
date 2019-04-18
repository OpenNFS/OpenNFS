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
#include "species.h"
#include "organism.h"
#include <cmath>
#include <iostream>
using namespace NEAT;

Species::Species(int i) {
	id=i;
	age=1;
	ave_fitness=0.0;
	expected_offspring=0;
	novel=false;
	age_of_last_improvement=0;
	max_fitness=0;
	max_fitness_ever=0;
	obliterate=false;

	average_est=0;
}

Species::Species(int i,bool n) {
	id=i;
	age=1;
	ave_fitness=0.0;
	expected_offspring=0;
	novel=n;
	age_of_last_improvement=0;
	max_fitness=0;
	max_fitness_ever=0;
	obliterate=false;

	average_est=0;
}


Species::~Species() {

	std::vector<Organism*>::iterator curorg;

	for(curorg=organisms.begin();curorg!=organisms.end();++curorg) {
		delete (*curorg);
	}

}

bool Species::rank() {
	//organisms.qsort(order_orgs);
    std::sort(organisms.begin(), organisms.end(), order_orgs);
	return true;
}

bool Species::add_Organism(Organism *o){
	organisms.push_back(o);
	return true;
}

Organism *Species::get_champ() {
	double champ_fitness=-1.0;
	Organism *thechamp;
	std::vector<Organism*>::iterator curorg;

	for(curorg=organisms.begin();curorg!=organisms.end();++curorg) {
		//TODO: Remove DEBUG code
		//cout<<"searching for champ...looking at org "<<(*curorg)->gnome->genome_id<<" fitness: "<<(*curorg)->fitness<<endl;
		if (((*curorg)->fitness)>champ_fitness) {
			thechamp=(*curorg);
			champ_fitness=thechamp->fitness;
		}
	}

	//cout<<"returning champ #"<<thechamp->gnome->genome_id<<endl;

	return thechamp;

}

bool Species::remove_org(Organism *org) {
	std::vector<Organism*>::iterator curorg;

	curorg=organisms.begin();
	while((curorg!=organisms.end())&&
		((*curorg)!=org))
		++curorg;

	if (curorg==organisms.end()) {
		//cout<<"ALERT: Attempt to remove nonexistent Organism from Species"<<endl;
		return false;
	}
	else {
		organisms.erase(curorg);
		return true;
	}

}

Organism *Species::first() {
	return *(organisms.begin());
}
/*
bool Species::print_to_file(std::ostream &outFile) {
	std::vector<Organism*>::iterator curorg;

	//Print a comment on the Species info
	//outFile<<endl<<"/* Species #"<<id<<" : (Size "<<organisms.size()<<") (AF "<<ave_fitness<<") (Age "<<age<<")  *///"<<endl<<endl;
	//char tempbuf[1024];
	//sprintf(tempbuf, sizeof(tempbuf), "/* Species #%d : (Size %d) (AF %f) (Age %d)  */\n\n", id, organisms.size(), average_est, age);
	//sprintf(tempbuf, sizeof(tempbuf), "/* Species #%d : (Size %d) (AF %f) (Age %d)  */\n\n", id, organisms.size(), ave_fitness, age);
	//outFile.write(strlen(tempbuf), tempbuf);

	//Show user what's going on
	//cout<<endl<<"/* Species #"<<id<<" : (Size "<<organisms.size()<<") (AF "<<ave_fitness<<") (Age "<<age<<")  */"<<endl;

	//Print all the Organisms' Genomes to the outFile
	//for(curorg=organisms.begin();curorg!=organisms.end();++curorg) {

		//Put the fitness for each organism in a comment
		//outFile<<endl<<"/* Organism #"<<((*curorg)->gnome)->genome_id<<" Fitness: "<<(*curorg)->fitness<<" Error: "<<(*curorg)->error<<" */"<<endl;

	//	char tempbuf2[1024];
	//	sprintf(tempbuf2, sizeof(tempbuf2), "/* Organism #%d Fitness: %f Error: %f */\n", ((*curorg)->gnome)->genome_id, (*curorg)->fitness, (*curorg)->error);
	//	outFile.write(strlen(tempbuf2), tempbuf2);

		//If it is a winner, mark it in a comment
	//	if ((*curorg)->winner) {
	//		char tempbuf3[1024];
	//		sprintf(tempbuf3, sizeof(tempbuf3), "/* ##------$ WINNER %d SPECIES #%d $------## */\n", ((*curorg)->gnome)->genome_id, id);
			//outFile<<"/* ##------$ WINNER "<<((*curorg)->gnome)->genome_id<<" SPECIES #"<<id<<" $------## */"<<endl;
	//	}

	//	((*curorg)->gnome)->print_to_file(outFile);
		//We can confirm by writing the genome #'s to the screen
		//cout<<((*curorg)->gnome)->genome_id<<endl;
	//}

	//return true;

//}*/

//Print Species to a file outFile
bool Species::print_to_file(std::ofstream &outFile) {
  std::vector<Organism*>::iterator curorg;

  //Print a comment on the Species info
  outFile<<std::endl<<"/* Species #"<<id<<" : (Size "<<organisms.size()<<") (AF "<<ave_fitness<<") (Age "<<age<<")  */"<<std::endl<<std::endl;

  //Show user what's going on
  std::cout<<std::endl<<"/* Species #"<<id<<" : (Size "<<organisms.size()<<") (AF "<<ave_fitness<<") (Age "<<age<<")  */"<<std::endl;

  //Print all the Organisms' Genomes to the outFile
  for(curorg=organisms.begin();curorg!=organisms.end();++curorg) {

    //Put the fitness for each organism in a comment
    outFile<<std::endl<<"/* Organism #"<<((*curorg)->gnome)->genome_id<<" Fitness: "<<(*curorg)->fitness<<" Error: "<<(*curorg)->error<<" */"<<std::endl;

    //If it is a winner, mark it in a comment
    if ((*curorg)->winner) outFile<<"/* ##------$ WINNER "<<((*curorg)->gnome)->genome_id<<" SPECIES #"<<id<<" $------## */"<<std::endl;

    ((*curorg)->gnome)->print_to_file(outFile);
    //We can confirm by writing the genome #'s to the screen
    //std::cout<<((*curorg)->gnome)->genome_id<<std::endl;
  }

  return true;

}


bool Species::print_to_file(std::ostream &outFile) {
	std::vector<Organism*>::iterator curorg;

	//Print a comment on the Species info
	//outFile<<std::endl<<"/* Species #"<<id<<" : (Size "<<organisms.size()<<") (AF "<<ave_fitness<<") (Age "<<age<<")  */"<<std::endl<<std::endl;
	char tempbuf[1024];
	sprintf(tempbuf,"/* Species #%d : (Size %d) (AF %f) (Age %d)  */\n\n", id, organisms.size(), average_est, age);
	//sprintf(tempbuf, "/* Species #%d : (Size %d) (AF %f) (Age %d)  */\n\n", id, organisms.size(), ave_fitness, age);
	outFile << tempbuf;

	//Show user what's going on
	//std::cout<<std::endl<<"/* Species #"<<id<<" : (Size "<<organisms.size()<<") (AF "<<ave_fitness<<") (Age "<<age<<")  */"<<std::endl;

	//Print all the Organisms' Genomes to the outFile
	for(curorg=organisms.begin();curorg!=organisms.end();++curorg) {

		//Put the fitness for each organism in a comment
		//outFile<<std::endl<<"/* Organism #"<<((*curorg)->gnome)->genome_id<<" Fitness: "<<(*curorg)->fitness<<" Error: "<<(*curorg)->error<<" */"<<std::endl;
		char tempbuf2[1024];
		sprintf(tempbuf2, "/* Organism #%d Fitness: %f Time: %d */\n", ((*curorg)->gnome)->genome_id, (*curorg)->fitness, (*curorg)->time_alive);
		outFile << tempbuf2;

		//If it is a winner, mark it in a comment
		if ((*curorg)->winner) {
			char tempbuf3[1024];
			sprintf(tempbuf3, "/* ##------$ WINNER %d SPECIES #%d $------## */\n", ((*curorg)->gnome)->genome_id, id);
			//outFile<<"/* ##------$ WINNER "<<((*curorg)->gnome)->genome_id<<" SPECIES #"<<id<<" $------## */"<<std::endl;
		}

		((*curorg)->gnome)->print_to_file(outFile);
		//We can confirm by writing the genome #'s to the screen
		//std::cout<<((*curorg)->gnome)->genome_id<<std::endl;
	}
	char tempbuf4[1024];
	sprintf(tempbuf4, "\n\n");
	outFile << tempbuf4;

	return true;

}


//Prints the champions of each species to files    
//starting with directory_prefix
//The file name are as follows: [prefix]g[generation_num]cs[species_num]
//Thus, they can be indexed by generation or species
//bool Population::print_species_champs_tofiles(char *directory_prefix, int generation) {
//
//ostrstream *fnamebuf; //File for output
//std::vector<Species*>::iterator curspecies;
//Organism *champ;
//int pause;
//
//std::cout<<generation<<std::endl;
//std::cout<<"Printing species champs to file"<<std::endl;
////cin>>pause;
//
////Step through the Species and print their champs to files
//for(curspecies=species.begin();curspecies!=species.end();++curspecies) {
//
//std::cout<<"Printing species "<<(*curspecies)->id<<" champ to file"<<std::endl;
//
////cin>>pause;
//
////Get the champ of this species
//champ=(*curspecies)->get_champ();
//
////Revise the file name
//fnamebuf=new ostrstream();
//(*fnamebuf)<<directory_prefix<<"g"<<generation<<"cs"<<(*curspecies)->id<<ends;  //needs end marker
//
////Print to file using organism printing (includes comments)
//champ->print_to_file(fnamebuf->str());
//
////Reset the name
//fnamebuf->clear();
//delete fnamebuf;
//}
//return true;
//}

void Species::adjust_fitness() {
	std::vector<Organism*>::iterator curorg;

	int num_parents;
	int count;

	int age_debt; 

	//std::cout<<"Species "<<id<<" last improved "<<(age-age_of_last_improvement)<<" steps ago when it moved up to "<<max_fitness_ever<<std::endl;

	age_debt=(age-age_of_last_improvement+1)-NEAT::dropoff_age;

	if (age_debt==0) age_debt=1;

	for(curorg=organisms.begin();curorg!=organisms.end();++curorg) {

		//Remember the original fitness before it gets modified
		(*curorg)->orig_fitness=(*curorg)->fitness;

		//Make fitness decrease after a stagnation point dropoff_age
		//Added an if to keep species pristine until the dropoff point
		//obliterate is used in competitive coevolution to mark stagnation
		//by obliterating the worst species over a certain age
		if ((age_debt>=1)||obliterate) {

			//Possible graded dropoff
			//((*curorg)->fitness)=((*curorg)->fitness)*(-atan(age_debt));

			//Extreme penalty for a long period of stagnation (divide fitness by 100)
			((*curorg)->fitness)=((*curorg)->fitness)*0.01;
			//std::cout<<"OBLITERATE Species "<<id<<" of age "<<age<<std::endl;
			//std::cout<<"dropped fitness to "<<((*curorg)->fitness)<<std::endl;
		}

		//Give a fitness boost up to some young age (niching)
		//The age_significance parameter is a system parameter
		//  if it is 1, then young species get no fitness boost
		if (age<=10) ((*curorg)->fitness)=((*curorg)->fitness)*NEAT::age_significance; 

		//Do not allow negative fitness
		if (((*curorg)->fitness)<0.0) (*curorg)->fitness=0.0001; 

		//Share fitness with the species
		(*curorg)->fitness=((*curorg)->fitness)/(organisms.size());

	}

	//Sort the population and mark for death those after survival_thresh*pop_size
	//organisms.qsort(order_orgs);
	std::sort(organisms.begin(), organisms.end(), order_orgs);

	//Update age_of_last_improvement here
	if (((*(organisms.begin()))->orig_fitness)> 
	    max_fitness_ever) {
	  age_of_last_improvement=age;
	  max_fitness_ever=((*(organisms.begin()))->orig_fitness);
	}

	//Decide how many get to reproduce based on survival_thresh*pop_size
	//Adding 1.0 ensures that at least one will survive
	num_parents=(int) floor((NEAT::survival_thresh*((double) organisms.size()))+1.0);
	
	//Mark for death those who are ranked too low to be parents
	curorg=organisms.begin();
	(*curorg)->champion=true;  //Mark the champ as such
	for(count=1;count<=num_parents;count++) {
	  if (curorg!=organisms.end())
	    ++curorg;
	}
	while(curorg!=organisms.end()) {
	  (*curorg)->eliminate=true;  //Mark for elimination
	  //std::std::cout<<"marked org # "<<(*curorg)->gnome->genome_id<<" fitness = "<<(*curorg)->fitness<<std::std::endl;
	  ++curorg;
	}             

}

double Species::compute_average_fitness() {
	std::vector<Organism*>::iterator curorg;

	double total=0.0;

	//int pause; //DEBUG: Remove

	for(curorg=organisms.begin();curorg!=organisms.end();++curorg) {
		total+=(*curorg)->fitness;
		//std::cout<<"new total "<<total<<std::endl; //DEBUG: Remove
	}

	ave_fitness=total/(organisms.size());

	//DEBUG: Remove
	//std::cout<<"average of "<<(organisms.size())<<" organisms: "<<ave_fitness<<std::endl;
	//cin>>pause;

	return ave_fitness;

}

double Species::compute_max_fitness() {
	double max=0.0;
	std::vector<Organism*>::iterator curorg;

	for(curorg=organisms.begin();curorg!=organisms.end();++curorg) {
		if (((*curorg)->fitness)>max)
			max=(*curorg)->fitness;
	}

	max_fitness=max;

	return max;
}

double Species::count_offspring(double skim) {
	std::vector<Organism*>::iterator curorg;
	int e_o_intpart;  //The floor of an organism's expected offspring
	double e_o_fracpart; //Expected offspring fractional part
	double skim_intpart;  //The whole offspring in the skim

	expected_offspring=0;

	for(curorg=organisms.begin();curorg!=organisms.end();++curorg) {
		e_o_intpart=(int) floor((*curorg)->expected_offspring);
		e_o_fracpart=fmod((*curorg)->expected_offspring,1.0);

		expected_offspring+=e_o_intpart;

		//Skim off the fractional offspring
		skim+=e_o_fracpart;

		//NOTE:  Some precision is lost by computer
		//       Must be remedied later
		if (skim>1.0) {
			skim_intpart=floor(skim);
			expected_offspring+=(int) skim_intpart;
			skim-=skim_intpart;
		}
	}

	return skim;

}

bool Species::reproduce(int generation, Population *pop,std::vector<Species*> &sorted_species) {
	int count;
	std::vector<Organism*>::iterator curorg;

	int poolsize;  //The number of Organisms in the old generation

	int orgnum;  //Random variable
	int orgcount;
	Organism *mom; //Parent Organisms
	Organism *dad;
	Organism *baby;  //The new Organism

	Genome *new_genome;  //For holding baby's genes

	std::vector<Species*>::iterator curspecies;  //For adding baby
	Species *newspecies; //For babies in new Species
	Organism *comporg;  //For Species determination through comparison

	Species *randspecies;  //For mating outside the Species
	double randmult;
	int randspeciesnum;
	int spcount;  
	std::vector<Species*>::iterator cursp;

	Network *net_analogue;  //For adding link to test for recurrency
	int pause;

	bool outside;

	bool found;  //When a Species is found

	bool champ_done=false; //Flag the preservation of the champion  

	Organism *thechamp;

	int giveup; //For giving up finding a mate outside the species

	bool mut_struct_baby;
	bool mate_baby;

	//The weight mutation power is species specific depending on its age
	double mut_power=NEAT::weight_mut_power;

	//Roulette wheel variables
	double total_fitness=0.0;
	double marble;  //The marble will have a number between 0 and total_fitness
	double spin;  //0Fitness total while the wheel is spinning

	//Compute total fitness of species for a roulette wheel
	//Note: You don't get much advantage from a roulette here
	// because the size of a species is relatively small.
	// But you can use it by using the roulette code here
	//for(curorg=organisms.begin();curorg!=organisms.end();++curorg) {
	//  total_fitness+=(*curorg)->fitness;
	//}

	
	//Check for a mistake
	if ((expected_offspring>0)&&
		(organisms.size()==0)) {
			//    std::cout<<"ERROR:  ATTEMPT TO REPRODUCE OUT OF EMPTY SPECIES"<<std::endl;
			return false;
		}

		poolsize=organisms.size()-1;

		thechamp=(*(organisms.begin()));

		//Create the designated number of offspring for the Species
		//one at a time
		for (count=0;count<expected_offspring;count++) {

			mut_struct_baby=false;
			mate_baby=false;

			outside=false;

			//Debug Trap
			if (expected_offspring>NEAT::pop_size) {
				//      std::cout<<"ALERT: EXPECTED OFFSPRING = "<<expected_offspring<<std::endl;
				//      cin>>pause;
			}

			//If we have a super_champ (Population champion), finish off some special clones
			if ((thechamp->super_champ_offspring) > 0) {
				mom=thechamp;
				new_genome=(mom->gnome)->duplicate(count);

				if ((thechamp->super_champ_offspring) == 1) {

				}

				//Most superchamp offspring will have their connection weights mutated only
				//The last offspring will be an exact duplicate of this super_champ
				//Note: Superchamp offspring only occur with stolen babies!
				//      Settings used for published experiments did not use this
				if ((thechamp->super_champ_offspring) > 1) {
					if ((randfloat()<0.8)||
						(NEAT::mutate_add_link_prob==0.0)) 
						//ABOVE LINE IS FOR:
						//Make sure no links get added when the system has link adding disabled
						new_genome->mutate_link_weights(mut_power,1.0,GAUSSIAN);
					else {
						//Sometimes we add a link to a superchamp
						net_analogue=new_genome->genesis(generation);
						new_genome->mutate_add_link(pop->innovations,pop->cur_innov_num,NEAT::newlink_tries);
						delete net_analogue;
						mut_struct_baby=true;
					}
				}

				baby=new Organism(0.0,new_genome,generation);

				if ((thechamp->super_champ_offspring) == 1) {
					if (thechamp->pop_champ) {
						//std::cout<<"The new org baby's genome is "<<baby->gnome<<std::endl;
						baby->pop_champ_child=true;
						baby->high_fit=mom->orig_fitness;
					}
				}

				thechamp->super_champ_offspring--;
			}
			//If we have a Species champion, just clone it 
			else if ((!champ_done)&&
				(expected_offspring>5)) {

					mom=thechamp; //Mom is the champ

					new_genome=(mom->gnome)->duplicate(count);

					baby=new Organism(0.0,new_genome,generation);  //Baby is just like mommy

					champ_done=true;

				}
				//First, decide whether to mate or mutate
				//If there is only one organism in the pool, then always mutate
			else if ((randfloat()<NEAT::mutate_only_prob)||
				poolsize== 0) {

					//Choose the random parent

					//RANDOM PARENT CHOOSER
					orgnum=randint(0,poolsize);
					curorg=organisms.begin();
					for(orgcount=0;orgcount<orgnum;orgcount++)
						++curorg;                       



					////Roulette Wheel
					//marble=randfloat()*total_fitness;
					//curorg=organisms.begin();
					//spin=(*curorg)->fitness;
					//while(spin<marble) {
					//++curorg;

					////Keep the wheel spinning
					//spin+=(*curorg)->fitness;
					//}
					////Finished roulette
					//

					mom=(*curorg);

					new_genome=(mom->gnome)->duplicate(count);

					//Do the mutation depending on probabilities of 
					//various mutations

					if (randfloat()<NEAT::mutate_add_node_prob) {
						//std::cout<<"mutate add node"<<std::endl;
						new_genome->mutate_add_node(pop->innovations,pop->cur_node_id,pop->cur_innov_num);
						mut_struct_baby=true;
					}
					else if (randfloat()<NEAT::mutate_add_link_prob) {
						//std::cout<<"mutate add link"<<std::endl;
						net_analogue=new_genome->genesis(generation);
						new_genome->mutate_add_link(pop->innovations,pop->cur_innov_num,NEAT::newlink_tries);
						delete net_analogue;
						mut_struct_baby=true;
					}
					//NOTE:  A link CANNOT be added directly after a node was added because the phenotype
					//       will not be appropriately altered to reflect the change
					else {
						//If we didn't do a structural mutation, we do the other kinds

						if (randfloat()<NEAT::mutate_random_trait_prob) {
							//std::cout<<"mutate random trait"<<std::endl;
							new_genome->mutate_random_trait();
						}
						if (randfloat()<NEAT::mutate_link_trait_prob) {
							//std::cout<<"mutate_link_trait"<<std::endl;
							new_genome->mutate_link_trait(1);
						}
						if (randfloat()<NEAT::mutate_node_trait_prob) {
							//std::cout<<"mutate_node_trait"<<std::endl;
							new_genome->mutate_node_trait(1);
						}
						if (randfloat()<NEAT::mutate_link_weights_prob) {
							//std::cout<<"mutate_link_weights"<<std::endl;
							new_genome->mutate_link_weights(mut_power,1.0,GAUSSIAN);
						}
						if (randfloat()<NEAT::mutate_toggle_enable_prob) {
							//std::cout<<"mutate toggle enable"<<std::endl;
							new_genome->mutate_toggle_enable(1);

						}
						if (randfloat()<NEAT::mutate_gene_reenable_prob) {
							//std::cout<<"mutate gene reenable"<<std::endl;
							new_genome->mutate_gene_reenable();
						}
					}

					baby=new Organism(0.0,new_genome,generation);

				}

				//Otherwise we should mate 
			else {

				//Choose the random mom
				orgnum=randint(0,poolsize);
				curorg=organisms.begin();
				for(orgcount=0;orgcount<orgnum;orgcount++)
					++curorg;


				////Roulette Wheel
				//marble=randfloat()*total_fitness;
				//curorg=organisms.begin();
				//spin=(*curorg)->fitness;
				//while(spin<marble) {
				//++curorg;

				////Keep the wheel spinning
				//spin+=(*curorg)->fitness;
				//}
				////Finished roulette
				//

				mom=(*curorg);         

				//Choose random dad

				if ((randfloat()>NEAT::interspecies_mate_rate)) {
					//Mate within Species

					orgnum=randint(0,poolsize);
					curorg=organisms.begin();
					for(orgcount=0;orgcount<orgnum;orgcount++)
						++curorg;


					////Use a roulette wheel
					//marble=randfloat()*total_fitness;
					//curorg=organisms.begin();
					//spin=(*curorg)->fitness;
					//while(spin<marble) {
					//++curorg;
					//}

					////Keep the wheel spinning
					//spin+=(*curorg)->fitness;
					//}
					////Finished roulette
					//

					dad=(*curorg);
				}
				else {

					//Mate outside Species  
					randspecies=this;

					//Select a random species
					giveup=0;  //Give up if you cant find a different Species
					while((randspecies==this)&&(giveup<5)) {

						//This old way just chose any old species
						//randspeciesnum=randint(0,(pop->species).size()-1);

						//Choose a random species tending towards better species
						randmult=gaussrand()/4;
						if (randmult>1.0) randmult=1.0;
						//This tends to select better species
						randspeciesnum=(int) floor((randmult*(sorted_species.size()-1.0))+0.5);
						cursp=(sorted_species.begin());
						for(spcount=0;spcount<randspeciesnum;spcount++)
							++cursp;
						randspecies=(*cursp);

						++giveup;
					}

					//OLD WAY: Choose a random dad from the random species
					//Select a random dad from the random Species
					//NOTE:  It is possible that a mating could take place
					//       here between the mom and a baby from the NEW
					//       generation in some other Species
					//orgnum=randint(0,(randspecies->organisms).size()-1);
					//curorg=(randspecies->organisms).begin();
					//for(orgcount=0;orgcount<orgnum;orgcount++)
					//  ++curorg;
					//dad=(*curorg);            

					//New way: Make dad be a champ from the random species
					dad=(*((randspecies->organisms).begin()));

					outside=true;	
				}

				//Perform mating based on probabilities of differrent mating types
				if (randfloat()<NEAT::mate_multipoint_prob) { 
					new_genome=(mom->gnome)->mate_multipoint(dad->gnome,count,mom->orig_fitness,dad->orig_fitness,outside);
				}
				else if (randfloat()<(NEAT::mate_multipoint_avg_prob/(NEAT::mate_multipoint_avg_prob+NEAT::mate_singlepoint_prob))) {
					new_genome=(mom->gnome)->mate_multipoint_avg(dad->gnome,count,mom->orig_fitness,dad->orig_fitness,outside);
				}
				else {
					new_genome=(mom->gnome)->mate_singlepoint(dad->gnome,count);
				}

				mate_baby=true;

				//Determine whether to mutate the baby's Genome
				//This is done randomly or if the mom and dad are the same organism
				if ((randfloat()>NEAT::mate_only_prob)||
					((dad->gnome)->genome_id==(mom->gnome)->genome_id)||
					(((dad->gnome)->compatibility(mom->gnome))==0.0))
				{

					//Do the mutation depending on probabilities of 
					//various mutations
					if (randfloat()<NEAT::mutate_add_node_prob) {
						new_genome->mutate_add_node(pop->innovations,pop->cur_node_id,pop->cur_innov_num);
						//  std::cout<<"mutate_add_node: "<<new_genome<<std::endl;
						mut_struct_baby=true;
					}
					else if (randfloat()<NEAT::mutate_add_link_prob) {
						net_analogue=new_genome->genesis(generation);
						new_genome->mutate_add_link(pop->innovations,pop->cur_innov_num,NEAT::newlink_tries);
						delete net_analogue;
						//std::cout<<"mutate_add_link: "<<new_genome<<std::endl;
						mut_struct_baby=true;
					}
					else {
						//Only do other mutations when not doing sturctural mutations

						if (randfloat()<NEAT::mutate_random_trait_prob) {
							new_genome->mutate_random_trait();
							//std::cout<<"..mutate random trait: "<<new_genome<<std::endl;
						}
						if (randfloat()<NEAT::mutate_link_trait_prob) {
							new_genome->mutate_link_trait(1);
							//std::cout<<"..mutate link trait: "<<new_genome<<std::endl;
						}
						if (randfloat()<NEAT::mutate_node_trait_prob) {
							new_genome->mutate_node_trait(1);
							//std::cout<<"mutate_node_trait: "<<new_genome<<std::endl;
						}
						if (randfloat()<NEAT::mutate_link_weights_prob) {
							new_genome->mutate_link_weights(mut_power,1.0,GAUSSIAN);
							//std::cout<<"mutate_link_weights: "<<new_genome<<std::endl;
						}
						if (randfloat()<NEAT::mutate_toggle_enable_prob) {
							new_genome->mutate_toggle_enable(1);
							//std::cout<<"mutate_toggle_enable: "<<new_genome<<std::endl;
						}
						if (randfloat()<NEAT::mutate_gene_reenable_prob) {
							new_genome->mutate_gene_reenable(); 
							//std::cout<<"mutate_gene_reenable: "<<new_genome<<std::endl;
						}
					}

					//Create the baby
					baby=new Organism(0.0,new_genome,generation);

				}
				else {
					//Create the baby without mutating first
					baby=new Organism(0.0,new_genome,generation);
				}

			}

			//Add the baby to its proper Species
			//If it doesn't fit a Species, create a new one

			baby->mut_struct_baby=mut_struct_baby;
			baby->mate_baby=mate_baby;

			curspecies=(pop->species).begin();
			if (curspecies==(pop->species).end()){
				//Create the first species
				newspecies=new Species(++(pop->last_species),true);
				(pop->species).push_back(newspecies);
				newspecies->add_Organism(baby);  //Add the baby
				baby->species=newspecies;  //Point the baby to its species
			} 
			else {
				comporg=(*curspecies)->first();
				found=false;
				while((curspecies!=(pop->species).end())&&
					(!found)) {	
						if (comporg==0) {
							//Keep searching for a matching species
							++curspecies;
							if (curspecies!=(pop->species).end())
								comporg=(*curspecies)->first();
						}
						else if (((baby->gnome)->compatibility(comporg->gnome))<NEAT::compat_threshold) {
							//Found compatible species, so add this organism to it
							(*curspecies)->add_Organism(baby);
							baby->species=(*curspecies);  //Point organism to its species
							found=true;  //Note the search is over
						}
						else {
							//Keep searching for a matching species
							++curspecies;
							if (curspecies!=(pop->species).end()) 
								comporg=(*curspecies)->first();
						}
					}

					//If we didn't find a match, create a new species
					if (found==false) {
					  newspecies=new Species(++(pop->last_species),true);
					  //std::std::cout<<"CREATING NEW SPECIES "<<pop->last_species<<std::std::endl;
					  (pop->species).push_back(newspecies);
					  newspecies->add_Organism(baby);  //Add the baby
					  baby->species=newspecies;  //Point baby to its species
					}


			} //end else 

		}



		return true;
}

bool NEAT::order_species(Species *x, Species *y) { 
	//std::cout<<"Comparing "<<((*((x->organisms).begin()))->orig_fitness)<<" and "<<((*((y->organisms).begin()))->orig_fitness)<<": "<<(((*((x->organisms).begin()))->orig_fitness) > ((*((y->organisms).begin()))->orig_fitness))<<std::endl;
	return (((*((x->organisms).begin()))->orig_fitness) > ((*((y->organisms).begin()))->orig_fitness));
}

bool NEAT::order_new_species(Species *x, Species *y) {
	return (x->compute_max_fitness() > y->compute_max_fitness());
}


