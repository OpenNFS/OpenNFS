#pragma once

#include "Interfaces.h"

namespace CrpLib {

    enum HEADER_ID {
        ID_CAR = 0x43617220,
        ID_TRACK = 0x5472616B,
    };

    enum ENTRY_ID {
        // parents
        ID_ARTI = 0x41727469,

        // normal use
        ID_DAMAGEZONE = 0x647A,
        ID_VERTEX = 0x7674,
        ID_UV = 0x7576,
        ID_TRANSFORM = 0x7472,
        ID_PART = 0x7072,
        ID_NORMAL = 0x6E6D,
        ID_NAME = 0x4E616D65,
        ID_BASE = 0x42617365,
        ID_CULL = 0x246E,
        ID_EFFECT = 0x6566,

        // misc
        ID_FSH = 0x7366,
        ID_MATERIAL = 0x6D74,
        ID_RMETHOD = 0x524D7468,
        ID_BPLANES = 0x426E6450,
    };

    enum RM_NAME {
        RM_EXT = 0,
        RM_EXT_ENV = 1,
        RM_EXT_ENV_NS = 2,
        RM_INT = 3,
        RM_INT_NS = 4,
        RM_WHEEL = 5,
        RM_WINDOW = 6,
    };

    enum PART_TRANS {
        PT_NORMAL = 0,
        PT_WINDOW = 1,
        PT_TEXTURE = 0x200,

        PT_WINDOW_TEXTURE = PT_WINDOW | PT_TEXTURE,
    };

    enum INDEXROW_ID {
        ID_INDEX_VERTEX = 0x4976,
        ID_INDEX_UV = 0x4975,
    };

    enum INFOROW_ID {
        ID_INFO_CULL = 0x246E,
        ID_INFO_NORMAL = 1,
        ID_INFO_UV = 2,
        ID_INFO_VERTEX = 0,
    };

    struct tPartIndex {
        short Index;
        short Id;        // INDEXROW_ID
        int Offset;
    };

    struct tPartInfo {
        int RMOffs, Offset;
        short Length;
        short Id;        // INFOROW_ID
        short Level, IndexRowRef;

        //TODO: These might *need* to be cast as INDEX_ROW
        int GetCount() {
            if ((INFOROW_ID) Id == ID_INFO_UV)
                return (Length / 8);
            else
                return (Length / 16);
        }

        void SetCount(int value) {
            if ((INFOROW_ID) Id == ID_INFO_UV)
                Length = (short) (value * 8);
            else
                Length = (short) (value * 16);
        }


        int GetOffsetIndex() {
            if ((INFOROW_ID) Id == ID_INFO_UV)
                return (Offset / 8);
            else
                return (Offset / 16);
        }

        void SetOffsetIndex(int value) {
            if ((INFOROW_ID) Id == ID_INFO_UV)
                Offset = value * 8;
            else
                Offset = value * 16;
        }
    };

    enum GLARE_TYPE {
        GT_NORMAL = 0,
        GT_BRAKE = 1,
        GT_REVERSE = 2,
        GT_SIGNAL = 3,
        GT_SPARKLE = 4,
    };

    enum GLARE_EXTRA {
        GE_NORMAL = 0,
        GE_MIRROR = 7,
        GE_TRANSFORM = 4,
        GE_LOGO = 8,
    };

    enum GLARE_INFO {
        GI_LIGHT = 1,
        GI_SIGNAL_FRONT = 5,
        GI_SIGNAL_REAR = 7,
        GI_SIGNAL_BRAKE = 0xB,
        GI_SIGNAL_BRAKE_1 = 2,
        GI_SIGNAL_BRAKE_2 = 3,
        GI_REVERSE = 8,
        GI_SPARKLE = 9,
    };

    struct tGlareInfo {
        GLARE_TYPE Type;
        GLARE_EXTRA Extra;
        GLARE_INFO Info;
        bool Headlight;
    };

#define LEVEL_MIN(x)    (short)(x<<12)
#define LEVEL_MAX(x)    (short)(LEVEL_MIN(x) | 0xFFF)

    struct tLevelMask {
        int LevType;    // 0x1
        short Min;        // LEVEL_MIN()
        short Max;        // LEVEL_MAX()
        short Level;
        short Mask;        // 0xFFFF

        tLevelMask() {
            LevType = 1;
            Min = LEVEL_MIN(0);
            Max = LEVEL_MAX(0);
            Level = 0;
            Mask = (short) 0xFFFF;
        }

        void SetLevel(int level) {
            Min = LEVEL_MIN(level);
            Max = LEVEL_MAX(level);
            Level = level;
        }

    };

#define LEVELINDEX_LEVEL(x)        ((x&0xF0)>>4)
#define LEVELINDEX_INDEX(x)        (x&0xF)
#define LEVELINDEX_CREATE(i, l)    ((i&0xF)<<4)|(l&0xF)

    enum BASEINFO_ANIMTYPE {
        BIAT_NONE = 0,
        BIAT_CABRIO = 1,
        BIAT_SPOILER = 2,
        BIAT_WIPER = 3,
        BIAT_RIGHTARM = 4,
        BIAT_LEFTARM = 5,
        BIAT_DRIVERBODY = 6,
        BIAT_ANTENNA = 7,
    };

