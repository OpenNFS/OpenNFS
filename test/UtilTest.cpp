#include "gtest/gtest.h"

#include "../src/Util/Logger.h"

using namespace OpenNFS;

class UtilTest : public testing::Test {
public:
    virtual void SetUp() {
        // Initialise Logging
        auto logger = std::make_shared<Logger>();
    }

    virtual void TearDown() {
    }
};
