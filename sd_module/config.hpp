#pragma once
#include <unordered_map>
#include <string>

std::unordered_map<std::string, std::string> readConfig(const std::string& configPath);
std::string getVoiceLicense(const std::string& voicePath);