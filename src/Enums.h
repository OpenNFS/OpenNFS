//
// Created by Amrik Sadhra on 02/08/2018.
//

#pragma once

#include <boost/preprocessor.hpp>

#define X_DEFINE_ENUM_WITH_STRING_CONVERSIONS_TOSTRING_CASE(r, data, elem)    \
    case elem : return BOOST_PP_STRINGIZE(elem);

#define DEFINE_ENUM_WITH_STRING_CONVERSIONS(name, enumerators)                \
    enum name {                                                               \
        BOOST_PP_SEQ_ENUM(enumerators)                                        \
    };                                                                        \
                                                                              \
    inline const char* ToString(name v)                                       \
    {                                                                         \
        switch (v)                                                            \
        {                                                                     \
            BOOST_PP_SEQ_FOR_EACH(                                            \
                X_DEFINE_ENUM_WITH_STRING_CONVERSIONS_TOSTRING_CASE,          \
                name,                                                         \
                enumerators                                                   \
            )                                                                 \
            default: return "[Unknown " BOOST_PP_STRINGIZE(name) "]";         \
        }                                                                     \
    }

DEFINE_ENUM_WITH_STRING_CONVERSIONS(NFSVer, (UNKNOWN)(NFS_1)(NFS_2)(NFS_2_PS1)(NFS_2_SE)(NFS_3)(NFS_3_PS1)(NFS_4)(NFS_5));
DEFINE_ENUM_WITH_STRING_CONVERSIONS(EntityType, (XOBJ)(OBJ_POLY)(LANE)(SOUND)(LIGHT)(ROAD)(GLOBAL)(CAR))