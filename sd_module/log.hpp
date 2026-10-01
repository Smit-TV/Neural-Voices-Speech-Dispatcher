#pragma once

#include <string>



void enableLogging(bool enable);

void enableMicrosoftLogging(int level, const std::string& path);

void log(std::string text);