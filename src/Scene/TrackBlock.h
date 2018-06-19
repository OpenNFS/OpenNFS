//
// Created by Amrik on 26/02/2018.
//

#ifndef OPENNFS3_TRACKBLOCK_H
#define OPENNFS3_TRACKBLOCK_H

#include "Track.h"
#include "Light.h"

class TrackBlock {
public:
    TrackBlock(int blockID, glm::vec3 center_pos);
    glm::vec3 center;
    int block_id;
    std::vector<Track> track;
    std::vector<Track> objects;
    std::vector<Light> lights;

    /* Iterators to allow for ranged for loops with class*/
    class iterator {
    public:
        explicit iterator(TrackBlock *ptr) : ptr(ptr) { }

        iterator operator++() {
            ++ptr;
            return *this;
        }

        bool operator!=(const iterator &other) { return ptr != other.ptr; }

        const TrackBlock &operator*() const { return *ptr; }

    private:
        TrackBlock *ptr;
    };

    iterator begin() const { return iterator(val); }

    iterator end() const { return iterator(val + len); }

    TrackBlock *val;
private:
    /* Iterator vars */
    unsigned len;
};

#endif //OPENNFS3_TRACKBLOCK_H
