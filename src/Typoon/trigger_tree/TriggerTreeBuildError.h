#pragma once
#include <cstdint>
#include <string_view>


namespace typoon::core
{
    struct Match;


    enum class ETriggerTreeBuildErrorType : std::uint8_t
    {
        UNREACHABLE_TRIGGER,
        IDENTICAL_TRIGGER,
    };


    struct TriggerTreeBuildError
    {
        ETriggerTreeBuildErrorType type;

        const core::Match* match = nullptr;
        std::wstring_view trigger{};

        const core::Match* otherMatch = nullptr;
        std::wstring_view otherTrigger{};
    };
}
