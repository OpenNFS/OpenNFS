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
#include "innovation.h"

using namespace NEAT;

Innovation::Innovation(int nin,int nout,double num1,double num2,int newid,double oldinnov) {
	innovation_type=NEWNODE;
	node_in_id=nin;
	node_out_id=nout;
	innovation_num1=num1;
	innovation_num2=num2;
	newnode_id=newid;
	old_innov_num=oldinnov;

	//Unused parameters set to zero
	new_weight=0;
	new_traitnum=0;
	recur_flag=false;
}

Innovation::Innovation(int nin,int nout,double num1,double w,int t) {
	innovation_type=NEWLINK;
	node_in_id=nin;
	node_out_id=nout;
	innovation_num1=num1;
	new_weight=w;
	new_traitnum=t;

	//Unused parameters set to zero
	innovation_num2=0;
	newnode_id=0;
	recur_flag=false;
}

Innovation::Innovation(int nin,int nout,double num1,double w,int t,bool recur) {
	innovation_type=NEWLINK;
	node_in_id=nin;
	node_out_id=nout;
	innovation_num1=num1;
	new_weight=w;
	new_traitnum=t;

	//Unused parameters set to zero
	innovation_num2=0;
	newnode_id=0;
	recur_flag=recur;
}
