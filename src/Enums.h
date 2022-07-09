#pragma once

#include <boost/preprocessor.hpp>

#define BIT(x) (1 << (x))

// Collision masks
enum collisionTypes {
    COL_NOTHING       = 0,      // Collide with nothing
    COL_RAY           = BIT(0), // Collide with rays
    COL_CAR           = BIT(1), // Collide with cars
    COL_TRACK         = BIT(2), // Collide with track
    COL_VROAD         = BIT(3), // Collide with VROAD edge
    COL_VROAD_CEIL    = BIT(4), // Collide with VROAD ceiling
    COL_DYNAMIC_TRACK = BIT(5)  // Collide with roadsigns
};

#define X_DEFINE_ENUM_WITH_STRING_CONVERSIONS_TOSTRING_CASE(r, data, elem) \
    case elem:                                                             \
        return BOOST_PP_STRINGIZE(elem);

#define DEFINE_ENUM_WITH_STRING_CONVERSIONS(name, enumerators)                                            \
    enum name { BOOST_PP_SEQ_ENUM(enumerators) };                                                         \
                                                                                                          \
    inline const char* ToString(name v) {                                                                 \
        switch (v) {                                                                                      \
            BOOST_PP_SEQ_FOR_EACH(X_DEFINE_ENUM_WITH_STRING_CONVERSIONS_TOSTRING_CASE, name, enumerators) \
        default:                                                                                          \
            return "[Unknown " BOOST_PP_STRINGIZE(name) "]";                                              \
        }                                                                                                 \
    }

DEFINE_ENUM_WITH_STRING_CONVERSIONS(NFSVer, (UNKNOWN) (NFS_1) (NFS_2) (NFS_2_PS1) (NFS_2_SE) (NFS_3) (NFS_3_PS1) (NFS_4) (NFS_4_PS1) (MCO) (NFS_5));
DEFINE_ENUM_WITH_STRING_CONVERSIONS(EntityType, (XOBJ) (OBJ_POLY) (LANE) (SOUND) (LIGHT) (ROAD) (GLOBAL) (CAR) (VROAD) (VROAD_CEIL))

// TODO: Use BOOST_PP to automate this
inline NFSVer getEnum(const std::string& nfsVerString) {
    if (nfsVerString == "NFS_1")
        return NFS_1;
    else if (nfsVerString == "NFS_2")
        return NFS_2;
    else if (nfsVerString == "NFS_2_PS1")
        return NFS_2_PS1;
    else if (nfsVerString == "NFS_2_SE")
        return NFS_2_SE;
    else if (nfsVerString == "NFS_3")
        return NFS_3;
    else if (nfsVerString == "NFS_3_PS1")
        return NFS_3_PS1;
    else if (nfsVerString == "NFS_4")
        return NFS_4;
    else if (nfsVerString == "NFS_4_PS1")
        return NFS_4_PS1;
    else if (nfsVerString == "MCO")
        return MCO;
    else if (nfsVerString == "NFS_5")
        return NFS_2;
    else
        return UNKNOWN;
}
