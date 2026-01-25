#pragma once

#include <forward_list>
#include <map>
#include <memory>
#include <vector>

#include "AABB.h"
#include "Frustum.h"
#include "IAABB.h"

#define AABB_NULL_NODE 0xffffffff

struct AABBNode {
    AABB aabb;
    std::shared_ptr<IAABB> object;
    // tree links
    unsigned parentNodeIndex;
    unsigned leftNodeIndex;
    unsigned rightNodeIndex;
    // node linked list link
    unsigned nextNodeIndex;

    [[nodiscard]] bool isLeaf() const {
        return leftNodeIndex == AABB_NULL_NODE;
    }

    AABBNode()
        : object(nullptr), parentNodeIndex(AABB_NULL_NODE), leftNodeIndex(AABB_NULL_NODE), rightNodeIndex(AABB_NULL_NODE),
          nextNodeIndex(AABB_NULL_NODE) {
    }
};

class AABBTree {
    std::map<std::shared_ptr<IAABB>, unsigned> _objectNodeIndexMap;
    std::vector<AABBNode> _nodes;
    unsigned _rootNodeIndex;
    unsigned _allocatedNodeCount;
    unsigned _nextFreeNodeIndex;
    unsigned _nodeCapacity;
    unsigned _growthSize;

    unsigned allocateNode();
    void deallocateNode(unsigned nodeIndex);
    void insertLeaf(unsigned leafNodeIndex);
    void removeLeaf(unsigned leafNodeIndex);
    void updateLeaf(unsigned leafNodeIndex, const AABB &newAaab);
    void fixUpwardsTree(unsigned treeNodeIndex);

  public:
    explicit AABBTree(unsigned initialSize);
    ~AABBTree();

    void insertObject(std::shared_ptr<IAABB> const &object);
    void removeObject(std::shared_ptr<IAABB> const &object);
    void updateObject(std::shared_ptr<IAABB> const &object);
    [[nodiscard]] std::forward_list<std::shared_ptr<IAABB>> queryOverlaps(Frustum const &frustum) const;
    [[nodiscard]] std::forward_list<std::shared_ptr<IAABB>> queryOverlaps(AABB const &bounds) const;
};