    enum BASEINFO_LEVEL {
        BIL_EFFECT = 0,
        BIL_BODY = 1,
        BIL_WHEEL = 2,
        BIL_STEER = 4,
        BIL_DRIVER = 8,
    };

    enum BASEINFO_WHEELTYPE {
        BIWT_FRONT1 = 0,
        BIWT_FRONT2 = 1,
        BIWT_REAR1 = 2,
        BIWT_REAR2 = 3,
        BIWT_NONWHEEL = 0xFF,
    };

    enum BASEINFO_EXTRATYPE {
        BIET_TYPE1 = 0,
        BIET_TYPE2 = 0x80,
        BIET_ANTENNA = 1,
    };

    enum BASEINFO_GLARETYPE {
        BIGT_SHINE = 9,
        BIGT_FRONT = 4,
        BIGT_BRAKE = 2,
        BIGT_REVERSE = 8,
        BIGT_REAR = 6,
    };

    enum BASEINFO_TEXTYPE {
        BITT_DEFAULT = 0,
        BITT_DOORIN = 1,
        BITT_INTERIOR = 3,
        BITT_TRUNKSPACE = 5,
        BITT_HANDLE = 0xF6,
        BITT_DECAL = 0xFD,
        BITT_NEEDLE = 0xFE,
        BITT_UNKNOWN = 0xFF,
    };

    enum BASEINFO_GEOMTYPE {
        BIGT_DEFAULT = 0,
        BIGT_WHEEL_FRONT1 = 1,
        BIGT_WHEEL_FRONT2 = 2,
        BIGT_WHEEL_REAR1 = 3,
        BIGT_WHEEL_REAR2 = 4,
        BIGT_STEER = 5,
        BIGT_DOOR_LEFT = 6,
        BIGT_DOOR_RIGHT = 7,
        BIGT_HOOD = 8,
        BIGT_TRUNK = 9,
        BIGT_SPOILER = 10,
        BIGT_THREAD_FRONT1 = 0x12,
        BIGT_THREAD_FRONT2 = 0x13,
        BIGT_THREAD_REAR1 = 0x14,
        BIGT_THREAD_REAR2 = 0x15,

        BIGT_GLARE_FRONTWHEEL = 1,
        BIGT_GLARE_MIRROR = 6,
        BIGT_GLARE_TRUNKBRAKE = 9,
        BIGT_GLARE_GASCAP = 8,
    };

    struct tBaseInfo {
        unsigned char GeomIndex;
        unsigned char TypeIndex;
        unsigned char GeomUnk;
        unsigned char GeomPlace;

        unsigned char WheelType;        // see BASEINFO_WHEELTYPE
        unsigned char ExtraType;        // see BASEINFO_EXTRATYPE
        unsigned char TextureType;        // see BASEINFO_TEXTYPE
        unsigned char GeomType;            // see BASEINFO_GEOMTYPE

        unsigned char AnimCount;
        unsigned char AnimType;        // see BASEINFO_ANIMTYPE
        unsigned char LevelIndex;    // see LEVELINDEX_n (n=LEVEL,INDEX,CREATE)
        unsigned char Zero1;

        unsigned char GlareType;    // see BASEINFO_GLARETYPE
        unsigned char InteriorVisible;    // 1 if true
        unsigned char Hand;            // 1 if true
        unsigned char Zero2;

        tBaseInfo() {
            GeomIndex = 0;
            TypeIndex = 0;
            GeomUnk = 0;
            GeomPlace = 0;

            WheelType = BIWT_NONWHEEL;
            ExtraType = BIET_TYPE1;
            TextureType = BITT_DEFAULT;
            GeomType = BIGT_DEFAULT;

            AnimCount = 1;
            AnimType = BIAT_NONE;
            LevelIndex = BIL_BODY;
            Zero1 = 0;

            GlareType = 0;
            InteriorVisible = 1;
            Hand = 0;
            Zero2 = 0;
        }

    };

    enum BASE_FLAGS {
        BF_NORMAL = 0,
        BF_WINDOW = 1,
        BF_EFFECT = 0x600,
    };

    struct tVector4 {
        float x, y, z, w;

        tVector4() {
            this->x = this->y = this->z = this->w = 0.0f;
        }

        tVector4(float x, float y, float z) {
            this->x = x;
            this->y = y;
            this->z = z;
            this->w = 0.0f;
        }

        tVector4(float x, float y, float z, float w) {
            this->x = x;
            this->y = y;
            this->z = z;
            this->w = w;
        }

        tVector4 operator+(const tVector4 &v) {
            return tVector4(x + v.x, y + v.y, z + v.z, w + v.w);
        }

        tVector4 operator-(const tVector4 &v) {
            return tVector4(x - v.x, y - v.y, z - v.z, w - v.w);
        }
    };

    struct tVector3 {
        float x, y, z;

        tVector3() {
            this->x = this->y = this->z = 0.0f;
        }

        tVector3(float x, float y, float z) {
            this->x = x;
            this->y = y;
            this->z = z;
        }

    };

    struct tVector2 {
        float u, v;

        tVector2() {
            this->u = this->v = 0.0f;
        }

        tVector2(float u, float v) {
            this->u = u;
            this->v = v;
        }
    };

    extern ICrpData *AllocateDataEntry(ENTRY_ID id);
}