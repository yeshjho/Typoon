#pragma once
#include <gtest/gtest.h>

#include "../../Typoon/utils/config.h"


void simulate_type(std::wstring_view text);


inline Config default_config{ .maxBackspaceCount = 5, .cursorPlaceholder = L"|_|" };


class BasicMatchTest : public testing::Test
{
protected:
    void SetUp(std::string_view matchesString);

    void TearDown() override;
};
