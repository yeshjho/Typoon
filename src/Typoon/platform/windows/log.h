#pragma once
#include <string>

#include "../../utils/logger.h"


std::wstring get_last_error_string();

void log_last_error(const std::wstring& additionalMsg, LogLevel logLevel = ELogLevel::ERROR);
