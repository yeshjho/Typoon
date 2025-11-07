#pragma once
#include <filesystem>
#include <string>
#include <vector>

#include "MatchValidateError.h"
#include "Options.h"


namespace typoon::parse
{

struct Match : Options
{
    std::wstring trigger{};
    std::vector<std::wstring> triggers{};
    std::wstring replace{};
    std::filesystem::path replace_image{};
    std::wstring replace_command{};

    /**
     * @brief 설정된 값들이 올바른지 검사. 옵션 검사 포함
     * @return 에러 리스트
     */
    [[nodiscard]] std::vector<MatchValidateError> Validate(std::wstring_view cursorPlaceholder) const;
    /**
     * @brief Validate 후 나온 에러들을 수정하려고 시도. 옵션 수정 포함
     * @return 수정에 성공했는지 여부. (트리거가 아예 비어 있는 등 수정이 불가능한 경우가 있음)
     */
    bool TryFixUp(std::wstring_view cursorPlaceholder);
};

}
