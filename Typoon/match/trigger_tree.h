#pragma once
#include <deque>
#include <filesystem>
#include <functional>
#include <set>
#include <thread>

#include "../input_multicast/input_multicast.h"
#include "match.h"


struct Letter
{
    /// These constants use the unassigned code points but not from the Private User Area to avoid conflicts with other software.
    /// Instead, use those within the Hangeul blocks, since they're unlikely to be assigned in the future. (Last modified in Unicode 5.2, which was released in 2009)

    /// Candidates: [
    ///         // Hangul Compatibility Jamo
    ///     0x3130, 0x318F,
    ///         // Hangul Jamo Extended-A
    ///     0xA97D, 0xA97E, 0xA97F,
    ///         // Hangul Compatibility. The least likely to be assigned since there can't be any more composite.
    ///     (0xD7A4), 0xD7A5, 0xD7A6, 0xD7A7, 0xD7A8, (0xD7A9), 0xD7AA, 0xD7AB, 0xD7AC, 0xD7AD, 0xD7AE, (0xD7AF),
    ///         // Hangul Jamo Extended-B
    ///     0xD7C7, 0xD7C8, 0xD7C9, 0xD7CA,
    ///     0xD7FC, 0xD7FD, 0xD7FE, 0xD7FF
    /// ]

    /// Constants for special triggers
    static constexpr wchar_t NON_WORD_LETTER = 0xD7A4;

    /// Constants for special replacements
    static constexpr wchar_t LAST_INPUT_LETTER = 0xD7AF;

    /// Constants for other purposes
    static constexpr wchar_t COMPOSITE_ENCLOSURE = 0xD7A9;


    wchar_t letter;
    bool isCaseSensitive;  // Won't be true if `letter` is not cased.
    bool doNeedFullComposite;  // Won't be true if `letter` is not Korean or not an ending.

    bool operator==(wchar_t ch) const;

    bool operator==(const Letter& other) const;

    // NOTE: We don't need partial ordering, since we're already saying that case insensitive and same-if-lowered `Letter`s are equal.
    std::strong_ordering operator<=>(const Letter& other) const;
};


// A node of the tree. It's essentially a link, since a node doesn't hold any information.
struct Node
{
    int parentIndex = -1;
    int childStartIndex = -1;
    int childLength = 0;
    Letter letter;
    int endingIndex = -1;
};


// The last letter of a trigger, contains the information for the replacement string.
struct Ending
{
    enum class EReplaceType
    {
        TEXT,
        IMAGE,
        COMMAND,
    };

    EReplaceType type = EReplaceType::TEXT;
    int replaceStringIndex = -1;
    unsigned int replaceStringLength = 0;
    unsigned int backspaceCount = 0;
    unsigned int cursorMoveCount = 0;
    bool propagateCase = false;  // Won't be true if the first letter is not cased.
    Match::EUppercaseStyle uppercaseStyle = Match::EUppercaseStyle::FIRST_LETTER;  // Only used if `propagateCase` is true.
    bool keepComposite = false;  // Won't be true if the letter is not Korean or need full composite.
};


class pcre2_real_code_16;

class RegexPattern
{
public:
    RegexPattern(pcre2_real_code_16* pattern) : mPattern(pattern) {}
    ~RegexPattern();
    RegexPattern(const RegexPattern& other) = default;
    RegexPattern(RegexPattern&& other) noexcept = default;
    RegexPattern& operator=(const RegexPattern& other) = default;
    RegexPattern& operator=(RegexPattern&& other) noexcept = default;

    operator pcre2_real_code_16* () const noexcept { return mPattern; }


private:
    pcre2_real_code_16* mPattern = nullptr;
};


struct RegexMatch
{
    RegexPattern pattern = nullptr;
    bool needFullComposite = false;

    Ending::EReplaceType type = Ending::EReplaceType::TEXT;
    std::wstring replaceString;
    bool keepComposite = false;
};


// The agents for tracking the current possible triggers.
struct Agent
{
    const Node* node = nullptr;
    int strokeStartIndex = -1;

    constexpr bool operator==(const Agent& other) const noexcept = default;
};


// The agents that is 'dead' but can be brought back to life with backspaces.
struct DeadAgent : Agent
{
    int backspacesNeeded = 0;
};


class TriggerTree
{
public:
    TriggerTree(std::filesystem::path matchFile, std::vector<std::filesystem::path> includes = {}, std::vector<std::filesystem::path> excludes = {});
    ~TriggerTree();
    TriggerTree(const TriggerTree& other) = delete;
    TriggerTree(TriggerTree&& other) noexcept = delete;
    TriggerTree& operator=(const TriggerTree& other) = delete;
    TriggerTree& operator=(TriggerTree&& other) noexcept = delete;

    void Reconstruct(std::string_view matchesString = {}, std::function<void()> onFinish = {});
    void ReconstructWith(std::filesystem::path matchFile);
    void HaltConstruction();
    // For unit tests
    void WaitForConstruction() const;
    void ResetAgents();
    void OnInput(const InputMessage(&inputs)[MAX_INPUT_COUNT], int length, bool clearAllAgents);

private:
    void replaceString(const Ending& ending, const Agent& agent, std::wstring_view stroke, const InputMessage(&inputs)[MAX_INPUT_COUNT], int inputLength, int inputIndex, bool doNeedFullComposite);


private:
    std::filesystem::path mMatchFile;
    std::vector<std::filesystem::path> mIncludes;
    std::vector<std::filesystem::path> mExcludes;
    std::set<std::filesystem::path> mImportedFiles;

    std::vector<Node> mTree;
    unsigned int mTreeHeight = 0;
    std::vector<Ending> mEndings;
    std::wstring mReplaceStrings;

    std::vector<Agent> mAgents;
    std::vector<Agent> mNextIterationAgents;
    std::deque<DeadAgent> mDeadAgents;
    std::wstring mStroke;
    bool mShouldResetAgents = false;
    Agent mRootAgent;

    bool mHasRegexMatches = false;
    std::vector<RegexMatch> mRegexMatches;
    std::wstring mStrokeForRegex;

    std::atomic<bool> mIsTriggerTreeOutdated = true;
    std::atomic<bool> mIsConstructingTriggerTree = false;

    std::jthread mTriggerTreeConstructorThread;
};


inline std::unordered_map<std::filesystem::path, std::deque<TriggerTree*>> trigger_trees_by_match_file;
