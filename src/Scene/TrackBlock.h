//
// Created by Amrik on 26/02/2018.
//

#pragma once

#include "Track.h"

class TrackBlock {
public:
    TrackBlock(int blockID, glm::vec3 center_pos);
    glm::vec3 center;
    int block_id;
    std::vector<Track> track;
    std::vector<Track> objects;

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
