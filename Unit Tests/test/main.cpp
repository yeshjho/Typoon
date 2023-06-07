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


TEST_F(BasicMatchTest, TestTest2)
{
    SetUp(R"({
        matches: [
            {
                trigger: 'wwww',
                replace: '가a나',
                keep_composite: true,
            },
            {
                trigger: '난',
                replace: '단',
            },
        ]
    })");

    simulate_type(L"wwwwㄴ");

    EXPECT_EQ(text_editor_simulator.GetText(), L"가a단");
}
