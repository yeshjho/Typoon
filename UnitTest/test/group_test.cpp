#include <doctest.h>

#include "../../Typoon/match/trigger_tree.h"
#include "../util/test_util.h"


TEST_SUITE("Group")
{
    TEST_CASE("Group")
    {
        start_match_test_case();

        SUBCASE("Basic Functionality")
        {
            reconstruct_trigger_tree_with_u8string(u8R"({
                groups: [
                    {
                        keep_composite: true,
                        matches: [
                            {
                                trigger: '가',
                                replace: '나가',
                            },
                            {
                                trigger: 'ㄷ',
                                replace: 'ㄷ',
                            }
                        ]
                    }
                ],
            })");
            wait_for_trigger_tree_construction();

            simulate_type(L"가 가가가ㄷㄷㅏ");
            check_text_editor_simulator({ L"나가 나가나가나가ㄷㄷ|_|ㅏ", true });
        }

        SUBCASE("Option Merges")
        {
            reconstruct_trigger_tree_with_u8string(u8R"({
                groups: [
                    {
                        case_sensitive: true,
                        matches: [
                            {
                                trigger: 'lower',
                                replace: 'triggered',
                            },
                            {
                                trigger: 'UPPER',
                                replace: 'TRIGGERED',
                            },
                            {
                                trigger: 'MiXeD',
                                replace: 'tRiGgErEd',
                            }
                        ]
                    },
                    {
                        word: true,
                        matches: [
                            {
                                trigger: 'apple',
                                replace: 'banana',
                            }
                        ]
                    },
                    {
                        full_composite: true,
                        matches: [
                            {
                                trigger: '구누',
                                replace: '두루',
                            },
                            {
                                trigger: 'ㄳ',
                                replace: '감사',
                            }
                        ]
                    },
                    {
                        propagate_case: true,
                        uppercase_style: 'capitalize_words',
                        matches: [
                            {
                                trigger: ';car',
                                replace: '1d2o3g dr!1l',
                            },
                        ]
                    },
                    {
                        keep_composite: true,
                        matches: [
                            {
                                trigger: '가',
                                replace: '나가',
                                keep_composite: true,
                            },
                            {
                                trigger: 'ㄷ',
                                replace: 'ㄷ',
                                keep_composite: false,
                            }
                        ]
                    },
                ],
            })");
            wait_for_trigger_tree_construction();

            simulate_type(L";car ;Car ;CAr ;cAr ;CAR apple, apples apple\n"
                          L"lower LOWER lOwER upper UPPER upPeR mixed MIXED MiXeD "
                          L"구누 구누카 구눈한 구누, ㄱ사 ㄳ. ㄳ 가 가가가ㄷㄷㅏ");
            check_text_editor_simulator({ L"1d2o3g dr!1l 1D2o3g Dr!1L 1D2o3g Dr!1L 1d2o3g dr!1l 1D2O3G DR!1L banana, apples banana\n"
                                          L"triggered LOWER lOwER upper TRIGGERED upPeR mixed MIXED tRiGgErEd "
                                          L"두루 두루카 구눈한 두루, ㄱ사 감사. 감사 나가 나가나가나가ㄷㄷ|_|ㅏ", true });
        }

        end_match_test_case();
    }
}