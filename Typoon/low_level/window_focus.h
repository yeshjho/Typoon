#pragma once
#include <any>
#include <optional>
#include <string>


std::optional<std::wstring> get_program_name(const std::any& data = {});
void check_for_window_focus_change(const std::any& data = {});
