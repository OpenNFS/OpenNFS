#pragma once

#include <fstream>
#include <cstring>

namespace CrpLib {

    class ICrpEntry {
    public:
        virtual void Read(std::fstream *file) {};

        virtual void WriteEntry(std::fstream *file) {};

        virtual void WriteData(std::fstream *file) {};

        virtual ~ICrpEntry() {};
    };

    class ICrpData {
    public:
        virtual void Read(std::fstream *file, ICrpEntry *entry) {};

        virtual void Write(std::fstream *file) {};

        virtual int GetEntryLength() = 0;

        virtual int GetEntryCount() = 0;

        virtual ~ICrpData() {};
    };

}