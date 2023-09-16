#pragma once
#include <filesystem>
#include <functional>


void push_current_clipboard_state();
void pop_clipboard_state();
void pop_clipboard_state_without_restoring();

void pop_clipboard_state_with_delay(std::function<bool()> predicate = []() { return true; });

void set_clipboard_image(const std::filesystem::path& imagePath);
void set_clipboard_text(const std::wstring& text);
