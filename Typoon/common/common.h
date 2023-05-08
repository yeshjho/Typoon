#pragma once
#include <any>


void turn_on(const std::any& data);

void turn_off();

[[nodiscard]] bool is_turned_on();
