#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>

std::unordered_map<std::string, std::string> readConfig(const std::string& filename) {
    std::unordered_map<std::string, std::string> config;
    std::ifstream file(filename);
    std::string line;

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue; // игнорируем комментарии
        std::istringstream is_line(line);
        std::string key;
        if (std::getline(is_line, key, '=')) {
            std::string value;
            if (std::getline(is_line, value)) {
                config[key] = value;
            }
        }
    }

    return config;
}

std::string getVoiceLicense(const std::string& voicePath) {
    std::ifstream file(voicePath + "/model.key");
    if (!file) {
        return "";
    }

    std::string license;
    std::string line;

    while (std::getline(file, line)) {
        if (!license.empty() && !line.empty()) {
            license += "\n";
        }
        license += line;
    }

    return license;
}