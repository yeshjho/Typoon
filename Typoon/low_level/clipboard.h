#pragma once
#include <filesystem>


void push_current_clipboard_state();
void pop_current_clipboard_state();

void set_clipboard_image(const std::filesystem::path& imagePath);
void set_clipboard_text(const std::wstring& text);
