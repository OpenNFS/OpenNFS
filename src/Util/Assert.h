//
// Created by Amrik on 01/03/2018.
//

#ifndef OPENNFS3_UTILS_H
#define OPENNFS3_UTILS_H

#include <cassert>
#include <cstdio>

#define ASSERT(condition,...) assert( \
    condition|| \
    (fprintf(stdout,__VA_ARGS__)&&fprintf(stdout," at %s:%d\n",__FILE__,__LINE__)) \
);

#endif //OPENNFS3_UTILS_H
