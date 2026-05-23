#include "Hangeul.h"

#include "ime/Composition.h"


namespace typoon::util
{
    std::wstring decompose_hangeul(const std::wstring_view str)
    {
        constexpr int lettersOfAnInitial = std::size(DECOMPOSED_MEDIAL_MAP) * std::size(DECOMPOSED_FINAL_MAP);

        std::wstring result;
        result.reserve(str.size() * 3);

        for (const wchar_t c : str)
        {
            if (is_hangeul_composite(c))
            {
                const auto [initialIndex, nonInitialPart] = std::div(c - L'가', lettersOfAnInitial);
                const auto [medialIndex, finalIndex] = std::div(nonInitialPart, std::size(DECOMPOSED_FINAL_MAP));
                result += INITIAL_MAP[initialIndex];
                result += DECOMPOSED_MEDIAL_MAP[medialIndex];
                result += DECOMPOSED_FINAL_MAP[finalIndex];
            }
            else if (is_hangeul_consonant(c))
            {
                result += DECOMPOSED_CONSONANT_MAP[c - L'ㄱ'];
            }
            else if (is_hangeul_vowel(c))
            {
                result += DECOMPOSED_MEDIAL_MAP[c - L'ㅏ'];
            }
            else
            {
                result += c;
            }
        }

        return result;
    }

    std::wstring combine_hangeul(const std::wstring_view str)
    {
        std::wstring result;
        result.reserve(str.size() / 2);
        core::Composition composition{ [&result](const wchar_t letter) { result += letter; } };

        for (const wchar_t c : str)
        {
            composition.AddAlphabet(c);
        }
        if (const wchar_t last = composition.ComposeLetter();
            last != 0)
        {
            result += last;
        }

        return result;
    }

    std::wstring latin_alphabet_to_hangeul_alphabet(const std::wstring_view str)
    {
        std::wstring result;
        result.reserve(str.size());

        for (const wchar_t c : str)
        {
            if ('a' <= c && c <= 'z')
            {
                result += LOWER_LATIN_ALPHABET_TO_HANGEUL[c - L'a'];
            }
            else if ('A' <= c && c <= 'Z')
            {
                result += UPPER_LATIN_ALPHABET_TO_HANGEUL[c - L'A'];
            }
            else
            {
                result += c;
            }
        }

        return result;
    }

    std::wstring hangeul_alphabet_to_latin_alphabet(const std::wstring_view str)
    {
        std::wstring result;
        result.reserve(str.size());

        // ㄱ~ㅣ
        constexpr char hangeulAlphabetToLatinAlphabet[] = {
            'r', 'R', 0, 's', 0, 0, 'e', 'E', 'f', 0, 0, 0, 0, 0, 0, 0, 'a', 'q', 'Q', 0, 't', 'T', 'd', 'w', 'W', 'c', 'z', 'x', 'v', 'g',
            'k', 'o', 'i', 'O', 'j', 'p', 'u', 'P', 'h', 0, 0, 0, 'y', 'n', 0, 0, 0, 'b', 'm', 0, 'l'
        };

        for (const wchar_t c : str)
        {
            if (is_hangeul_alphabet(c))
            {
                if (const char converted = hangeulAlphabetToLatinAlphabet[c - L'ㄱ'])
                {
                    result += converted;
                }
                else
                {
                    result += c;
                }
            }
            else
            {
                result += c;
            }
        }

        return result;
    }
}
