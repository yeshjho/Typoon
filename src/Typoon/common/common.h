#pragma once
#include <any>


bool turn_on(const std::any& data);

void turn_off();

[[nodiscard]] bool is_turned_on();
