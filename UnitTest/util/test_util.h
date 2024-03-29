#pragma once
#include <doctest.h>
#include <uni-algo/conv.h>

#include "../../Typoon/utils/config.h"
#include "text_editor_simulator.h"


inline Config default_config{.maxBackspaceCount = 5, .cursorPlaceholder = L"|_|" };


void simulate_type(std::wstring_view text);

void reconstruct_trigger_tree_with_u8string(std::u8string_view text);

void wait_for_trigger_tree_construction();

void check_text_editor_simulator(const TextState& textState);

void check_normalization(std::wstring_view original, std::wstring_view normalized);

void start_match_test_case(const Config& config = default_config);

void end_match_test_case();


namespace doctest
{

template<std::convertible_to<std::wstring_view> T>
struct StringMaker<T>
{
    static String convert(const T& value)
    {
        const std::string s = '\n' + una::utf16to8(value) + '\n';
        return String{ s.c_str() };
    }
};

template<>
struct StringMaker<TextState>
{
    static String convert(const TextState& value)
    {
        std::wstring originalStr{ value.text };

        const auto [cursorPos, endPos] = value.RemoveCursorPos(originalStr);

        std::wstring augmentedStr{ originalStr };
        if (value.isLetterAtCursorInComposition)
        {
            augmentedStr.insert(cursorPos, 1, L'<');
            if (const unsigned int pos = cursorPos + 2;
                augmentedStr.size() >= pos)
            {
                augmentedStr.insert(pos, 1, L'>');
            }
            else
            {
                augmentedStr.push_back(L'>');
            }
        }
        if (cursorPos != endPos)
        {
            augmentedStr.insert(cursorPos, value.cursorPlaceholder);
        }

        const std::wstring formattedStr = std::format(L"\"{}\" [\"{}\", compose: {}, cursor: {}]", 
            augmentedStr, originalStr, value.isLetterAtCursorInComposition, cursorPos);
        return StringMaker<std::wstring>::convert(formattedStr);
    }
};

template<>
struct StringMaker<TextEditorSimulator>
{
    static String convert(const TextEditorSimulator& value)
    {
        return StringMaker<TextState>::convert({ value.GetText(), value.IsLetterAtCursorInComposition(), value.GetCursorPos() });
    }
};

}
