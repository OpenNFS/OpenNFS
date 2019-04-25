***************************************************************************
Copyright 2010 The University of Texas at Austin

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

****************************************************************************

NEAT source code distribution version 1.2, 7/23/10
----------------------------------------------------

The NEAT software is based on the real-time NeuroEvolution of
Augmenting Topologies method of evolving artificial neural networks, by
Stanley and Miikkulainen (2002).

Source code included in this release is originally written by Kenneth
Stanley (kstanley@cs.ucf.edu).  The official distribution site is:

http://nn.cs.utexas.edu/keyword?neat-c

The core NEAT code is updated significantly from earlier releases.  Traits
can now be made functional,  and adaptive networks can be evolved (though
the default is  non-adaptive).

-------------------------------------
Included Files
-------------------------------------

This is a list of files included in the distribution, and also ones that
are created when it is run or made using the Makefile.

Makefile        : Makes neat on linux
CMakeLists.txt  : cmake file for cross-platform make (use
http://www.cmake.org/)
README          : This file
LICENSE         : The Apache License Version 2.0 which describes the terms
for the release of this package
experiments.cpp : Sample experiments code
experiments.h
gen_*           : A printout of a generation- produced by generational
                 NEAT
gene.cpp        : Gene class definitions
gene.h
genome.cpp      : Genome class definitions
genome.h
innovation.cpp  : Innovation class definitions
innovation.h
link.cpp        : Link class definitions
link.h
neat.cpp        : Main NEAT class
neat.h
neatmain.cpp    : Location of main, entry to the executable
neatmain.h
network.cpp     : Network class defintions
network.h
nnode.cpp       : NNode class definitions
nnode.h
organism.cpp    : Organism class definitions
organism.h
p2mpar3bare.ne  : Sample parameter file
p2nv.ne         : Parameter file for non-markov double pole balancing
p2test.ne       : Sample parameter file
params256.ne    : Sample parameter file that was used in some major 
    experiments with pop size 256
pole1startgenes : Starter genes for signle pole balancing
pole2_markov.ne : Parameter file for markovian double pole balancing
pole2startgenes : Start genes for double pole balancing
pole2startgenes1 : Start genes for markovian double pole balancing
pole2startgenes2 : Start genes for non-markovian double pole balancing
population.cpp  : Population class definitions
population.h
neat            : Main executable (must execute "make" to produce this file)
species.cpp     : Species class definitions
species.h
statout         : Stat file output after some experiments
test.ne         : Sample parameter file
trait.cpp       : Trait class definitions
trait.h
xorstartgenes   : Start genes for XOR experiment

-------------------------------------
Included Experiments
-------------------------------------

After running "make" to create the "neat" executable, NEAT can be run
from the command line as follows:

% ./neat paramfilename.ne

"paramfilename.ne" must be included so that NEAT knows what evolution
parameters you want to use.  You can use one of the supplied parameter
files (they all end in the .ne extension), or create your own.

The pole2_markov.ne parameter file was designed for markovian pole
balancing of any type (single pole, double pole, real-time).

The p2nv.ne parameter file was designed for double pole balancing without
veolicity information.  However, pole2_markov.ne also works with this
experiment, and with XOR.

When you run NEAT from the command line, you are given the option of 5
experiments:

Please choose an experiment:
1 - 1-pole balancing
2 - 2-pole balancing, velocity info provided
3 - 2-pole balancing, no velocity info provided (non-markov)
4 - XOR
Number:

At the "Number:" prompt, you can enter your choice.  The correct starter
genome will be loaded in automatically and evolution will commence.
Most experiments output generational population files ("gen_#") at
regular intervals based on the "print_every" parameter in the supplied
.ne file. For example, if print_every is 5, then gen_5, gen_10, gen_15,
etc., will be written to the directory containing NEAT.

------------------------------------------------
Conclusion
------------------------------------------------

We hope that this software will be a useful starting point for your own
explorations in NEAT. The software is provided as is, however,
we will do our best to maintain it and accommodate suggestions. If you
want to be notified of future releases of the software or have questions,
comments, bug reports or suggestions, send email to kstanley@cs.ucf.edu.
