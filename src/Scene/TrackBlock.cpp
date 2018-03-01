//
// Created by Amrik on 26/02/2018.
//

#include "TrackBlock.h"

TrackBlock::TrackBlock(int blockID, struct TRKBLOCK &nfs_track_block) {
    block_id = blockID;
    trk = nfs_track_block;
}
