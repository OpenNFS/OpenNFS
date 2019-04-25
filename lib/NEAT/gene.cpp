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
#include "gene.h"

#include <iostream>
#include <sstream>
using namespace NEAT;

Gene::Gene(double w, NNode *inode, NNode *onode, bool recur, double innov, double mnum) {
	lnk = new Link(w, inode, onode, recur);
	innovation_num = innov;
	mutation_num = mnum;

	enable = true;

	frozen = false;
}


Gene::Gene(Trait *tp,double w,NNode *inode,NNode *onode,bool recur,double innov,double mnum) {
	lnk=new Link(tp,w,inode,onode,recur);
	innovation_num=innov;
	mutation_num=mnum;

	enable=true;

	frozen=false;
}

Gene::Gene(Gene *g,Trait *tp,NNode *inode,NNode *onode) {
	//cout<<"Trying to attach nodes: "<<inode<<" "<<onode<<endl;
	lnk=new Link(tp,(g->lnk)->weight,inode,onode,(g->lnk)->is_recurrent);
	innovation_num=g->innovation_num;
	mutation_num=g->mutation_num;
	enable=g->enable;

	frozen=g->frozen;
}

Gene::Gene(const char *argline, std::vector<Trait*> &traits, std::vector<NNode*> &nodes) {
	//Gene parameter holders
	int traitnum;
	int inodenum;
	int onodenum;
	NNode *inode;
	NNode *onode;
	double weight;
	int recur;
	Trait *traitptr;

	std::vector<Trait*>::iterator curtrait;
	std::vector<NNode*>::iterator curnode;

	//Get the gene parameters

    std::stringstream ss(argline);

	//char curword[128];
	//char delimiters[] = " \n";
	//int curwordnum = 0;

	//strcpy(curword, NEAT::getUnit(argline, curwordnum++, delimiters));
	//traitnum = atoi(curword);
	//strcpy(curword, NEAT::getUnit(argline, curwordnum++, delimiters));
	//inodenum = atoi(curword);
	//strcpy(curword, NEAT::getUnit(argline, curwordnum++, delimiters));
	//onodenum = atoi(curword);
	//strcpy(curword, NEAT::getUnit(argline, curwordnum++, delimiters));
	//weight = atof(curword);
	//strcpy(curword, NEAT::getUnit(argline, curwordnum++, delimiters));
	//recur = atoi(curword);
	//strcpy(curword, NEAT::getUnit(argline, curwordnum++, delimiters));
	//innovation_num = atof(curword);
	//strcpy(curword, NEAT::getUnit(argline, curwordnum++, delimiters));
	//mutation_num = atof(curword);
	//strcpy(curword, NEAT::getUnit(argline, curwordnum++, delimiters));
	//enable = (bool)(atoi(curword));

    ss >> traitnum >> inodenum >> onodenum >> weight >> recur >> innovation_num >> mutation_num >> enable;
    //std::cout << traitnum << " " << inodenum << " " << onodenum << " ";
    //std::cout << weight << " " << recur << " " << innovation_num << " ";
    //std::cout << mutation_num << " " << enable << std::endl;

	frozen=false; //TODO: MAYBE CHANGE

	//Get a pointer to the linktrait
	if (traitnum==0) traitptr=0;
	else {
		curtrait=traits.begin();
		while(((*curtrait)->trait_id)!=traitnum)
			++curtrait;
		traitptr=(*curtrait);
	}

	//Get a pointer to the input node
	curnode=nodes.begin();
	while(((*curnode)->node_id)!=inodenum)
		++curnode;
	inode=(*curnode);

	//Get a pointer to the output node
	curnode=nodes.begin();
	while(((*curnode)->node_id)!=onodenum)
		++curnode;
	onode=(*curnode);

	lnk=new Link(traitptr,weight,inode,onode,recur);
}

Gene::Gene(const Gene& gene)
{
	innovation_num = gene.innovation_num;
	mutation_num = gene.mutation_num;
	enable = gene.enable;
	frozen = gene.frozen;

	lnk = new Link(*gene.lnk);
}

Gene::~Gene() {
	delete lnk;
}


void Gene::print_to_file(std::ofstream &outFile) {
  outFile<<"gene ";
  //Start off with the trait number for this gene
  if ((lnk->linktrait)==0) outFile<<"0 ";
  else outFile<<((lnk->linktrait)->trait_id)<<" ";
  outFile<<(lnk->in_node)->node_id<<" ";
  outFile<<(lnk->out_node)->node_id<<" ";
  outFile<<(lnk->weight)<<" ";
  outFile<<(lnk->is_recurrent)<<" ";
  outFile<<innovation_num<<" ";
  outFile<<mutation_num<<" ";
  outFile<<enable<<std::endl;
}


void Gene::print_to_file(std::ostream &outFile) {
	outFile<<"gene ";
	//outFile.write(5, "gene ");

	//Start off with the trait number for this gene
	if ((lnk->linktrait)==0) {
		outFile<<"0 ";
		//outFile.write(2, "0 ");
	}
	else {
		outFile<<((lnk->linktrait)->trait_id)<<" ";
		//char tempbuf2[128];
		//sprintf(tempbuf2, sizeof(tempbuf2),"%d ", (lnk->linktrait)->trait_id);
		//outFile.write(strlen(tempbuf2),tempbuf2);
	}
	//char tempbuf[1024];
	//sprintf(tempbuf,sizeof(tempbuf),"%d %d %f %d %f %f %d\n", (lnk->in_node)->node_id,
	//	(lnk->out_node)->node_id, lnk->weight, lnk->is_recurrent, innovation_num, mutation_num, enable);
	//outFile.write(strlen(tempbuf),tempbuf);
	outFile<<(lnk->in_node)->node_id<<" ";
	outFile<<(lnk->out_node)->node_id<<" ";
	outFile<<(lnk->weight)<<" ";
	outFile<<(lnk->is_recurrent)<<" ";
	outFile<<innovation_num<<" ";
	outFile<<mutation_num<<" ";
    outFile<<enable<<std::endl;
}
