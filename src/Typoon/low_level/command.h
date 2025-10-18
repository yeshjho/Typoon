#pragma once
#include <string>


std::pair<std::wstring, int> run_command_and_get_output(std::wstring_view command);
