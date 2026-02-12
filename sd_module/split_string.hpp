#pragma once

#include <string>
#include <vector>

enum class Alphabet {
    LATIN,
    CYRILLIC,
    NONE
};

struct StringPart {
    Alphabet alphabet;
    std::string str;
};

std::vector<StringPart> splitString(const std::string& str);

