#pragma once
#include <any>


inline bool is_hangeul_on = false;

bool start_input_watcher(const std::any& data = {});
void end_input_watcher();
