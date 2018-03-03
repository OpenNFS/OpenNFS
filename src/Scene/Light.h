//
// Created by Amrik on 01/03/2018.
//

#ifndef OPENNFS3_LIGHT_H
#define OPENNFS3_LIGHT_H

#include <glm/vec3.hpp>
#include "../nfs_data.h"

class Light {

public:
    Light(INTPT light_position, long light_type);
    Light(glm::vec3 light_position, long light_type);

    glm::vec3 position{};
    glm::vec3 colour;
    long type;
    /* Iterators to allow for ranged for loops with class*/
    class iterator {
    public:
        explicit iterator(Light *ptr) : ptr(ptr) { }

        iterator operator++() {
            ++ptr;
            return *this;
        }

        bool operator!=(const iterator &other) { return ptr != other.ptr; }

        const Light &operator*() const { return *ptr; }

    private:
        Light *ptr;
    };
    unsigned len;
    iterator begin() const { return iterator(val); }
    iterator end() const { return iterator(val + len); }
    Light *val;
};


#endif //OPENNFS3_LIGHT_H
