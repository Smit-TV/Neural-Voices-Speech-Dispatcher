#pragma once

#include <string>
#include <vector>
#include <unordered_map>

enum class Alphabet {
    LATIN,
    CYRILLIC,
    GREEK,
    ARABIC,
    HEBREW,
    ARMENIAN,
    GEORGIAN,
    DEVANAGARI,
    BENGALI,
    GURMUKHI,
    GUJARATI,
    ORIYA,
    TAMIL,
    TELUGU,
    KANNADA,
    MALAYALAM,
    SINHALA,
    THAI,
    LAO,
    KHMER,
    MYANMAR,
    HANGUL,
    HAN,
    HIRAGANA,
    KATAKANA,
    ETHIOPIC,
    SYRIAC,
    THAANA,
    MONGOLIAN,
    TIBETAN,
    NONE
};

struct StringPart {
    Alphabet alphabet;
    std::string str;
};

std::vector<StringPart> splitString(const std::string& str);

std::string getVoiceName(const std::unordered_map<std::string, std::string>* config, Alphabet alphabet);

