#pragma once
#include <any>
#include <string>


void show_tray_icon(const std::any& data);

void remove_tray_icon();

void show_notification(const std::wstring& title, const std::wstring& body, bool isRealtime = false);
