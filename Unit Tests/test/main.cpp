#include <gtest/gtest.h>

#include "../util/text_editor_simulator.h"
#include "../util/test_util.h"


TEST_F(BasicMatchTest, TestTest)
{
    SetUp(R"({
        matches: [
            {
                trigger: ';sti',
                replace: 'static_cast<int>(|_|)'
            },
        ]
    })");

    simulate_type(L";stifloatValue");
    
    EXPECT_EQ(text_editor_simulator.GetText(), L"static_cast<int>(floatValue)");
}
